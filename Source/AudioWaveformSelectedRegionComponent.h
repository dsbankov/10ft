/*
  ==============================================================================

    AudioWaveformSelectedRegionComponent.h
    Created: 25 Jun 2018 1:54:44pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once


#include "../JuceLibraryCode/JuceHeader.h"

#include "AudioWaveformComponent.h"
#include "TenFtUtil.h"


class AudioWaveformSelectedRegionComponent : public Component,
                                             public AudioWaveformComponent::Listener
{
public:
    enum ColourIds
    {
        waveformSelectedRegionBackgroundColour = 3
    };

public:
    AudioWaveformSelectedRegionComponent ();

    ~AudioWaveformSelectedRegionComponent ();

    void paint (Graphics& g) override;

    void resized () override;

private:
    void visibleRegionChanged (AudioWaveformComponent* waveform) override;

    void selectedRegionChanged (AudioWaveformComponent* waveform) override;

private:
    bool hasSelectedRegion = false;
    double selectedRegionStartTime;
    double selectedRegionEndTime;
    double visibleRegionStartTime;
    double visibleRegionEndTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformSelectedRegionComponent)
};
