/*
  ==============================================================================

    AudioPlaybackPositionComponent.cpp
    Created: 29 May 2018 11:17:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "AudioPlaybackPositionComponent.h"

AudioPlaybackPositionComponent::AudioPlaybackPositionComponent ()
{
    setInterceptsMouseClicks (false, true);
}

AudioPlaybackPositionComponent::~AudioPlaybackPositionComponent ()
{
}

void AudioPlaybackPositionComponent::paint (Graphics& g)
{
    if (isAudioLoaded)
    {
        if (currentPosition < visibleRegionStartTime ||
            currentPosition > visibleRegionEndTime)
        {
            return;
        }

        juce::Rectangle<float> localBounds = getLocalBounds ().toFloat ();
        float drawPosition = (float) (
                (currentPosition - visibleRegionStartTime) /
                (visibleRegionEndTime - visibleRegionStartTime)
            )
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

void AudioPlaybackPositionComponent::resized ()
{
    setBounds (getLocalBounds());
}

// ==============================================================================

void AudioPlaybackPositionComponent::currentPositionChanged (
    TenFtAudioTransportSource* audioSource
)
{
    currentPosition = audioSource->getCurrentPositionGlobal ();
    repaint ();
}

void AudioPlaybackPositionComponent::visibleRegionChanged (
    AudioWaveformComponent* waveform
)
{
    isAudioLoaded = true;
    visibleRegionStartTime = waveform->getVisibleRegionStartTime ();
    visibleRegionEndTime = waveform->getVisibleRegionEndTime ();
    repaint ();
}

void AudioPlaybackPositionComponent::thumbnailCleared (
    AudioWaveformComponent*
)
{
    isAudioLoaded = false;
    repaint ();
}
