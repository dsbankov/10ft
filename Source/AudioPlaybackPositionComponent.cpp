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

        g.setColour (findColour (ColourIds::lineColour));
        g.drawLine (
            drawPosition,
            localBounds.getY (),
            drawPosition,
            localBounds.getBottom (),
            2.0f
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
    repaint ();
}
