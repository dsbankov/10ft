/*
  ==============================================================================

    TrackProgressSliderComponent.h
    Created: 20 May 2018 5:48:36pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class TrackProgressSliderComponent : public Component, public Timer
{
public:
    TrackProgressSliderComponent(AudioTransportSource& audioSource) : audioSource(audioSource)
    {
		addAndMakeVisible(&timeSlider);
		timeSlider.setRange(0.0, 100.0, 1.0);
		timeSlider.setSliderStyle(Slider::LinearHorizontal);
		timeSlider.setTextBoxStyle(Slider::NoTextBox, false, 0, 0);
		timeSlider.setChangeNotificationOnlyOnRelease(true);
		timeSlider.setEnabled(false);
		timeSlider.onDragStart = [this] { onTimeSliderDragStart(); };
		timeSlider.onDragEnd = [this] { onTimeSliderDragEnd(); };
    }

    ~TrackProgressSliderComponent()
    {
    }

    void resized() override
    {
		timeSlider.setBounds(getLocalBounds());
    }

	void stopTimer()
	{
		Timer::stopTimer();
		updateValue();
	}

	void setEnabled(bool enabled)
	{
		timeSlider.setEnabled(enabled);
	}

private:

	Slider timeSlider;
	AudioTransportSource& audioSource;
	bool timeSliderDragStartOn = false;

	void timerCallback() override
	{
		updateValue();
	}

	void updateValue()
	{
		if (!timeSliderDragStartOn)
		{
			double currentPositionPercentage = getSliderValue(audioSource);
			timeSlider.setValue(currentPositionPercentage, NotificationType::dontSendNotification);
		}
	}

	void onTimeSliderDragStart()
	{
		timeSliderDragStartOn = true;
	}

	void onTimeSliderDragEnd()
	{
		double newSliderValue = timeSlider.getValue();
		double lengthInSeconds = audioSource.getLengthInSeconds();
		double newPosition = (newSliderValue / 100) * lengthInSeconds;
		audioSource.setPosition(newPosition);
		timeSliderDragStartOn = false;
	}

	static double getSliderValue(AudioTransportSource& audioSource)
	{
		double lengthInSeconds = audioSource.getLengthInSeconds();
		if (lengthInSeconds <= 0)
			return 0.0;
		double currentPositionSeconds = audioSource.getCurrentPosition();
		double sliderValue = (currentPositionSeconds / lengthInSeconds) * 100;
		return sliderValue;
	}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackProgressSliderComponent)
};
