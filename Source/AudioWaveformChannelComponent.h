/*
  ==============================================================================

    AudioWaveformChannelComponent.h
    Created: 4 Jul 2018 12:42:51pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "OpenGLComponent.h"
#include "VertexBuffer.h"


class AudioWaveformChannelComponent : public OpenGLComponent
{

public:
    AudioWaveformChannelComponent ();

    ~AudioWaveformChannelComponent ();

    void initialise (OpenGLContext& openGLContext) override;

    void shutdown (OpenGLContext& openGLContext) override;

    void render (OpenGLContext& openGLContext) override;

    void redraw (int startSample, int numSamples);

    void loadBuffer (const float* readBuffer);

private:
    void updateVertices ();

private:
    std::unique_ptr<OpenGLShaderProgram> shaderProgram;
    //std::unique_ptr<VertexBuffer> vertexBuffer;
    Array<Vertex> vertices;
    int startSample;
    int numSamples;
    const float* readBuffer = nullptr;
    GLuint vertexBufferId;
    bool isVisibleRegionChanged = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformChannelComponent)
};