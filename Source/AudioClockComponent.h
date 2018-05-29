/*
  ==============================================================================

    TrackProgressLabelComponent.h
    Created: 20 May 2018 5:48:09pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "../JuceLibraryCode/JuceHeader.h"

#include "AudioWaveformComponent.h"


class AudioClockComponent :    public Component,
                               public Timer,
                               public ChangeListener
{
public:
    AudioClockComponent (AudioWaveformComponent& waveform);

    ~AudioClockComponent ();

    void resized () override;

    void stopTimer ();

private:
    void timerCallback () override;

    void changeListenerCallback (
        ChangeBroadcaster *source
    ) override;

    void updateText ();

    static std::string getCurrentPositionFormatted (
        AudioTransportSource& audioSource
    );

    static std::string formatTime (double timeInSeconds);

private:
    Label timeLabel;
    AudioWaveformComponent& waveform;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioClockComponent)
};
