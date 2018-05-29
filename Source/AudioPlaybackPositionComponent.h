/*
  ==============================================================================

    TrackProgressLineComponent.h
    Created: 20 May 2018 3:33:37pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"

#include "AudioWaveformComponent.h"


class AudioPlaybackPositionComponent :    public Component,
                                          public Timer,
                                          public ChangeListener
{
public:
    AudioPlaybackPositionComponent (
        AudioWaveformComponent & waveform
    );

    ~AudioPlaybackPositionComponent ();

    void paint (Graphics& g) override;

    void stopTimer ();

    void setIsLooping (bool isLooping);

private:
    void timerCallback () override;

    void changeListenerCallback (
        ChangeBroadcaster *source
    ) override;

    void respondToChange ();

private:
    AudioWaveformComponent& waveform;
    bool isLooping;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlaybackPositionComponent)
};
