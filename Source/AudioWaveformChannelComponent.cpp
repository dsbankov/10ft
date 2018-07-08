/*
  ==============================================================================

    AudioWaveformChannelComponent.cpp
    Created: 4 Jul 2018 12:42:51pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "AudioWaveformChannelComponent.h"


AudioWaveformChannelComponent::AudioWaveformChannelComponent ()
{
    setInterceptsMouseClicks (false, true);
}

AudioWaveformChannelComponent::~AudioWaveformChannelComponent ()
{
}

void AudioWaveformChannelComponent::initialise (OpenGLContext& openGLContext)
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
        uniform.reset (new OpenGLShaderProgram::Uniform (*shaderProgram, "colour"));
        uniform->set (waveformColour.getFloatRed (),
            waveformColour.getFloatGreen (),
            waveformColour.getFloatBlue (),
            waveformColour.getFloatAlpha ());

        attributes.reset (new Attributes (openGLContext, *shaderProgram));
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

void AudioWaveformChannelComponent::shutdown (OpenGLContext& openGLContext)
{
    attributes.reset ();
    uniform.reset ();
    vertexBuffer.reset ();
    shaderProgram.reset ();
}

void AudioWaveformChannelComponent::render (OpenGLContext& openGLContext)
{
    jassert (OpenGLHelpers::isContextActive ());
    
    auto desktopScale = (float)openGLContext.getRenderingScale ();
    OpenGLHelpers::clear (
        getLookAndFeel ().findColour (ColourIds::waveformBackgroundColour)
    );

    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glViewport (0, 0, roundToInt (desktopScale * getWidth ()),
        roundToInt (desktopScale * getHeight ()));
    
    shaderProgram->use ();
    
    if (calculateVerticesTrigger)
    {
        calculateVertices ();
        calculateVerticesTrigger = false;
    }

    vertexBuffer->bind (sampleVertices);
    attributes->enable ();
    
    glDrawArrays (GL_LINE_STRIP, 0, sampleVertices.size ());
    
    attributes->disable ();
    vertexBuffer->unbind ();
}

void AudioWaveformChannelComponent::load (const float* samples)
{
    samplesArray = samples;
}

void AudioWaveformChannelComponent::display (int startSample, int numSamples)
{
    visibleRegionStartSample = startSample;
    visibleRegionNumSamples = numSamples;
    calculateVerticesTrigger = true;
}

// ==============================================================================

void AudioWaveformChannelComponent::calculateVertices ()
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
    Logger::outputDebugString ("calculateVertices [" +
        String(visibleRegionStartSample) + ", " + String(endSample) + "]");
}
