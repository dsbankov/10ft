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
    looping (shouldLoop),
    buffer (audioBuffer)
{
}

AudioBufferSource::~AudioBufferSource ()
{
    buffer = nullptr;
}

void AudioBufferSource::getNextAudioBlock (
    const AudioSourceChannelInfo& info
)
{
    if (info.numSamples > 0)
    {
        const int64 start = position;

        if (looping)
        {
            const int newStart = start % buffer->getNumSamples(),
                newEnd = (start + info.numSamples) % buffer->getNumSamples ();

            if (newEnd > newStart)
            {
                copy (
                    info.buffer,
                    info.startSample,
                    buffer,
                    newStart,
                    (int) (newEnd - newStart)
                );
            }
            else
            {
                const int endSamps = (int) (
                    buffer->getNumSamples () - newStart
                );

                copy (
                    info.buffer,
                    info.startSample,
                    buffer,
                    newStart,
                    endSamps
                );

                copy (
                    info.buffer,
                    info.startSample + endSamps,
                    buffer,
                    0,
                    (int) newEnd
                );
            }

            position = newEnd;
        }
        else if (start < buffer->getNumSamples ())
        {
            copy (
                info.buffer,
                info.startSample,
                buffer,
                (int) start,
                jmin (info.numSamples, (int) (buffer->getNumSamples () - start))
            );

            position += info.numSamples;
        }
    }
}

void AudioBufferSource::setNextReadPosition (int64 newPosition)
{
    position = newPosition;
}

int64 AudioBufferSource::getNextReadPosition () const
{
    return looping ? position % buffer->getNumSamples () : position;
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

void AudioBufferSource::copy (
    AudioSampleBuffer* dest,
    int destStartSample,
    AudioSampleBuffer* source,
    int sourceStartSample,
    int numSamples
)
{
    for (int channel = 0; channel < source->getNumChannels (); channel++)
    {
        dest->copyFrom (
            channel,
            destStartSample,
            *source,
            channel,
            sourceStartSample,
            numSamples
        );
    }
}
