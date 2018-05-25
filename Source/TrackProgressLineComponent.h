/*
  ==============================================================================

    PlaybackPositionComponent.h
    Created: 20 May 2018 3:33:37pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/
class TrackProgressLineComponent : public Component, public Timer, public ChangeListener
{
public:
	TrackProgressLineComponent(AudioWaveformComponent & waveform) : waveform(waveform)
    {
		setInterceptsMouseClicks(false, true); // so we can handle mouse events for the component behind it (AudioWaveformComponent)
		waveform.addChangeListener(this);
    }

    ~TrackProgressLineComponent()
    {
    }

    void paint (Graphics& g) override
    {
		auto startTimeSeconds = waveform.getVisibleRegionStartTimeSeconds();
		auto endTimeSeconds = waveform.getVisibleRegionEndTimeSeconds();
		auto audioLengthSeconds(endTimeSeconds - startTimeSeconds);
		if (audioLengthSeconds > 0)
		{
			Rectangle<int> localBounds(getLocalBounds());
			g.setColour(Colours::green);
			auto currentPosition(waveform.getCurrentTimeSeconds());
			auto drawPosition(((currentPosition - startTimeSeconds) / audioLengthSeconds) * localBounds.getWidth() + localBounds.getX());
			g.drawLine(drawPosition, localBounds.getY(), drawPosition, localBounds.getBottom(), 3.0f);
		} 
    }

	void stopTimer()
	{
		Timer::stopTimer();
		repaint();
	}

	void setIsLooping(bool isLooping)
	{
		this->isLooping = isLooping;
	}

private:

	AudioWaveformComponent & waveform;
	bool isLooping = false;

	void timerCallback() override
	{
		respondToChange();
	}

	void changeListenerCallback(ChangeBroadcaster *source) override
	{
		if (source == &waveform)
		{
			respondToChange();
		}
	}

	void respondToChange()
	{
		auto currentPosition(waveform.getCurrentTimeSeconds());
		if (waveform.getHasSelectedRegion() && currentPosition >= waveform.getSelectedRegionEndTimeSeconds())
		{
			if (isLooping)
				waveform.setPosition(waveform.getSelectedRegionStartTimeSeconds());
			else
				waveform.pauseAudio(waveform.getSelectedRegionStartTimeSeconds());
			return;
		}
		/*else if (!waveform.getHasSelectedRegion() && currentPosition >= waveform.getTotalLengthInSeconds() && isLooping)
		{
			waveform.playAudio(0.0);
			return;
		}*/
		repaint();
	}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrackProgressLineComponent)
};
