/*
  ==============================================================================

    AudioRecorder.h
    Created: 4 Sep 2018 3:14:28pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AudioRecorder : public AudioIODeviceCallback
{
public:
    AudioRecorder () { }

    ~AudioRecorder () { }

    void audioDeviceAboutToStart (AudioIODevice* device) override;

    void audioDeviceStopped () override;

    void audioDeviceIOCallback (
        const float** inputChannelData, int numInputChannels,
        float** outputChannelData, int numOutputChannels,
        int numSamples
    ) override;

    void loadRecordingBuffer (AudioSampleBuffer* recordingBuffer);

private:
    class BufferPreallocationThread;

private:
    AudioSampleBuffer* buffer = nullptr;
    AudioSampleBuffer preallocatedBuffer;
    int numSamplesUsed = 0;
    double sampleRate = 0.0;
    std::unique_ptr<BufferPreallocationThread> bufferPreallocationThread;

private:
    class BufferPreallocationThread : public Thread
    {
    public:
        BufferPreallocationThread (
            AudioSampleBuffer& preallocatedBuffer,
            int& numSamplesUsed,
            int numSamplesUnusedLimit,
            int numSamplesAllocation
        );

        void run () override;

    private:
        AudioSampleBuffer& preallocatedBuffer;
        int& numSamplesUsed;
        int numSamplesUnusedLimit;
        int numSamplesAllocation;
    };
};