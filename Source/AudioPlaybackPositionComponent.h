/*
  ==============================================================================

    TrackProgressLineComponent.h
    Created: 20 May 2018 3:33:37pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"

#include "TenFtAudioTransportSource.h"
#include "AudioWaveformComponent.h"


class AudioPlaybackPositionComponent :    public Component,
                                          public TenFtAudioTransportSource::Listener,
                                          public AudioWaveformComponent::Listener
{
public:
    enum ColourIds
    {
        lineColour = 4
    };

public:
    AudioPlaybackPositionComponent ();

    ~AudioPlaybackPositionComponent ();

    void paint (Graphics& g) override;

    void resized () override;

private:
    void currentPositionChanged (TenFtAudioTransportSource* audioSource) override;

    void visibleRegionChanged (AudioWaveformComponent* waveform) override;
    
    void thumbnailCleared (AudioWaveformComponent* waveform) override;

private:
    bool isAudioLoaded = false;
    double visibleRegionStartTime;
    double visibleRegionEndTime;
    double currentPosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlaybackPositionComponent)
};
