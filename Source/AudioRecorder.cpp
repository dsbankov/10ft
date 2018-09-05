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
    preallocatedBuffer.setSize (1, device->getCurrentBufferSizeSamples ());
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
    // TODO use the same strategy for the main buffer?
    AudioSampleBuffer* tmpBuffer = new AudioSampleBuffer(
        preallocatedBuffer.getArrayOfWritePointers (),
        preallocatedBuffer.getNumChannels(),
        numSamples
    );

    for (int inputChannel = 0, bufferChannel = 0;
        inputChannel < numInputChannels;
        inputChannel++)
    {
        if (inputChannelData[inputChannel] != nullptr)
        {
            tmpBuffer->copyFrom (bufferChannel++, 0,
                inputChannelData[inputChannel], numSamples);
        }
    }

    bufferWriterThread->addBufferToAppend (
        tmpBuffer
    );

    for (int i = 0; i < numOutputChannels; ++i)
        if (outputChannelData[i] != nullptr)
            FloatVectorOperations::clear (outputChannelData[i], numSamples);
}

void AudioRecorder::loadRecordingBuffer (AudioSampleBuffer* recordingBuffer)
{
    bufferWriterThread.reset (new BufferWriterThread (recordingBuffer));
}

// ==============================================================================

AudioRecorder::BufferWriterThread::BufferWriterThread (AudioSampleBuffer* buffer)
    : Thread ("BufferWriterThread"), buffer (buffer), samplesRead(0)
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

    int allNumSamples = samplesRead + bufferToAppend->getNumSamples ();
    if (buffer->getNumSamples () < allNumSamples)
    {
        buffer->setSize (buffer->getNumChannels (),
            allNumSamples * 2, true);
    }

    for (int channel = 0; channel < bufferToAppend->getNumChannels (); ++channel)
    {
        buffer->copyFrom (channel, samplesRead,
            *bufferToAppend, channel, 0, bufferToAppend->getNumSamples ());
    }

    samplesRead += bufferToAppend->getNumSamples ();
}
