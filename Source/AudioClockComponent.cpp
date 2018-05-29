/*
  ==============================================================================

    AudioClockComponent.cpp
    Created: 29 May 2018 11:17:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "AudioClockComponent.h"


AudioClockComponent::AudioClockComponent (
    AudioWaveformComponent& waveform
)
    :
        waveform (waveform)
{
    addAndMakeVisible (&timeLabel);
    timeLabel.setJustificationType (
        Justification::horizontallyCentred
    );
    updateText ();
    waveform.addChangeListener (this);
}

AudioClockComponent::~AudioClockComponent ()
{
}

void AudioClockComponent::resized ()
{
    timeLabel.setBounds (getLocalBounds ());
}

void AudioClockComponent::stopTimer ()
{
    Timer::stopTimer ();
    updateText ();
}

// ==============================================================================

void AudioClockComponent::timerCallback ()
{
    updateText ();
}

void AudioClockComponent::changeListenerCallback (
    ChangeBroadcaster *source
)
{
    if (source == &waveform)
    {
        updateText ();
    }
}

void AudioClockComponent::updateText ()
{
    std::string currentPositionFormatted =
        getCurrentPositionFormatted (waveform.getAudioSource ());
    timeLabel.setText (
        currentPositionFormatted,
        NotificationType::dontSendNotification
    );
}

std::string AudioClockComponent::getCurrentPositionFormatted (
    AudioTransportSource& audioSource
)
{
    auto lengthInSeconds = audioSource.getLengthInSeconds (),
      currentPosition = audioSource.getCurrentPosition ();

    if (lengthInSeconds <= 0)
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
