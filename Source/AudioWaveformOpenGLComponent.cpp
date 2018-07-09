/*
  ==============================================================================

    AudioWaveformOpenGLComponent.cpp
    Created: 9 Jul 2018 12:16:15pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "AudioWaveformOpenGLComponent.h"

AudioWaveformOpenGLComponent::AudioWaveformOpenGLComponent ()
{
    setInterceptsMouseClicks (false, true);
}

AudioWaveformOpenGLComponent::~AudioWaveformOpenGLComponent ()
{
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

    float scale = openGLContext.getRenderingScale ();
    Component* parent = getParentComponent ();
    Rectangle<int> parentBounds = parent->getBounds ();
    Rectangle<int> globalBounds = parent->getLocalArea (this, getLocalBounds ());
    glViewport (scale * globalBounds.getX (),
        scale * (parentBounds.getHeight () - globalBounds.getBottom ()),
        scale * globalBounds.getWidth (), scale * globalBounds.getHeight ());

    OpenGLHelpers::clear (
        getLookAndFeel ().findColour (ColourIds::waveformBackgroundColour)
    );

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_LINE_SMOOTH);

    shaderProgram->use ();

    if (calculateVerticesTrigger)
    {
        calculateVertices ();
        calculateVerticesTrigger = false;
    }

    vertexBuffer->bind (sampleVertices);

    openGLContext.extensions.glVertexAttribPointer (position->attributeID, 2,
        GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
    openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);

    glDrawArrays (GL_LINE_STRIP, 0, sampleVertices.size ());

    openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);

    vertexBuffer->unbind ();

    glDisable (GL_LINE_SMOOTH);
}

void AudioWaveformOpenGLComponent::load (const float* samples)
{
    samplesArray = samples;
}

void AudioWaveformOpenGLComponent::display (
    int startSample, int numSamples)
{
    visibleRegionStartSample = startSample;
    visibleRegionNumSamples = numSamples;
    calculateVerticesTrigger = true;
}

// ==============================================================================

void AudioWaveformOpenGLComponent::calculateVertices ()
{
    Array<Vertex> newArray;
    int endSample = visibleRegionStartSample + visibleRegionNumSamples;
    for (int sampleIndex = visibleRegionStartSample;
        sampleIndex < endSample;
        sampleIndex++)
    {
        Vertex vertex;
        vertex.x = (((GLfloat)(sampleIndex - visibleRegionStartSample) /
            visibleRegionNumSamples) * 2) - 1;
        vertex.y = samplesArray[sampleIndex];
        newArray.add (vertex);
    }
    sampleVertices.swapWith (newArray);
    Logger::outputDebugString ("calculateVertices @ Program " +
        String (shaderProgram->getProgramID ()) + " [" +
        String (visibleRegionStartSample) + ", " + String (endSample) +
        "]");
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
    Array<Vertex>& buffer)
{
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, id);
    openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr> (
            static_cast<size_t> (buffer.size ()) * sizeof (Vertex)
            ),
        buffer.getRawDataPointer (),
        GL_STATIC_DRAW
    );
}

void AudioWaveformOpenGLComponent::VertexBuffer::unbind ()
{
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
}