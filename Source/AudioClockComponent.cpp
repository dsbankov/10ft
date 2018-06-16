/*
  ==============================================================================

    AudioClockComponent.cpp
    Created: 29 May 2018 11:17:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "AudioClockComponent.h"


AudioClockComponent::AudioClockComponent ()
{
    addAndMakeVisible (&timeLabel);
    timeLabel.setJustificationType (
        Justification::horizontallyCentred
    );
}

AudioClockComponent::~AudioClockComponent ()
{
}

void AudioClockComponent::resized ()
{
    timeLabel.setBounds (getLocalBounds ());
}

// ==============================================================================

void AudioClockComponent::currentPositionChanged (
    TenFtAudioTransportSource* audioSource
)
{
    std::string currentPositionFormatted =
        getCurrentPositionFormatted (
            audioSource->getLengthInSeconds(),
            audioSource->getCurrentPosition()
        );
    timeLabel.setText (
        currentPositionFormatted,
        NotificationType::dontSendNotification
    );
}

std::string AudioClockComponent::getCurrentPositionFormatted (
    double lengthInSeconds, double currentPosition
)
{
    bool disabled = lengthInSeconds <= 0;

    timeLabel.setEnabled (!disabled);

    if (disabled)
    {
        return "--:--/--:--";
    }

    std::string passedTime = formatTime (currentPosition),
        entireDurationTime = formatTime (lengthInSeconds),
        currentTime = passedTime + "/" + entireDurationTime;

    return currentTime;
}

std::string AudioClockComponent::formatTime (
    double timeInSeconds
)
{
    RelativeTime relativeTime (timeInSeconds);
    int minutes = round (floor (relativeTime.inMinutes ())),
        seconds = round (
            floor (relativeTime.inSeconds ()) - (minutes * 60)
        );
    std::ostringstream time;

    time 
        << std::setw (2) << std::setfill ('0') << minutes
        << ":"
        << std::setw (2) << std::setfill ('0') << seconds;

    return time.str ();
}
