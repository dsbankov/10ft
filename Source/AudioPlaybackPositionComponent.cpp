/*
  ==============================================================================

    AudioPlaybackPositionComponent.cpp
    Created: 29 May 2018 11:17:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "AudioPlaybackPositionComponent.h"


AudioPlaybackPositionComponent::AudioPlaybackPositionComponent (
    AudioWaveformComponent & waveform
)
    :
        waveform (waveform),
        isLooping (false)
{
    // so we can handle mouse events for the component behind it (AudioWaveformComponent)
    setInterceptsMouseClicks (false, true);

    waveform.addChangeListener (this);
}

AudioPlaybackPositionComponent::~AudioPlaybackPositionComponent ()
{
}

void AudioPlaybackPositionComponent::paint (Graphics& g)
{
    auto startTimeSeconds = waveform.getVisibleRegionStartTime (),
        endTimeSeconds = waveform.getVisibleRegionEndTime (),
        audioLengthSeconds = endTimeSeconds - startTimeSeconds;

    if (audioLengthSeconds > 0)
    {
        juce::Rectangle<float> localBounds = getLocalBounds ().toFloat ();
        auto currentPosition = waveform.getAudioSource ().getCurrentPosition (),
            drawPosition = ((currentPosition - startTimeSeconds) / audioLengthSeconds)
                * localBounds.getWidth () + localBounds.getX ();

        g.setColour (Colours::green);
        g.drawLine (
            drawPosition,
            localBounds.getY (),
            drawPosition,
            localBounds.getBottom (),
            3.0f
        );
    } 
}

void AudioPlaybackPositionComponent::stopTimer ()
{
    Timer::stopTimer ();
    repaint ();
}

void AudioPlaybackPositionComponent::setIsLooping (bool isLooping)
{
    this->isLooping = isLooping;
}

// ==============================================================================

void AudioPlaybackPositionComponent::timerCallback ()
{
    respondToChange ();
}

void AudioPlaybackPositionComponent::changeListenerCallback (
    ChangeBroadcaster *source
)
{
    if (source == &waveform)
    {
        respondToChange ();
    }
}

void AudioPlaybackPositionComponent::respondToChange ()
{
    auto currentPosition = waveform.getAudioSource ().getCurrentPosition ();

    if (
        waveform.getHasSelectedRegion () &&
        currentPosition >= waveform.getSelectedRegionEndTime ()
    )
    {
        waveform.getAudioSource ().setPosition (waveform.getSelectedRegionStartTime ());
        if (! isLooping)
        {
            waveform.getAudioSource ().pauseAudio ();
        }
    }

    repaint ();
}
