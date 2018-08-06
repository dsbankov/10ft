/*
  ==============================================================================

    AudioBufferSource.cpp
    Created: 6 Aug 2018 2:08:06pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "AudioBufferSource.h"

AudioBufferSource::AudioBufferSource (
    AudioSampleBuffer* audioBuffer,
    bool shouldLoop
) :
    position (0),
    start (0),
    looping (shouldLoop),
    buffer (audioBuffer)
{
}

AudioBufferSource::~AudioBufferSource ()
{
    buffer = nullptr;
}

void AudioBufferSource::getNextAudioBlock (
    const AudioSourceChannelInfo& bufferToFill
)
{
    int bufferSamples = buffer->getNumSamples (),
        bufferChannels = buffer->getNumChannels ();

    if (bufferToFill.numSamples > 0) {

        int startSample = position,
            numSamplesToCopy = 0;

        if (startSample + bufferToFill.numSamples <= bufferSamples)
        {
            numSamplesToCopy = bufferToFill.numSamples;
        }
        else if (startSample > bufferSamples)
        {
            numSamplesToCopy = 0;
        }
        else if (bufferSamples - startSample > 0)
        {
            numSamplesToCopy = bufferSamples - startSample;
        }
        else
        {
            numSamplesToCopy = 0;
        }

        if (numSamplesToCopy > 0)
        {
            for (int channel = 0; channel < bufferChannels; channel++)
            {
                bufferToFill.buffer->copyFrom (
                    channel,
                    bufferToFill.startSample,
                    *buffer,
                    channel,
                    startSample,
                    numSamplesToCopy
                );
            }

            position += numSamplesToCopy;
        }

        if (looping && position == bufferSamples)
        {
            position = 0;
        }

    }
}

void AudioBufferSource::setNextReadPosition (int64 newPosition)
{
    if (newPosition >= 0 && newPosition < buffer->getNumSamples ())
    {
        position = newPosition;
    }
}

int64 AudioBufferSource::getNextReadPosition () const
{
    return position;
}

int64 AudioBufferSource::getTotalLength () const
{
    return buffer->getNumSamples ();
}

bool AudioBufferSource::isLooping () const
{
    return looping;
}

void AudioBufferSource::setLooping (bool shouldLoop)
{
    looping = shouldLoop;
}

void AudioBufferSource::setBuffer (AudioSampleBuffer* audioBuffer)
{
    buffer = audioBuffer;
    setNextReadPosition (0);
}