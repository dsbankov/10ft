/*
  ==============================================================================

    TenFtLookAndFeel.cpp
    Created: 6 Jun 2018 12:25:57pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "TenFtLookAndFeel.h"

TenFtLookAndFeel::TenFtLookAndFeel ()
{
    Colour colour = getMainColour (),
        contrastingColour = colour.contrasting (1.0f);
    setColour (TextButton::ColourIds::buttonColourId, colour);
    setColour (TextButton::ColourIds::buttonOnColourId, colour);
    setColour (TextButton::ColourIds::textColourOffId, contrastingColour);
    setColour (TextButton::ColourIds::textColourOnId, contrastingColour);
    setColour (Slider::ColourIds::backgroundColourId, contrastingColour);
    setColour (Slider::ColourIds::thumbColourId, colour.brighter());
    setColour (Slider::ColourIds::trackColourId, contrastingColour);
    setColour (Label::ColourIds::textColourId, contrastingColour);
    setColour (ToggleButton::ColourIds::textColourId, contrastingColour);
    setColour (ToggleButton::ColourIds::tickColourId, contrastingColour);
}

TenFtLookAndFeel::~TenFtLookAndFeel ()
{
}

const Colour TenFtLookAndFeel::getMainColour ()
{
    return Colours::yellow.withBrightness (0.8f).withSaturation (0.5f);
}
