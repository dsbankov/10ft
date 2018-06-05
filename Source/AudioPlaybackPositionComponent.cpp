/*
  ==============================================================================

    AudioPlaybackPositionComponent.cpp
    Created: 29 May 2018 11:17:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "AudioPlaybackPositionComponent.h"

AudioPlaybackPositionComponent::AudioPlaybackPositionComponent (
    TenFtAudioTransportSource& audioSource,
    float& visibleRegionStartTime,
    float& visibleRegionEndTime,
    bool& hasSelectedRegion,
    float& selectedRegionStartTime,
    float& selectedRegionEndTime
) :
    audioSource(audioSource),
    visibleRegionStartTime(visibleRegionStartTime),
    visibleRegionEndTime(visibleRegionEndTime),
    selectedRegionStartTime(selectedRegionStartTime),
    selectedRegionEndTime(selectedRegionEndTime),
    hasSelectedRegion(hasSelectedRegion)
{
    // so we can handle mouse events for the component behind it (AudioWaveformComponent)
    setInterceptsMouseClicks (false, true);
}

AudioPlaybackPositionComponent::~AudioPlaybackPositionComponent ()
{
}

void AudioPlaybackPositionComponent::paint (Graphics& g)
{
    auto visibleRegionLength = visibleRegionEndTime - visibleRegionStartTime;

    if (visibleRegionLength > 0)
    {
        auto currentPosition = audioSource.getCurrentPosition ();
        if (currentPosition < visibleRegionStartTime || currentPosition > visibleRegionEndTime)
        {
            return;
        }

        juce::Rectangle<float> localBounds = getLocalBounds ().toFloat ();
        auto drawPosition = ((currentPosition - visibleRegionStartTime) / visibleRegionLength)
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

// ==============================================================================

void AudioPlaybackPositionComponent::timerCallback ()
{
    respondToChange ();
}

void AudioPlaybackPositionComponent::changeListenerCallback (
    ChangeBroadcaster *source
)
{
    respondToChange ();
}

void AudioPlaybackPositionComponent::respondToChange ()
{
    auto currentPosition = audioSource.getCurrentPosition ();

    if (
        hasSelectedRegion &&
        currentPosition >= selectedRegionEndTime
    )
    {
        audioSource.setPosition (selectedRegionStartTime);
        if (!audioSource.isLooping ())
        {
            audioSource.pauseAudio ();
        }
    }

    repaint ();
}
