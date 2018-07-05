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
        "in vec3 vertexColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = vec4(vertexColor, 1.0);\n"
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
    
        openGLContext.extensions.glGenBuffers (1, &vertexBufferId);
    
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
    openGLContext.extensions.glDeleteBuffers (1, &vertexBufferId);
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
    
    OpenGLShaderProgram::Attribute position (*shaderProgram, "position");
    
    shaderProgram->use ();
    
    if (calculateVerticesTrigger)
    {
        calculateVertices ();
        calculateVerticesTrigger = false;
    }
    
    // TODO pass ColourIds::waveformColour to the fragment shader
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBufferId);
    openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
            static_cast<GLsizeiptr> (static_cast<size_t> (vertices.size ()) * sizeof (Vertex)),
            vertices.getRawDataPointer (), GL_STATIC_DRAW);
    
    openGLContext.extensions.glVertexAttribPointer (position.attributeID, 2,
        GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
    openGLContext.extensions.glEnableVertexAttribArray (position.attributeID);
    
    glDrawArrays (GL_LINE_STRIP, 0, vertices.size ());
    
    openGLContext.extensions.glDisableVertexAttribArray (position.attributeID);
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
}

void AudioWaveformChannelComponent::redraw (int startSample, int numSamples)
{
    this->startSample = startSample;
    this->numSamples = numSamples;
    calculateVerticesTrigger = true;
}

void AudioWaveformChannelComponent::loadBuffer (const float* readBuffer)
{
    this->readBuffer = readBuffer;
}

// ==============================================================================

void AudioWaveformChannelComponent::calculateVertices ()
{
    Array<Vertex> newArray;
    int endSample = startSample + numSamples;
    for (int sampleIndex = startSample; sampleIndex < endSample; sampleIndex++)
    {
        Vertex vertex;
        vertex.x = (((GLfloat)(sampleIndex - startSample) /
            numSamples) * 2) - 1;
        vertex.y = readBuffer[sampleIndex];
        newArray.add (vertex);
    }
    vertices.swapWith (newArray);
    Logger::outputDebugString ("calculateVertices [" +
        String(startSample) + ", " + String(endSample) + "]");
}
