/*
  ==============================================================================

    AudioRecorder.cpp
    Created: 4 Sep 2018 3:14:28pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "AudioRecorder.h"

void AudioRecorder::audioDeviceAboutToStart (AudioIODevice* device)
{
    // TODO determine number of channels?
    // TODO determine preallocated memory? currently = 5 seconds
    sampleRate = device->getCurrentSampleRate ();
    preallocatedBuffer.setSize (1, (int)(5 * sampleRate));

    bufferPreallocationThread.reset (
        new BufferPreallocationThread (
            preallocatedBuffer,
            numSamplesUsed,
            (int)(3 * sampleRate),
            (int)(5 * sampleRate)
        )
    );
    bufferPreallocationThread->startThread ();
}

void AudioRecorder::audioDeviceStopped ()
{
    bufferPreallocationThread->stopThread (1000);
}

void AudioRecorder::audioDeviceIOCallback (
    const float** inputChannelData, int numInputChannels,
    float** outputChannelData, int numOutputChannels,
    int numSamples
)
{
    for (int inputChannel = 0, bufferChannel = 0;
        inputChannel < numInputChannels;
        inputChannel++)
    {
        if (inputChannelData[inputChannel] != nullptr)
        {
            preallocatedBuffer.copyFrom (bufferChannel++, numSamplesUsed,
                inputChannelData[inputChannel], numSamples);
        }
    }

    numSamplesUsed += numSamples;

    buffer->setDataToReferTo (
        preallocatedBuffer.getArrayOfWritePointers (),
        preallocatedBuffer.getNumChannels (),
        numSamplesUsed
    );

    for (int i = 0; i < numOutputChannels; ++i)
        if (outputChannelData[i] != nullptr)
            FloatVectorOperations::clear (outputChannelData[i], numSamples);
}

void AudioRecorder::loadRecordingBuffer (AudioSampleBuffer* recordingBuffer)
{
    buffer = recordingBuffer;
}

// ==============================================================================

AudioRecorder::BufferPreallocationThread::BufferPreallocationThread (
    AudioSampleBuffer& preallocatedBuffer,
    int& numSamplesUsed,
    int numSamplesUnusedLimit,
    int numSamplesAllocation
) :
    Thread ("BufferWriterThread"),
    preallocatedBuffer (preallocatedBuffer),
    numSamplesUsed (numSamplesUsed),
    numSamplesUnusedLimit(numSamplesUnusedLimit),
    numSamplesAllocation(numSamplesAllocation)
{
}

void AudioRecorder::BufferPreallocationThread::run ()
{
    while (!threadShouldExit ())
    {
        int preallocatedSamples = preallocatedBuffer.getNumSamples ();
        if (preallocatedSamples - numSamplesUsed < numSamplesUnusedLimit)
        {
            int newNumSamples =
                preallocatedBuffer.getNumSamples () + numSamplesAllocation;
            preallocatedBuffer.setSize (preallocatedBuffer.getNumChannels (),
                newNumSamples, true);
        }
    }
}
