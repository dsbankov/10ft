/*
  ==============================================================================

    TrackProgressLabelComponent.h
    Created: 20 May 2018 5:48:09pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

//==============================================================================
/*
*/
class TrackProgressLabelComponent : public Component, public Timer, public ChangeListener
{
public:

    TrackProgressLabelComponent(AudioTransportSource& audioSource) : audioSource(audioSource)
    {
		addAndMakeVisible(&timeLabel);
		timeLabel.setJustificationType(Justification::horizontallyCentred);
		updateText();
		audioSource.addChangeListener(this);
    }

    ~TrackProgressLabelComponent()
    {
    }

    void resized() override
    {
		timeLabel.setBounds(getLocalBounds());
    }

	void stopTimer()
	{
		Timer::stopTimer();
		updateText();
	}

private:

	Label timeLabel;
	AudioTransportSource& audioSource;

	void timerCallback() override
	{
		updateText();
	}

	void changeListenerCallback(ChangeBroadcaster *source) override
	{
		if (source == &audioSource)
		{
			updateText();
		}
	}

	void updateText()
	{
		std::string currentPositionFormatted = getCurrentPositionFormatted(audioSource);
		timeLabel.setText(currentPositionFormatted, NotificationType::dontSendNotification);
	}

	static std::string getCurrentPositionFormatted(AudioTransportSource& audioSource)
	{
		auto lengthInSeconds(audioSource.getLengthInSeconds());
		auto currentPosition(audioSource.getCurrentPosition());
		if (lengthInSeconds <= 0)
			return "--:--/--:--";
		std::string passedTime = formatTime(currentPosition);
		std::string entireDurationTime = formatTime(lengthInSeconds);
		std::string currentTime = passedTime + "/" + entireDurationTime;
		return currentTime;
	}

	static std::string formatTime(double timeInSeconds)
	{
		RelativeTime relativeTime(timeInSeconds);
		int minutes = round(floor(relativeTime.inMinutes()));
		int seconds = round(floor(relativeTime.inSeconds()) - (minutes * 60));
		std::ostringstream time;
		time << std::setw(2) << std::setfill('0') << minutes << ":" << std::setw(2) << std::setfill('0') << seconds;
		std::string currentTime = time.str();
		return currentTime;
	}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackProgressLabelComponent)
};
