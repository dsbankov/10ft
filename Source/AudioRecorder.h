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

    void audioDeviceAboutToStart (AudioIODevice* device) override { }

    void audioDeviceStopped () override { }

    void audioDeviceIOCallback (
        const float** inputChannelData, int numInputChannels,
        float** outputChannelData, int numOutputChannels,
        int numSamples) override;

    void loadRecordingBuffer (AudioSampleBuffer* buffer);

private:
    AudioSampleBuffer* buffer = nullptr;
    CriticalSection writerLock;

};