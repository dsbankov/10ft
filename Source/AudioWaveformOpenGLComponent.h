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

    void load (
        AudioSampleBuffer* buffer, const CriticalSection* bufferUpdateLock
    );

    void display (
        AudioSampleBuffer* buffer, int64 startSample, int64 numSamples
    );

    void refresh (AudioSampleBuffer* buffer);

private:
    void calculateVertices (AudioSampleBuffer* buffer);

    void calculateVertices (AudioSampleBuffer* buffer, unsigned int channel);

    GLfloat getAverageSampleValue (
        const float* samples, int64 startSample, int64 numSamples
    );

    GLfloat getPeakSampleValue (
        const float* samples, int64 startSample, int64 numSamples
    );

private:
    struct Vertex;
    class VertexBuffer;

private:
    std::unique_ptr<OpenGLShaderProgram> shaderProgram;
    std::unique_ptr<OpenGLShaderProgram::Attribute> position;
    std::unique_ptr<OpenGLShaderProgram::Uniform> uniform;
    std::unique_ptr<VertexBuffer> vertexBuffer;

    int bufferNumChannels = 0;
    int64 visibleRegionStartSample = 0;
    int64 visibleRegionNumSamples = 0;
    unsigned int skipSamples = 8;

    Array<Array<Vertex, CriticalSection>> vertices;

    const CriticalSection* bufferUpdateLock_ = nullptr;

private:
    struct Vertex
    {
        GLfloat x, y;
    };

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

    class ScopedNullableLock
    {
    public:
        inline explicit ScopedNullableLock (const CriticalSection* lock) noexcept
            : lock_(lock)
        {
            if (lock_ != nullptr)
            {
                lock_->enter ();
            }
        }

        inline ~ScopedNullableLock () noexcept
        {
            if (lock_ != nullptr)
            {
                lock_->exit ();
            }
        }

    private:
        const CriticalSection* lock_ = nullptr;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformOpenGLComponent)
};