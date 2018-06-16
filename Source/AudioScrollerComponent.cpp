/*
  ==============================================================================

    AudioScrollerComponent.cpp
    Created: 29 May 2018 11:15:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "AudioScrollerComponent.h"


AudioScrollerComponent::AudioScrollerComponent () :
    dragVisibleRegion (false),
    visibleRegionMinX (0.0),
    visibleRegionMaxX (0.0)
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

    visibleRegionMinX = getMinimum ();
    visibleRegionMaxX = getMaximum ();
}

AudioScrollerComponent::~AudioScrollerComponent ()
{
}

void AudioScrollerComponent::mouseDown (const MouseEvent & event)
{
    if (!isEnabled ())
    {
        return;
    }

    dragVisibleRegion = isMouseDownInDragRange (event);
    visibleRegionMinX = valueToProportionOfLength (getMinValue ())
        * getLocalBounds ().getWidth ();
    visibleRegionMaxX = valueToProportionOfLength (getMaxValue ())
        * getLocalBounds ().getWidth ();

    Slider::mouseDown (event);
}

void AudioScrollerComponent::mouseDrag (const MouseEvent & event)
{
    if (!isEnabled ())
    {
        return;
    }

    double distanceFromMouseDownX = event.getDistanceFromDragStartX (),
        newMinValue = offsetValue (
            visibleRegionMinX,
            distanceFromMouseDownX
        ),
        newMaxValue = offsetValue (
            visibleRegionMaxX,
            distanceFromMouseDownX
        );

    if (dragVisibleRegion)
    {
        setMinAndMaxValuesWithCheck (newMinValue, newMaxValue);
    }
    else
    {
        double mouseDownXBeforeDrag = event.getMouseDownX (),
            minValueMouseDownXProximity =
                std::abs (visibleRegionMinX - mouseDownXBeforeDrag),
            maxValueMouseDownXProximity =
                std::abs (visibleRegionMaxX - mouseDownXBeforeDrag),
            distancePassed = std::abs (distanceFromMouseDownX),
            distanceAllowed = visibleRegionMaxX - visibleRegionMinX;

        if (minValueMouseDownXProximity < maxValueMouseDownXProximity)
        {
            if (distanceFromMouseDownX > 0 && distancePassed >= distanceAllowed)
            {
                return;
            }
            setMinAndMaxValuesWithCheck (newMinValue, getMaxValue());
        }
        else
        {
            if (distanceFromMouseDownX < 0 && distancePassed >= distanceAllowed)
            {
                return;
            }
            setMinAndMaxValuesWithCheck (getMinValue(), newMaxValue);
        }

    }
}

void AudioScrollerComponent::mouseWheelMove (
    const MouseEvent& event,
    const MouseWheelDetails& wheelDetails
)
{
    onMouseWheelMove (event, wheelDetails);
}

void AudioScrollerComponent::visibleRegionChanged (
    AudioWaveformComponent* waveform
)
{
    totalLength = waveform->getTotalLength ();
    if (totalLength <= 0)
    {
        setMinAndMaxValuesWithCheck (
            0.0,
            100.0,
            NotificationType::dontSendNotification
        );
    }
    else
    {
        double minValue = waveform->getVisibleRegionStartTime ()
            / totalLength,
            maxValue = waveform->getVisibleRegionEndTime ()
            / totalLength;

        setMinAndMaxValuesWithCheck (
            minValue * 100.0,
            maxValue * 100.0,
            NotificationType::dontSendNotification
        );
    }
}

void AudioScrollerComponent::disable ()
{
    setMinAndMaxValuesWithCheck (
        0.0,
        100.0,
        NotificationType::dontSendNotification
    );
    setEnabled (false);
}

// ==============================================================================

void AudioScrollerComponent::setMinAndMaxValuesWithCheck (
    double newMinValue,
    double newMaxValue,
    NotificationType notification
)
{
    setEnabled (totalLength > 0);
    setMinAndMaxValues (newMinValue, newMaxValue, notification);
}

bool AudioScrollerComponent::isMouseDownInDragRange (
    const MouseEvent& event
)
{
    double proportionOfLength = ((double) event.getMouseDownX ())
            / ((double) getWidth ()),
        value = proportionOfLengthToValue (proportionOfLength),
        space = 1.0;

    return value > (getMinValue () + space)
        && value < (getMaxValue () - space);
}

double AudioScrollerComponent::offsetValue (
    double startX,
    double distanceFromStartX
)
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
