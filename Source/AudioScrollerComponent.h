/*
  ==============================================================================

    TrackScrollerComponent.h
    Created: 23 May 2018 11:54:11am
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"

#include "AudioWaveformComponent.h"


class AudioScrollerComponent :    public Slider,
                                  public AudioWaveformComponent::Listener
{
public:
    std::function<void (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    )> onMouseWheelMove;

public:
    AudioScrollerComponent ();

    ~AudioScrollerComponent ();

    void mouseDown (const MouseEvent& event) override;

    void mouseDrag (const MouseEvent& event) override;

    void mouseWheelMove (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) override;

    void visibleRegionChanged (AudioWaveformComponent* waveform) override;

    void disable ();

private:
    void setMinAndMaxValuesWithCheck (
        double newMinValue,
        double newMaxValue,
        NotificationType notification = sendNotificationAsync
    );

    bool isMouseDownInDragRange (const MouseEvent & event);

    double offsetValue (double startX, double distanceFromStartX);

private:
    bool dragVisibleRegion;
    double visibleRegionMinX;
    double visibleRegionMaxX;
    double totalLength;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioScrollerComponent)
};
