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


//==============================================================================
/*
!!! CURRENTLY NOT WOKRING !!!
Draws a single audio channel using OpenGL. The idea was to add multiple such
components for every audio channel as children of a parent component. The
parent component had a single OpenGLContext attached and dispatched the OpenGL
rendering callbacks back to these components. Couldn't get it to work because
somehow only one of the channels gets drawn.
*/
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

    void display (int startSample, int numSamples);

private:
    void calculateVertices (unsigned int channel);

private:
    struct Vertex {
        GLfloat x, y;
    };

    class VertexBuffer
    {
    public:
        VertexBuffer (OpenGLContext& openGLContext);

        ~VertexBuffer ();

        void bind (Array<Vertex>& buffer);

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
    const float** samplesPerChannel;
    unsigned int numChannels;
    int64 lengthInSamples;
    OwnedArray<Array<Vertex>> verticesPerChannel;
    int visibleRegionStartSample;
    int visibleRegionNumSamples;
    bool calculateVerticesTrigger = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformOpenGLComponent)
};