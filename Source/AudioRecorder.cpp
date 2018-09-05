/*
  ==============================================================================

    AudioRecorder.cpp
    Created: 4 Sep 2018 3:14:28pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "AudioRecorder.h"

void AudioRecorder::audioDeviceIOCallback (
    const float** inputChannelData, int numInputChannels,
    float** outputChannelData, int numOutputChannels,
    int numSamples
)
{
    const ScopedLock sl (writerLock);

    if (buffer == nullptr)
    {
        return;
    }

    int destStartSample = buffer->getNumSamples ();
    buffer->setSize (buffer->getNumChannels (),
        buffer->getNumSamples () + numSamples, true);

    for (int i = 0, bufferChannel = 0; i < numInputChannels; ++i)
    {
        if (inputChannelData[i] != nullptr)
        {
            buffer->copyFrom (bufferChannel++, destStartSample,
                inputChannelData[i], numSamples);
        }
    }

    for (int i = 0; i < numOutputChannels; ++i)
        if (outputChannelData[i] != nullptr)
            FloatVectorOperations::clear (outputChannelData[i], numSamples);
}

void AudioRecorder::loadRecordingBuffer (AudioSampleBuffer* buffer)
{
    this->buffer = buffer;
}