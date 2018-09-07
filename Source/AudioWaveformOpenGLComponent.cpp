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
            waveformColour.getFloatAlpha ()
        );

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

void AudioWaveformOpenGLComponent::shutdown (OpenGLContext&)
{
    uniform.reset ();
    position.reset ();
    vertexBuffer.reset ();
    shaderProgram.reset ();
}

void AudioWaveformOpenGLComponent::render (OpenGLContext& openGLContext)
{
    jassert (OpenGLHelpers::isContextActive ());

    if (vertices.isEmpty())
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

    double scale = openGLContext.getRenderingScale ();
    Component* parent = getParentComponent ();
    Rectangle<int> parentBounds = parent->getBounds (),
        globalBounds = parent->getLocalArea (this, getLocalBounds ());
    GLint x = (GLint) (scale * globalBounds.getX ());
    GLsizei width = (GLsizei) (scale * globalBounds.getWidth ());
    GLsizei height = (GLsizei) (scale * (globalBounds.getHeight () / bufferNumChannels));

    for (int channel = 0; channel < bufferNumChannels; channel++)
    {
        GLint y = (GLint) (scale * (
            parentBounds.getHeight () - globalBounds.getBottom () +
            channel * (globalBounds.getHeight () / bufferNumChannels)
        ));

        glViewport (x, y, width, height);

        Array<Vertex, CriticalSection> channelVertices = vertices.getReference (channel);

        vertexBuffer->bind (channelVertices.getRawDataPointer (), channelVertices.size ());

        openGLContext.extensions.glVertexAttribPointer (position->attributeID, 2,
            GL_FLOAT, GL_FALSE, 2 * sizeof (GLfloat), 0);
        openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);

        glDrawArrays (GL_LINE_STRIP, 0, (GLsizei)channelVertices.size ());

        openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);

        vertexBuffer->unbind ();
    }

    glDisable (GL_LINE_SMOOTH);
    glDisable (GL_BLEND);
}

void AudioWaveformOpenGLComponent::load (AudioSampleBuffer* buffer,
    const CriticalSection& lock)
{
    bufferNumChannels = buffer->getNumChannels ();
    visibleRegionStartSample = 0;
    visibleRegionNumSamples = buffer->getNumSamples ();
    bufferUpdateLock = &lock;

    vertices.clear ();
    vertices.insertMultiple (0, Array<Vertex, CriticalSection>(), bufferNumChannels);
}

void AudioWaveformOpenGLComponent::display (
    AudioSampleBuffer* buffer, int64 startSample, int64 numSamples)
{
    visibleRegionStartSample = startSample;
    visibleRegionNumSamples = numSamples;

    calculateVertices (buffer);
}

void AudioWaveformOpenGLComponent::refresh (AudioSampleBuffer* buffer)
{
    calculateVertices (buffer);
}

// ==============================================================================

void AudioWaveformOpenGLComponent::calculateVertices (AudioSampleBuffer* buffer)
{
    for (int channel = 0; channel < bufferNumChannels; channel++)
    {
        calculateVertices (buffer, channel);
    }
}

void AudioWaveformOpenGLComponent::calculateVertices (
    AudioSampleBuffer* buffer, unsigned int channel
)
{
    //auto start = std::chrono::system_clock::now ();

    // More accurate because we depend on the count of the samples 
    // of the current file. The larger the file the less samples 
    // we use when zoomed out
    skipSamples = (unsigned int) (
        visibleRegionNumSamples / (buffer->getNumSamples () * 0.04)
    );
    skipSamples = (skipSamples > 0) ? skipSamples : 1;
    // Alternative approach:
    // skipSamples = numSamples / (sampleRate * 12);
    // More of a constant UI speed but not very accurate

    int64 endSample = visibleRegionStartSample + visibleRegionNumSamples,
        numVertices = visibleRegionNumSamples % skipSamples ?
            visibleRegionNumSamples / skipSamples + 1 :
            visibleRegionNumSamples / skipSamples;

    vertices.getReference (channel).resize (numVertices);

    const ScopedLock lock (*bufferUpdateLock);
    Logger::outputDebugString ("ENTER calculateVertices");

    const float* samples = buffer->getReadPointer (channel);

    for (int64 sample = visibleRegionStartSample, vertice = 0;
        sample < endSample;
        sample += skipSamples, vertice++)
    {
        GLfloat sampleValue = getPeakSampleValue (samples, sample,
            jmin ((int64)skipSamples, endSample - sample));

        Vertex vertex;
        // should be in the [-1,+1] range
        vertex.x = (((GLfloat)vertice / (GLfloat)numVertices) * 2) - 1;
        vertex.y = sampleValue;

        vertices.getReference (channel).setUnchecked (vertice, vertex);
    }

    Logger::outputDebugString ("EXIT calculateVertices");

    //auto end = std::chrono::system_clock::now ();
    //std::chrono::duration<double> diff = end - start;
    //Logger::outputDebugString (
    //    String(buffer->getNumSamples ()) + " samples / " +
    //    String(numVertices) + " vertices / " +
    //    String(skipSamples) + " skipping @ " +
    //    String(diff.count()) + " s");
}

GLfloat AudioWaveformOpenGLComponent::getAverageSampleValue (
    const float* samples,
    int64 currentStartSample,
    int64 currentNumSamples)
{
    GLfloat samplesSum = 0;
    unsigned int samplesCount = 0;
    int64 endSample = currentStartSample + currentNumSamples;

    for (int64 sample = currentStartSample; sample < endSample; sample++)
    {
        samplesSum += samples[sample];
        samplesCount++;
    }

    return samplesSum / samplesCount;
}

GLfloat AudioWaveformOpenGLComponent::getPeakSampleValue (
    const float* samples,
    int64 currentStartSample,
    int64 currentNumSamples)
{
    GLfloat peakValue = 0.0f;
    int64 endSample = currentStartSample + currentNumSamples;

    for (int64 sample = currentStartSample; sample < endSample; sample++)
    {
        float sampleValue = samples[sample];
        if (std::abs (peakValue) < std::abs (sampleValue))
        {
            peakValue = sampleValue;
        }
    }

    return peakValue;
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