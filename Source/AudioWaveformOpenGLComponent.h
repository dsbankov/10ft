/*
  ==============================================================================

    AudioWaveformOpenGLComponent.h
    Created: 9 Jul 2018 12:16:15pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "OpenGLComponent.h"


class AudioWaveformOpenGLComponent : public OpenGLComponent
{
public:
    enum ColourIds
    {
        waveformColour = 0,
        waveformBackgroundColour = 1
    };

public:
    AudioWaveformOpenGLComponent ();

    ~AudioWaveformOpenGLComponent ();

    void initialise (OpenGLContext& openGLContext) override;

    void shutdown (OpenGLContext& openGLContext) override;

    void render (OpenGLContext& openGLContext) override;

    void load (AudioFormatReader* reader);

    void display (int64 displayStartSample, int64 displayNumSamples);

private:
    void calculateVertices (unsigned int channel);

    void clearVertices ();

    bool areVerticesCleared ();

private:
    struct Vertex { GLfloat x, y; };

    class VertexBuffer
    {
    public:
        VertexBuffer (OpenGLContext& openGLContext);

        ~VertexBuffer ();

        void bind (Vertex* vertices, int64 verticesCount);

        void unbind ();

    private:
        OpenGLContext & openGLContext;
        GLuint id;

    };

private:
    std::unique_ptr<OpenGLShaderProgram> shaderProgram;
    std::unique_ptr<OpenGLShaderProgram::Attribute> position;
    std::unique_ptr<OpenGLShaderProgram::Uniform> uniform;
    std::unique_ptr<VertexBuffer> vertexBuffer;

    AudioBuffer<float> audioBuffer;
    int64 startSample;
    int64 numSamples;
    unsigned int skipSamples = 8;

    Vertex** vertices = nullptr;
    bool calculateVerticesTrigger = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformOpenGLComponent)
};