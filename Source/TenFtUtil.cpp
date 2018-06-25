/*
  ==============================================================================

    TenFtUtil.cpp
    Created: 25 Jun 2018 4:24:33pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "TenFtUtil.h"

namespace util
{

    double xToSeconds (
        float x,
        double visibleRegionStartTime,
        double visibleRegionEndTime,
        juce::Rectangle<float> bounds
    )
    {
        double visibleRegionLengthSeconds =
            visibleRegionEndTime - visibleRegionStartTime;

        return (
                ((double) x / (double) bounds.getWidth ()) *
                visibleRegionLengthSeconds
            ) +
            visibleRegionStartTime;
    }

    float secondsToX (
        double s,
        double visibleRegionStartTime,
        double visibleRegionEndTime,
        juce::Rectangle<float> bounds
    )
    {
        double visibleRegionLengthSeconds =
            visibleRegionEndTime - visibleRegionStartTime;

        return (float) (
                (s - visibleRegionStartTime) / visibleRegionLengthSeconds
            ) *
            bounds.getWidth ();
    }

    float flattenX (
        float x,
        juce::Rectangle<float> bounds
    )
    {
        if (x < 0)
        {
            return 0;
        }
        if (x > bounds.getWidth ())
        {
            return bounds.getWidth ();
        }
        return x;
    }

    double flattenSeconds (double s, double totalLength)
    {
        if (s < 0.0f)
        {
            return 0.0f;
        }
        if (s > totalLength)
        {
            return totalLength;
        }
        return s;
    }

}
