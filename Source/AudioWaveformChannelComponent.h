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
#include "Attributes.h"


class AudioWaveformChannelComponent : public OpenGLComponent
{
public:
    enum ColourIds
    {
        waveformColour = 0,
        waveformBackgroundColour = 1
    };

public:
    AudioWaveformChannelComponent ();

    ~AudioWaveformChannelComponent ();

    void initialise (OpenGLContext& openGLContext) override;

    void shutdown (OpenGLContext& openGLContext) override;

    void render (OpenGLContext& openGLContext) override;

    void load (const float* samples);

    void display (int startSample, int numSamples);

private:
    void calculateVertices ();

private:
    std::unique_ptr<OpenGLShaderProgram> shaderProgram;
    std::unique_ptr<VertexBuffer> vertexBuffer;
    std::unique_ptr<Attributes> attributes;
    std::unique_ptr<OpenGLShaderProgram::Uniform> uniform;
    const float* samplesArray = nullptr;
    Array<Vertex> sampleVertices;
    int visibleRegionStartSample;
    int visibleRegionNumSamples;
    bool calculateVerticesTrigger = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformChannelComponent)
};