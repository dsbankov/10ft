/*
  ==============================================================================

    TenFtUtil.h
    Created: 25 Jun 2018 4:24:33pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once


#include "../JuceLibraryCode/JuceHeader.h"


namespace util
{

    double xToSeconds (
        float x,
        double visibleRegionStartTime,
        double visibleRegionEndTime,
        juce::Rectangle<float> bounds
    );

    float secondsToX (
        double s,
        double visibleRegionStartTime,
        double visibleRegionEndTime,
        juce::Rectangle<float> bounds
    );

    float flattenX (
        float x,
        juce::Rectangle<float> bounds
    );

    double flattenSeconds (
        double s,
        double totalLength
    );

}