/*
  ==============================================================================

    AudioWaveformComponent.h
    Created: 20 May 2018 3:33:07pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
/*
*/

class AudioWaveformComponent : public Component, public ChangeBroadcaster, private ChangeListener
{
public:

	AudioWaveformComponent() : formatManager(), audioSource(formatManager), thumbnailCache(5), thumbnail(1024, formatManager, thumbnailCache)
	{
		thumbnail.addChangeListener(this);
	}

    ~AudioWaveformComponent()
    {
    }

	AudioFileTransportSource& getAudioSource()
	{
		return audioSource;
	}

	void paint(Graphics& g) override
	{
		Rectangle<int> thumbnailBounds = getLocalBounds();
		if (thumbnail.getNumChannels() == 0)
			paintIfNoFileLoaded(g, thumbnailBounds);
		else
			paintIfFileLoaded(g, thumbnailBounds);
	}

	void setSource(InputSource* newSource)
	{
		setDrawRange(0.0, audioSource.getLengthInSeconds());
		thumbnail.setSource(newSource);
	}

	void setDrawRange(double visibleRegionStartTimeSeconds, double visibleRegionEndTimeSeconds)
	{
		if (visibleRegionStartTimeSeconds >= visibleRegionEndTimeSeconds)
			return;
		this->visibleRegionStartTimeSeconds = flattenTime(visibleRegionStartTimeSeconds);
		this->visibleRegionEndTimeSeconds = flattenTime(visibleRegionEndTimeSeconds);
		sendChangeMessage();
		repaint();
	}

	void clear()
	{
		setDrawRange(0.0, 0.0);
		thumbnail.clear();
	}

	void clearSelectedRegion()
	{
		hasSelectedRegion = false;
		selectedRegionStartTimeSeconds = 0.0;
		selectedRegionEndTimeSeconds = 0.0;
		repaint();
	}

	void mouseWheelMove(const MouseEvent & event, const MouseWheelDetails & wheelDetails) override
	{
		if (audioSource.getLengthInSeconds() <= 0)
			return;
		auto bounds = getLocalBounds();
		auto leftRelativeAmmount = (double)(event.getMouseDownX() - bounds.getX()) / (double)bounds.getWidth();
		auto rightRelativeAmmount = 1 - leftRelativeAmmount;
		auto visibleRegionLengthInSeconds = visibleRegionEndTimeSeconds - visibleRegionStartTimeSeconds;
		const double scrollAmmount = 0.10 * visibleRegionLengthInSeconds;
		const double scrollAmmountLeft = scrollAmmount * leftRelativeAmmount;
		const double scrollAmmountRight = scrollAmmount * rightRelativeAmmount;
		if (wheelDetails.deltaY < 0) // downwards
		{
			setDrawRange(visibleRegionStartTimeSeconds - scrollAmmountLeft, visibleRegionEndTimeSeconds + scrollAmmountRight);
		}
		else if (wheelDetails.deltaY > 0) // upwards
		{
			setDrawRange(visibleRegionStartTimeSeconds + scrollAmmountLeft, visibleRegionEndTimeSeconds - scrollAmmountRight);
		}
	}

	void mouseDoubleClick(const MouseEvent& event) override
	{
		if (audioSource.getLengthInSeconds() <= 0)
			return;
		hasSelectedRegion = false;
		double newPositionSeconds = xToSeconds(event.getMouseDownX());
		audioSource.setPosition(newPositionSeconds);
		sendChangeMessage();
	}

	void mouseDrag(const MouseEvent &event) override
	{
		if (audioSource.getLengthInSeconds() <= 0)
			return;
		if (hasSelectedRegion)
		{
			double mouseDownX = event.getMouseDownX() + event.getDistanceFromDragStartX();
			double mouseDownSeconds = xToSeconds(mouseDownX);
			updateDragRegion(mouseDownSeconds);
		}
		else
		{
			hasSelectedRegion = true;
			auto startOfDragX = secondsToX(selectedRegionStartTimeSeconds, getLocalBounds());
			auto endOfDragX = startOfDragX + event.getDistanceFromDragStartX();
			selectedRegionEndTimeSeconds = xToSeconds(endOfDragX);
			audioSource.setPosition(selectedRegionStartTimeSeconds);
		}
		repaint();
	}

	void mouseDown(const MouseEvent &event) override
	{
		if (audioSource.getLengthInSeconds() <= 0)
			return;
		double mouseDownX = event.getMouseDownX();
		if (hasSelectedRegion)
		{
			auto seconds = xToSeconds(mouseDownX);
			updateDragRegion(seconds);
			repaint();
		}
		else
		{
			selectedRegionStartTimeSeconds = xToSeconds(mouseDownX);
		}
	}

	double getVisibleRegionStartTimeSeconds()
	{
		return visibleRegionStartTimeSeconds;
	}

	double getVisibleRegionEndTimeSeconds()
	{
		return visibleRegionEndTimeSeconds;
	}

	double getSelectedRegionStartTimeSeconds()
	{
		return selectedRegionStartTimeSeconds;
	}

	double getSelectedRegionEndTimeSeconds()
	{
		return selectedRegionEndTimeSeconds;
	}

	bool getHasSelectedRegion()
	{
		return hasSelectedRegion;
	}

private:

	void changeListenerCallback(ChangeBroadcaster* source) override
	{
		if (source == &thumbnail) repaint();
	}

	void paintIfNoFileLoaded(Graphics& g, Rectangle<int>& thumbnailBounds)
	{
		g.setColour(Colours::darkgrey);
		g.fillRect(thumbnailBounds);
		g.setColour(Colours::white);
		g.drawFittedText("No File Loaded", thumbnailBounds, Justification::centred, 1.0f);
	}

	void paintIfFileLoaded(Graphics& g, Rectangle<int>& thumbnailBounds)
	{
		const Colour backgroundColour = Colours::white;
		const Colour selectedRegionBackgroundColour = Colours::lightblue;
		const Colour waveformColour = Colours::red;
		if (hasSelectedRegion)
		{
			auto visibleRegionLengthSeconds = visibleRegionEndTimeSeconds - visibleRegionStartTimeSeconds;
			if (selectedRegionStartTimeSeconds > selectedRegionEndTimeSeconds)
				std::swap(selectedRegionStartTimeSeconds, selectedRegionEndTimeSeconds);
			auto startOfDragX = secondsToX(selectedRegionStartTimeSeconds, thumbnailBounds);
			auto endOfDragX = secondsToX(selectedRegionEndTimeSeconds, thumbnailBounds);
			auto notSelectedRegionLeft = thumbnailBounds.removeFromLeft(startOfDragX);
			auto selectedRegion = thumbnailBounds.removeFromLeft(endOfDragX - startOfDragX);
			auto notSelectedRegionRight = thumbnailBounds;

			g.setColour(backgroundColour);
			g.fillRect(notSelectedRegionLeft);
			g.setColour(waveformColour);
			thumbnail.drawChannels(g, notSelectedRegionLeft, visibleRegionStartTimeSeconds, selectedRegionStartTimeSeconds, 1.0f);

			g.setColour(selectedRegionBackgroundColour);
			g.fillRect(selectedRegion);
			g.setColour(waveformColour);
			thumbnail.drawChannels(g, selectedRegion, selectedRegionStartTimeSeconds, selectedRegionEndTimeSeconds, 1.0f);

			g.setColour(backgroundColour);
			g.fillRect(notSelectedRegionRight);
			g.setColour(waveformColour);
			thumbnail.drawChannels(g, notSelectedRegionRight, selectedRegionEndTimeSeconds, visibleRegionEndTimeSeconds, 1.0f);
		}
		else
		{
			g.setColour(backgroundColour);
			g.fillRect(thumbnailBounds);
			g.setColour(waveformColour);
			thumbnail.drawChannels(g, thumbnailBounds, visibleRegionStartTimeSeconds, visibleRegionEndTimeSeconds, 1.0f);
		}
	}

	double xToSeconds(double x)
	{
		auto visibleRegionLengthSeconds = visibleRegionEndTimeSeconds - visibleRegionStartTimeSeconds;
		return (x / getLocalBounds().getWidth()) * visibleRegionLengthSeconds + visibleRegionStartTimeSeconds;
	}

	double secondsToX(double s, const Rectangle<int>& thumbnailBounds)
	{
		auto visibleRegionLengthSeconds = visibleRegionEndTimeSeconds - visibleRegionStartTimeSeconds;
		return ((s - visibleRegionStartTimeSeconds) / visibleRegionLengthSeconds) * thumbnailBounds.getWidth();
	}

	void updateDragRegion(double mouseDownSeconds)
	{
		auto distanceFromStartOfDragSeconds = std::abs(mouseDownSeconds - selectedRegionStartTimeSeconds);
		auto distanceFromEndOfDragSeconds = std::abs(mouseDownSeconds - selectedRegionEndTimeSeconds);
		if (distanceFromStartOfDragSeconds < distanceFromEndOfDragSeconds)
			selectedRegionStartTimeSeconds = mouseDownSeconds;
		else
			selectedRegionEndTimeSeconds = mouseDownSeconds;
	}

	double flattenTime(double timeSeconds)
	{
		if (timeSeconds < 0)
			return 0;
		if (timeSeconds > audioSource.getLengthInSeconds())
			return audioSource.getLengthInSeconds();
		return timeSeconds;
	}

	//==============================================================================================

	AudioFormatManager formatManager;
	AudioFileTransportSource audioSource;
	AudioThumbnailCache thumbnailCache;
	AudioThumbnail thumbnail;
	double visibleRegionStartTimeSeconds = 0.0;
	double visibleRegionEndTimeSeconds = 0.0;
	bool hasSelectedRegion = false;
	double selectedRegionStartTimeSeconds = 0.0;
	double selectedRegionEndTimeSeconds = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformComponent)
};