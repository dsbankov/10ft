/*
  ==============================================================================

    TenFtLookAndFeel.h
    Created: 6 Jun 2018 12:25:57pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class TenFtLookAndFeel : public LookAndFeel_V4
{
public:
    TenFtLookAndFeel ();

    ~TenFtLookAndFeel ();

    static const Colour getMainColour ();

};