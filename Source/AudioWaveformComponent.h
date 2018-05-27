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

	void paint(Graphics& g) override
	{
		Rectangle<int> thumbnailBounds = getLocalBounds();
		if (thumbnail.getNumChannels() == 0)
			paintIfNoFileLoaded(g, thumbnailBounds);
		else
			paintIfFileLoaded(g, thumbnailBounds);
	}

	void mouseWheelMove(const MouseEvent & event, const MouseWheelDetails & wheelDetails) override
	{
		if (audioSource.getLengthInSeconds() <= 0)
			return;
		auto bounds = getLocalBounds();
		auto leftRelativeAmmount = (double)(event.getMouseDownX() - bounds.getX()) / (double)bounds.getWidth();
		auto rightRelativeAmmount = 1 - leftRelativeAmmount;
		auto visibleRegionLengthInSeconds = visibleRegionEndTime - visibleRegionStartTime;
		const double scrollAmmount = 0.10 * visibleRegionLengthInSeconds;
		const double scrollAmmountLeft = scrollAmmount * leftRelativeAmmount;
		const double scrollAmmountRight = scrollAmmount * rightRelativeAmmount;
		if (wheelDetails.deltaY < 0) // downwards
		{
			setDrawRange(visibleRegionStartTime - scrollAmmountLeft, visibleRegionEndTime + scrollAmmountRight);
		}
		else if (wheelDetails.deltaY > 0) // upwards
		{
			setDrawRange(visibleRegionStartTime + scrollAmmountLeft, visibleRegionEndTime - scrollAmmountRight);
		}
	}

	void mouseDoubleClick(const MouseEvent& event) override
	{
		if (audioSource.getLengthInSeconds() <= 0)
			return;
		hasSelectedRegion = false;
		double newPosition = xToSeconds(event.getMouseDownX());
		audioSource.setPosition(newPosition);
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
			auto startOfDragX = secondsToX(selectedRegionStartTime, getLocalBounds());
			auto endOfDragX = startOfDragX + event.getDistanceFromDragStartX();
			selectedRegionEndTime = xToSeconds(endOfDragX);
			audioSource.setPosition(selectedRegionStartTime);
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
			selectedRegionStartTime = xToSeconds(mouseDownX);
		}
	}

	void setSource(InputSource* newSource)
	{
		setDrawRange(0.0, audioSource.getLengthInSeconds());
		thumbnail.setSource(newSource);
	}

	void setDrawRange(double visibleRegionStartTime, double visibleRegionEndTime)
	{
		if (visibleRegionStartTime >= visibleRegionEndTime)
			return;
		this->visibleRegionStartTime = flattenTime(visibleRegionStartTime);
		this->visibleRegionEndTime = flattenTime(visibleRegionEndTime);
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
		selectedRegionStartTime = 0.0;
		selectedRegionEndTime = 0.0;
		repaint();
	}

	AudioFileTransportSource& getAudioSource()
	{
		return audioSource;
	}

	double getVisibleRegionStartTime()
	{
		return visibleRegionStartTime;
	}

	double getVisibleRegionEndTime()
	{
		return visibleRegionEndTime;
	}

	double getSelectedRegionStartTime()
	{
		return selectedRegionStartTime;
	}

	double getSelectedRegionEndTime()
	{
		return selectedRegionEndTime;
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
			auto visibleRegionLengthSeconds = visibleRegionEndTime - visibleRegionStartTime;
			if (selectedRegionStartTime > selectedRegionEndTime)
				std::swap(selectedRegionStartTime, selectedRegionEndTime);
			auto startOfDragX = secondsToX(selectedRegionStartTime, thumbnailBounds);
			auto endOfDragX = secondsToX(selectedRegionEndTime, thumbnailBounds);
			auto notSelectedRegionLeft = thumbnailBounds.removeFromLeft(startOfDragX);
			auto selectedRegion = thumbnailBounds.removeFromLeft(endOfDragX - startOfDragX);
			auto notSelectedRegionRight = thumbnailBounds;

			g.setColour(backgroundColour);
			g.fillRect(notSelectedRegionLeft);
			g.setColour(waveformColour);
			thumbnail.drawChannels(g, notSelectedRegionLeft, visibleRegionStartTime, selectedRegionStartTime, 1.0f);

			g.setColour(selectedRegionBackgroundColour);
			g.fillRect(selectedRegion);
			g.setColour(waveformColour);
			thumbnail.drawChannels(g, selectedRegion, selectedRegionStartTime, selectedRegionEndTime, 1.0f);

			g.setColour(backgroundColour);
			g.fillRect(notSelectedRegionRight);
			g.setColour(waveformColour);
			thumbnail.drawChannels(g, notSelectedRegionRight, selectedRegionEndTime, visibleRegionEndTime, 1.0f);
		}
		else
		{
			g.setColour(backgroundColour);
			g.fillRect(thumbnailBounds);
			g.setColour(waveformColour);
			thumbnail.drawChannels(g, thumbnailBounds, visibleRegionStartTime, visibleRegionEndTime, 1.0f);
		}
	}

	double xToSeconds(double x)
	{
		auto visibleRegionLengthSeconds = visibleRegionEndTime - visibleRegionStartTime;
		return (x / getLocalBounds().getWidth()) * visibleRegionLengthSeconds + visibleRegionStartTime;
	}

	double secondsToX(double s, const Rectangle<int>& thumbnailBounds)
	{
		auto visibleRegionLengthSeconds = visibleRegionEndTime - visibleRegionStartTime;
		return ((s - visibleRegionStartTime) / visibleRegionLengthSeconds) * thumbnailBounds.getWidth();
	}

	void updateDragRegion(double mouseDownSeconds)
	{
		auto distanceFromStartOfDragSeconds = std::abs(mouseDownSeconds - selectedRegionStartTime);
		auto distanceFromEndOfDragSeconds = std::abs(mouseDownSeconds - selectedRegionEndTime);
		if (distanceFromStartOfDragSeconds < distanceFromEndOfDragSeconds)
			selectedRegionStartTime = mouseDownSeconds;
		else
			selectedRegionEndTime = mouseDownSeconds;
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
	double visibleRegionStartTime = 0.0;
	double visibleRegionEndTime = 0.0;
	bool hasSelectedRegion = false;
	double selectedRegionStartTime = 0.0;
	double selectedRegionEndTime = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformComponent)
};