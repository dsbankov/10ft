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
                                  public ChangeListener
{
public:
    AudioScrollerComponent (AudioWaveformComponent& waveform);

    ~AudioScrollerComponent ();

    void valueChanged () override;

    void mouseDown (const MouseEvent & event) override;

    void mouseDrag (const MouseEvent & event) override;

    void mouseWheelMove (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) override;

    void disable ();

private:
    void changeListenerCallback (ChangeBroadcaster *source) override;

    void setMinAndMaxValuesWithCheck (
        double newMinValue,
        double newMaxValue,
        NotificationType notification = sendNotificationAsync
    );

    bool isMouseDownInDragRange (const MouseEvent & event);

    double offsetValue (double startX, double distanceFromStartX);

private:
    AudioWaveformComponent& waveform;
    bool dragVisibleRegion;
    double visibleRegionMinX;
    double visibleRegionMaxX;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioScrollerComponent)
};
