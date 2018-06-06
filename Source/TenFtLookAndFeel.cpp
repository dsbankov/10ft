/*
  ==============================================================================

    TenFtLookAndFeel.cpp
    Created: 6 Jun 2018 12:25:57pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "TenFtLookAndFeel.h"
#include "AudioWaveformComponent.h"

TenFtLookAndFeel::TenFtLookAndFeel ()
{
    Colour mainColour = Colours::yellow.withBrightness (0.8f).withSaturation (0.5f),
        contrastingColour = mainColour.contrasting (1.0f);
    setColour (TextButton::ColourIds::buttonColourId, mainColour.brighter ());
    setColour (TextButton::ColourIds::buttonOnColourId, mainColour.brighter ());
    setColour (TextButton::ColourIds::textColourOffId, contrastingColour);
    setColour (TextButton::ColourIds::textColourOnId, contrastingColour);
    setColour (Slider::ColourIds::backgroundColourId, mainColour.contrasting(0.3f));
    setColour (Slider::ColourIds::thumbColourId, mainColour.contrasting (0.8f));
    setColour (Slider::ColourIds::trackColourId, mainColour);
    setColour (Label::ColourIds::textColourId, contrastingColour);
    setColour (ToggleButton::ColourIds::textColourId, contrastingColour);
    setColour (ToggleButton::ColourIds::tickColourId, contrastingColour);

    int waveformColourId =
            AudioWaveformComponent::ColourIds::waveformColour,
        waveformBackgroundColourId =
            AudioWaveformComponent::ColourIds::waveformBackgroundColour,
        waveformSelectedRegionBackgroundColourId =
            AudioWaveformComponent::ColourIds::waveformSelectedRegionBackgroundColour,
        lineColourId =
            AudioPlaybackPositionComponent::ColourIds::lineColour;

    setColour (waveformColourId, contrastingColour);
    setColour (waveformBackgroundColourId, mainColour.contrasting (0.2f));
    setColour (waveformSelectedRegionBackgroundColourId, mainColour.contrasting (0.4f));
    setColour (lineColourId,
        Colour::contrasting (
            Colour::contrasting (
                findColour (waveformColourId),
                findColour (waveformBackgroundColourId)
            ),
            findColour (waveformSelectedRegionBackgroundColourId)
        ).overlaidWith (Colours::red.withAlpha (0.8f)).withAlpha (0.8f)
    );
}

TenFtLookAndFeel::~TenFtLookAndFeel ()
{
}