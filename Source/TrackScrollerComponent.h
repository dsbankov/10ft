/*
  ==============================================================================

    TrackScrollerComponent.h
    Created: 23 May 2018 11:54:11am
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class TrackScrollerComponent    : public Slider, public ChangeListener
{
public:

    TrackScrollerComponent(AudioWaveformComponent& waveform) : Slider(), waveform(waveform)
    {
		setRange(0.0, 100.0, 0.1);
		setSliderStyle(Slider::TwoValueHorizontal);
		setMinAndMaxValuesWithCheck(0.0, 100.0, NotificationType::dontSendNotification);
		setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
		setChangeNotificationOnlyOnRelease(false);
		setEnabled(false);
		startOfDragMinX = getMinimum();
		startOfDragMaxX = getMaximum();
		waveform.addChangeListener(this);
    }

    ~TrackScrollerComponent()
    {
    }

	void valueChanged() override
	{
		double minValue = getMinValue();
		double maxValue = getMaxValue();
		double lengthInSeconds = waveform.getTotalLengthInSeconds();
		double leftPositionSeconds = (minValue / 100.0) * lengthInSeconds;
		double rightPositionSeconds = (maxValue / 100.0) * lengthInSeconds;
		waveform.setDrawRange(leftPositionSeconds, rightPositionSeconds);
	}

	void mouseDown(const MouseEvent & event) override
	{
		if (isEnabled() && isMouseDownInDragRange(event))
		{
			dragEntireRange = true;
			startOfDragMinX = valueToProportionOfLength(getMinValue()) * getWidth();
			startOfDragMaxX = valueToProportionOfLength(getMaxValue()) * getWidth();
		}
		else
		{
			dragEntireRange = false;
			Slider::mouseDown(event);
		}
	}

	void mouseDrag(const MouseEvent & event) override
	{
		if (isEnabled() && dragEntireRange)
		{
			double distanceFromStartX = event.getDistanceFromDragStartX();
			auto newMinValue = offsetValue(startOfDragMinX, distanceFromStartX);
			auto newMaxValue = offsetValue(startOfDragMaxX, distanceFromStartX);
			setMinAndMaxValuesWithCheck(newMinValue, newMaxValue);
		}
		else
		{
			Slider::mouseDrag(event);
		}
	}

private:

	void changeListenerCallback(ChangeBroadcaster *source) override
	{
		if (source == &waveform)
		{
			if (waveform.getTotalLengthInSeconds() <= 0)
			{
				setMinAndMaxValuesWithCheck(0.0, 100.0, NotificationType::dontSendNotification);
			}
			else
			{
				double minValue = waveform.getVisibleRegionStartTimeSeconds() / waveform.getTotalLengthInSeconds();
				double maxValue = waveform.getVisibleRegionEndTimeSeconds() / waveform.getTotalLengthInSeconds();
				setMinAndMaxValuesWithCheck(minValue * 100, maxValue * 100, NotificationType::dontSendNotification);
			}
		}
	}

	void setMinAndMaxValuesWithCheck(double	newMinValue, double	newMaxValue, NotificationType notification = sendNotificationAsync)
	{
		setEnabled(waveform.getTotalLengthInSeconds() > 0);
		Slider::setMinAndMaxValues(newMinValue, newMaxValue, notification);
	}

	bool isMouseDownInDragRange(const MouseEvent & event)
	{
		double proportionOfLength = ((double) event.getMouseDownX()) / ((double) getWidth());
		auto value = proportionOfLengthToValue(proportionOfLength);
		auto space = 2;
		return value > getMinValue() + space && value < getMaxValue() - space;
	}

	double offsetValue(double startX, double distanceFromStartX)
	{
		auto proportionX = (startX + distanceFromStartX) / getWidth();
		if (proportionX > 1) proportionX = 1.0;
		if (proportionX < 0) proportionX = 0.0;
		return proportionOfLengthToValue(proportionX);
	}

	//==============================================================================================

	AudioWaveformComponent& waveform;
	bool dragEntireRange = false;
	double startOfDragMinX;
	double startOfDragMaxX;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackScrollerComponent)
};
