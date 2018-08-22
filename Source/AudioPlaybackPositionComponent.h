/*
  ==============================================================================

    TrackProgressLineComponent.h
    Created: 20 May 2018 3:33:37pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"

#include "TenFtAudioSource.h"
#include "AudioWaveformComponent.h"


class AudioPlaybackPositionComponent :    public Component,
                                          public TenFtAudioSource::Listener,
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
    void currentPositionChanged (TenFtAudioSource* audioSource) override;

    void visibleRegionChanged (AudioWaveformComponent* waveform) override;
    
    void thumbnailCleared (AudioWaveformComponent* waveform) override;

private:
    bool isAudioLoaded = false;
    bool isRecording = false;
    double visibleRegionStartTime;
    double visibleRegionEndTime;
    double currentPosition;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlaybackPositionComponent)
};
