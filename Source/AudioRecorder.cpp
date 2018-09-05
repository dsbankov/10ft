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
    bufferWriterThread->startThread ();
}

void AudioRecorder::audioDeviceStopped ()
{
    bufferWriterThread->stopThread (500);
}

void AudioRecorder::audioDeviceIOCallback (
    const float** inputChannelData, int numInputChannels,
    float** outputChannelData, int numOutputChannels,
    int numSamples
)
{
    const ScopedLock sl (writerLock);

    bufferWriterThread->addBufferToAppend (
        createBuffer(inputChannelData, numInputChannels, numSamples)
    );

    for (int i = 0; i < numOutputChannels; ++i)
        if (outputChannelData[i] != nullptr)
            FloatVectorOperations::clear (outputChannelData[i], numSamples);
}

void AudioRecorder::loadRecordingBuffer (AudioSampleBuffer* recordingBuffer)
{
    buffer = recordingBuffer;
    bufferWriterThread.reset (new BufferWriterThread (buffer));
}

// ==============================================================================

AudioSampleBuffer* AudioRecorder::createBuffer (
    const float** inputChannelData, int numInputChannels, int numSamples
)
{
    AudioSampleBuffer* bufferToAppend =
        new AudioSampleBuffer (buffer->getNumChannels (), numSamples);
    for (int inputChannel = 0, bufferChannel = 0;
        inputChannel < numInputChannels; ++inputChannel)
    {
        if (inputChannelData[inputChannel] != nullptr)
        {
            bufferToAppend->copyFrom (bufferChannel++, 0,
                inputChannelData[inputChannel], numSamples);
        }
    }
    return bufferToAppend;
}

// ==============================================================================

AudioRecorder::BufferWriterThread::BufferWriterThread (AudioSampleBuffer* buffer)
    : Thread ("BufferWriterThread"), buffer (buffer)
{
}

void AudioRecorder::BufferWriterThread::run ()
{
    while (!threadShouldExit ())
    {
        for (int i = buffersToAppend.size () - 1; i >= 0; i--)
        {
            appendToBuffer (buffersToAppend.getUnchecked (i));
            buffersToAppend.remove (i);
        }
    }
}

void AudioRecorder::BufferWriterThread::addBufferToAppend (
    AudioSampleBuffer* bufferToAppend
)
{
    buffersToAppend.insert (0, bufferToAppend);
}

// ==============================================================================

void AudioRecorder::BufferWriterThread::appendToBuffer (
    AudioSampleBuffer* bufferToAppend
)
{
    int destStartSample = buffer->getNumSamples ();
    buffer->setSize (buffer->getNumChannels (),
        buffer->getNumSamples () + bufferToAppend->getNumSamples (), true);

    for (int channel = 0; channel < bufferToAppend->getNumChannels (); ++channel)
    {
        buffer->copyFrom (channel, destStartSample,
            *bufferToAppend, channel, 0, bufferToAppend->getNumSamples ());
    }
}
