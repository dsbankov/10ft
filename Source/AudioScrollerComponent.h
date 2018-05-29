/*
  ==============================================================================

    TrackScrollerComponent.h
    Created: 23 May 2018 11:54:11am
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"


class AudioScrollerComponent :    public Slider,
                                  public ChangeListener
{
public:
    AudioScrollerComponent (AudioWaveformComponent& waveform)
        :
            Slider (),
            waveform (waveform),
            dragEntireRange (false),
            startOfDragMinX (0.0),
            startOfDragMaxX (0.0)
    {
        setRange (0.0, 100.0, 0.1);
        setSliderStyle (Slider::TwoValueHorizontal);
        setMinAndMaxValuesWithCheck (
            0.0,
            100.0,
            NotificationType::dontSendNotification
        );
        setTextBoxStyle (Slider::NoTextBox, false, 0, 0);
        setChangeNotificationOnlyOnRelease (false);
        setEnabled (false);

        startOfDragMinX = getMinimum ();
        startOfDragMaxX = getMaximum ();

        waveform.addChangeListener (this);
    }

    ~AudioScrollerComponent ()
    {
    }

    void valueChanged () override
    {
        double minValue = getMinValue (),
            maxValue = getMaxValue ();

        if (minValue >= maxValue)
        {
            std::swap (minValue, maxValue);
            setMinAndMaxValuesWithCheck (minValue, maxValue);
            return;
        }

        double lengthInSeconds = waveform.getAudioSource ().getLengthInSeconds (),
            leftPositionSeconds = (minValue / 100.0) * lengthInSeconds,
            rightPositionSeconds = (maxValue / 100.0) * lengthInSeconds;

        waveform.setVisibleRegion (leftPositionSeconds, rightPositionSeconds);
    }

    void mouseDown (const MouseEvent & event) override
    {
        if (isEnabled () && isMouseDownInDragRange (event))
        {
            dragEntireRange = true;
            startOfDragMinX = valueToProportionOfLength (getMinValue ())
                * getLocalBounds ().getWidth ();
            startOfDragMaxX = valueToProportionOfLength (getMaxValue ())
                * getLocalBounds ().getWidth ();
        }
        else
        {
            dragEntireRange = false;
            Slider::mouseDown (event);
        }
    }

    void mouseDrag (const MouseEvent & event) override
    {
        if (isEnabled () && dragEntireRange)
        {
            double distanceFromStartX = event.getDistanceFromDragStartX (),
                newMinValue = offsetValue (startOfDragMinX, distanceFromStartX),
                newMaxValue = offsetValue (startOfDragMaxX, distanceFromStartX);

            setMinAndMaxValuesWithCheck (newMinValue, newMaxValue);
        }
        else
        {
            Slider::mouseDrag (event);
        }
    }

    void mouseWheelMove (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) override
    {
        waveform.mouseWheelMove (event, wheelDetails);
    }

    void disable ()
    {
        setMinAndMaxValuesWithCheck (
            0.0,
            100.0,
            NotificationType::dontSendNotification
        );
        setEnabled (false);
    }

private:
    void changeListenerCallback (ChangeBroadcaster *source) override
    {
        if (source == &waveform)
        {
            if (waveform.getAudioSource ().getLengthInSeconds () <= 0)
            {
                setMinAndMaxValuesWithCheck (
                    0.0,
                    100.0,
                    NotificationType::dontSendNotification
                );
            }
            else
            {
                double minValue = waveform.getVisibleRegionStartTime ()
                        / waveform.getAudioSource ().getLengthInSeconds (),
                    maxValue = waveform.getVisibleRegionEndTime ()
                        / waveform.getAudioSource ().getLengthInSeconds ();

                setMinAndMaxValuesWithCheck (
                    minValue * 100.0,
                    maxValue * 100.0,
                    NotificationType::dontSendNotification
                );
            }
        }
    }

    void setMinAndMaxValuesWithCheck (
        double newMinValue,
        double newMaxValue,
        NotificationType notification = sendNotificationAsync
    )
    {
        setEnabled (waveform.getAudioSource ().getLengthInSeconds () > 0);
        setMinAndMaxValues (newMinValue, newMaxValue, notification);
    }

    bool isMouseDownInDragRange (const MouseEvent & event)
    {
        double proportionOfLength = ((double) event.getMouseDownX ())
                / ((double) getWidth ()),
            value = proportionOfLengthToValue (proportionOfLength),
            space = 1.0;

        return value > getMinValue () + space && value < getMaxValue () - space;
    }

    double offsetValue (double startX, double distanceFromStartX)
    {
        double proportionX = (startX + distanceFromStartX) / getWidth ();

        if (proportionX > 1)
        {
            proportionX = 1.0;
        }

        if (proportionX < 0)
        {
            proportionX = 0.0;
        }

        return proportionOfLengthToValue (proportionX);
    }

private:
    AudioWaveformComponent& waveform;
    bool dragEntireRange;
    double startOfDragMinX;
    double startOfDragMaxX;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioScrollerComponent)
};
