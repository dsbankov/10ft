/*
  ==============================================================================

    AudioWaveformOpenGLComponent.cpp
    Created: 9 Jul 2018 12:16:15pm
    Author:  DBANKOV

  ==============================================================================
*/

#include <chrono>

#include "AudioWaveformOpenGLComponent.h"


AudioWaveformOpenGLComponent::AudioWaveformOpenGLComponent ()
{
    setInterceptsMouseClicks (false, true);
}

AudioWaveformOpenGLComponent::~AudioWaveformOpenGLComponent ()
{
    clearVertices ();
}

void AudioWaveformOpenGLComponent::initialise (
    OpenGLContext& openGLContext)
{
    String vertexShader =
        "attribute vec2 position;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position = vec4(position.x, position.y, 1.0, 1.0);\n"
        "};";
    String fragmentShader =
        "uniform vec4 colour;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = colour;\n"
        "}";

    std::unique_ptr<OpenGLShaderProgram> newShaderProgram (
        new OpenGLShaderProgram (openGLContext)
    );
    String statusText;

    if (
        newShaderProgram->addVertexShader (
            OpenGLHelpers::translateVertexShaderToV3 (vertexShader)) &&
        newShaderProgram->addFragmentShader (
            OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader)) &&
        newShaderProgram->link ())
    {
        shaderProgram.reset (newShaderProgram.release ());

        shaderProgram->use ();

        Colour waveformColour = getLookAndFeel ().findColour (
            ColourIds::waveformColour
        );
        uniform.reset (
            new OpenGLShaderProgram::Uniform (*shaderProgram, "colour")
        );
        uniform->set (waveformColour.getFloatRed (),
            waveformColour.getFloatGreen (),
            waveformColour.getFloatBlue (),
            waveformColour.getFloatAlpha ());

        position.reset (
            new OpenGLShaderProgram::Attribute (*shaderProgram, "position")
        );

        vertexBuffer.reset (new VertexBuffer (openGLContext));

        statusText = "GLSL: v" +
            String (OpenGLShaderProgram::getLanguageVersion (), 2);
    }
    else
    {
        statusText = newShaderProgram->getLastError ();
        Logger::outputDebugString (statusText);
    }
}

void AudioWaveformOpenGLComponent::shutdown (OpenGLContext& openGLContext)
{
    uniform.reset ();
    position.reset ();
    vertexBuffer.reset ();
    shaderProgram.reset ();
}

void AudioWaveformOpenGLComponent::render (OpenGLContext& openGLContext)
{
    jassert (OpenGLHelpers::isContextActive ());

    if (verticesPerChannel == nullptr)
    {
        return;
    }

    OpenGLHelpers::clear (
        getLookAndFeel ().findColour (ColourIds::waveformBackgroundColour)
    );

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_LINE_SMOOTH);

    shaderProgram->use ();

    float scale = openGLContext.getRenderingScale ();
    Component* parentComponent = getParentComponent ();
    Rectangle<int> parentBounds = parentComponent->getBounds (),
        globalBounds = parentComponent->getLocalArea (this, getLocalBounds ());
    GLint x = scale * globalBounds.getX ();
    GLsizei width = scale * globalBounds.getWidth ();
    GLsizei height = scale * (globalBounds.getHeight () / numChannels);

    for (unsigned int channel = 0; channel < numChannels; channel++)
    {
        GLint y = scale * (
            parentBounds.getHeight () - globalBounds.getBottom () +
            channel * (globalBounds.getHeight () / numChannels)
        );

        glViewport (x, y, width, height);

        if (calculateVerticesTrigger)
        {
            calculateVertices (channel);
        }

        vertexBuffer->bind (verticesPerChannel[channel], numSamples);

        openGLContext.extensions.glVertexAttribPointer (position->attributeID, 2,
            GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
        openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);

        glDrawArrays (GL_LINE_STRIP, 0, numSamples);

        openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);

        vertexBuffer->unbind ();
    }

    if (calculateVerticesTrigger)
    {
        calculateVerticesTrigger = false;
    }

    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_BLEND);
}

void AudioWaveformOpenGLComponent::load (AudioFormatReader* audioReader)
{
    audioBuffer.setSize(audioReader->numChannels, audioReader->lengthInSamples);
    audioReader->read (&audioBuffer, 0, audioReader->lengthInSamples, 0, true, true);

    numChannels = audioReader->numChannels;
    lengthInSamples = audioReader->lengthInSamples;
    samplesPerChannel = audioBuffer.getArrayOfReadPointers ();

    clearVertices ();

    verticesPerChannel = new Vertex*[numChannels];
    for (unsigned int channel = 0; channel < numChannels; channel++)
    {
        verticesPerChannel[channel] = new Vertex[lengthInSamples];
    }
}

void AudioWaveformOpenGLComponent::display (
    int64 displayStartSample, int64 displayNumSamples)
{
    startSample = displayStartSample;
    numSamples = displayNumSamples;
    calculateVerticesTrigger = true;
}

// ==============================================================================

void AudioWaveformOpenGLComponent::calculateVertices (unsigned int channel)
{
    auto start = std::chrono::system_clock::now ();
    int64 endSample = startSample + numSamples;

    for (int64 sample = startSample, i = 0; sample < endSample; sample++, i++)
    {
        Vertex vertex;
        vertex.x = (((GLfloat)(sample - startSample) / numSamples) * 2) - 1;
        vertex.y = samplesPerChannel[channel][sample];
        verticesPerChannel[channel][i] = vertex;
    }

    auto end = std::chrono::system_clock::now ();
    std::chrono::duration<double> diff = end - start;

    Logger::outputDebugString (
        String(numSamples) + " samples @ " +
        String(diff.count()) + " s");
}

void AudioWaveformOpenGLComponent::clearVertices ()
{
    if (verticesPerChannel != nullptr)
    {
        for (unsigned int channel = 0; channel < numChannels; channel++)
        {
            delete[] verticesPerChannel[channel];
        }
        delete[] verticesPerChannel;
        verticesPerChannel = nullptr;
    }
}

// ==============================================================================

AudioWaveformOpenGLComponent::VertexBuffer::VertexBuffer (
    OpenGLContext& openGLContext
) :
    openGLContext (openGLContext)
{
    openGLContext.extensions.glGenBuffers (1, &id);
};

AudioWaveformOpenGLComponent::VertexBuffer::~VertexBuffer ()
{
    openGLContext.extensions.glDeleteBuffers (1, &id);
}

void AudioWaveformOpenGLComponent::VertexBuffer::bind (
    Vertex* vertices, int64 verticesCount)
{
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, id);
    openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
        verticesCount * sizeof (Vertex),
        vertices,
        GL_STATIC_DRAW
    );
}

void AudioWaveformOpenGLComponent::VertexBuffer::unbind ()
{
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
}