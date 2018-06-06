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


class AudioPlaybackPositionComponent :    public Component,
                                          public Timer,
                                          public ChangeListener
{
public:
    AudioPlaybackPositionComponent (
        TenFtAudioTransportSource& audioSource,
        float& visibleRegionStartTime,
        float& visibleRegionEndTime,
        bool& hasSelectedRegion,
        float& selectedRegionStartTime,
        float& selectedRegionEndTime,
        Colour& lineColour
    );

    ~AudioPlaybackPositionComponent ();

    void paint (Graphics& g) override;

    void stopTimer ();

private:
    void timerCallback () override;

    void changeListenerCallback (
        ChangeBroadcaster *source
    ) override;

    void respondToChange ();

private:
    TenFtAudioTransportSource& audioSource;
    float& visibleRegionStartTime;
    float& visibleRegionEndTime;
    float& selectedRegionStartTime;
    float& selectedRegionEndTime;
    bool& hasSelectedRegion;
    Colour& lineColour;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlaybackPositionComponent)
};
