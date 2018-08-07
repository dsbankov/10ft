/*
  ==============================================================================

    AudioBufferSource.h
    Created: 6 Aug 2018 2:08:06pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AudioBufferSource :    public PositionableAudioSource
{
public:
    AudioBufferSource (
        AudioSampleBuffer* audioBuffer,
        bool shouldLoop = false
    );

    ~AudioBufferSource ();

    void getNextAudioBlock (
        const AudioSourceChannelInfo& bufferToFill
    ) override;

    void prepareToPlay (int, double) override { };

    void releaseResources () override { };

    void setNextReadPosition (int64 newPosition) override;

    int64 getNextReadPosition () const override;

    int64 getTotalLength () const override;

    bool isLooping () const override;

    void setLooping (bool shouldLoop) override;

    void setBuffer (AudioSampleBuffer* audioBuffer);
    
private:
    void copy (
        AudioSampleBuffer* dest,
        int destStartSample,
        AudioSampleBuffer* source,
        int sourceStartSample,
        int numSamples
    );

private:
    int64 position;
    bool looping;
    AudioSampleBuffer* buffer;

};