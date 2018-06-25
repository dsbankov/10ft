/*
  ==============================================================================

    AudioWaveformSelectedRegionComponent.cpp
    Created: 25 Jun 2018 1:54:44pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "AudioWaveformSelectedRegionComponent.h"

AudioWaveformSelectedRegionComponent::AudioWaveformSelectedRegionComponent ()
{
    setInterceptsMouseClicks (false, true);
}

AudioWaveformSelectedRegionComponent::~AudioWaveformSelectedRegionComponent ()
{
}

void AudioWaveformSelectedRegionComponent::paint (Graphics& g)
{
    juce::Rectangle<float> thumbnailBounds = getLocalBounds ().toFloat ();
    bool hasIntersectionWithSelectedRegion = !(
        selectedRegionEndTime < visibleRegionStartTime ||
        selectedRegionStartTime > visibleRegionEndTime
    );

    if (hasSelectedRegion && hasIntersectionWithSelectedRegion)
    {
        float notSelectedRegionLeftWidth =
            util::flattenX (
                util::secondsToX (
                    selectedRegionStartTime,
                    visibleRegionStartTime,
                    visibleRegionEndTime,
                    thumbnailBounds
                ),
                thumbnailBounds
            ),
            notSelectedRegionRightWidth =
            util::flattenX (
                util::secondsToX (
                    selectedRegionEndTime,
                    visibleRegionStartTime,
                    visibleRegionEndTime,
                    thumbnailBounds),
                thumbnailBounds
            ),
            selectedRegionWidth =
            util::flattenX (
                notSelectedRegionRightWidth - notSelectedRegionLeftWidth,
                thumbnailBounds
            );
        juce::Rectangle<float> notSelectedRegionLeft =
                thumbnailBounds.removeFromLeft (notSelectedRegionLeftWidth),
            selectedRegion =
                thumbnailBounds.removeFromLeft (selectedRegionWidth),
            notSelectedRegionRight = thumbnailBounds;

        g.setColour (findColour (
            ColourIds::waveformSelectedRegionBackgroundColour
        ).withAlpha(0.3f));
        g.fillRect (selectedRegion);
    }
}

void AudioWaveformSelectedRegionComponent::resized ()
{
    setBounds (getLocalBounds ());
}

// ==============================================================================

void AudioWaveformSelectedRegionComponent::visibleRegionChanged (
    AudioWaveformComponent* waveform
)
{
    visibleRegionStartTime = waveform->getVisibleRegionStartTime ();
    visibleRegionEndTime = waveform->getVisibleRegionEndTime ();
    repaint ();
}

void AudioWaveformSelectedRegionComponent::selectedRegionChanged (
    AudioWaveformComponent* waveform
)
{
    hasSelectedRegion = waveform->getHasSelectedRegion ();
    selectedRegionStartTime = waveform->getSelectedRegionStartTime ();
    selectedRegionEndTime = waveform->getSelectedRegionEndTime ();
    repaint ();
}