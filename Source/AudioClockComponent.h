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

#include "TenFtAudioTransportSource.h"


class AudioClockComponent :    public Component,
                               public TenFtAudioTransportSource::Listener
{
public:
    AudioClockComponent ();

    ~AudioClockComponent ();

    void resized () override;

    void currentPositionChanged (TenFtAudioTransportSource* audioSource) override;

private:
    std::string getCurrentPositionFormatted (double lengthInSeconds, double currentPosition);

    std::string formatTime (double timeInSeconds);

private:
    Label timeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioClockComponent)
};
