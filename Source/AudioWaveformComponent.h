/*
  ==============================================================================

    AudioWaveformComponent.h
    Created: 20 May 2018 3:33:07pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"


class AudioWaveformComponent :    public Component,
                                  public ChangeBroadcaster,
                                  private ChangeListener
{
public:
    AudioWaveformComponent ()
        :
            formatManager (),
            audioSource (formatManager),
            thumbnailCache (5),
            thumbnail (1024, formatManager, thumbnailCache),
            visibleRegionStartTime (0.0),
            visibleRegionEndTime (0.0),
            hasSelectedRegion (false),
            selectedRegionStartTime (0.0),
            selectedRegionEndTime (0.0)
    {
        thumbnail.addChangeListener (this);
    }

    ~AudioWaveformComponent ()
    {
    }

    void paint (Graphics& g) override
    {
        juce::Rectangle<float> thumbnailBounds = getLocalBounds ().toFloat ();

        if (thumbnail.getNumChannels () == 0)
        {
            paintIfNoFileLoaded (g, thumbnailBounds.toNearestInt ());
        }
        else
        {
            paintIfFileLoaded (g, thumbnailBounds);
        }
    }

    void mouseWheelMove (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) override
    {
        if (audioSource.getLengthInSeconds () <= 0)
        {
            return;
        }

        juce::Rectangle<float> bounds = getLocalBounds ().toFloat ();
        float leftRelativeAmmount =
                float (event.getMouseDownX () - bounds.getX ())
                    / bounds.getWidth (),
            rightRelativeAmmount = 1.0f - leftRelativeAmmount,
            visibleRegionLengthInSeconds = getVisibleRegionLengthSeconds ();

        const float scrollAmmount = 0.1f * visibleRegionLengthInSeconds,
            scrollAmmountLeft = scrollAmmount * leftRelativeAmmount,
            scrollAmmountRight = scrollAmmount * rightRelativeAmmount;

        if (wheelDetails.deltaY < 0)
        {
            // downwards

            setVisibleRegion (
                visibleRegionStartTime - scrollAmmountLeft,
                visibleRegionEndTime + scrollAmmountRight
            );
        }
        else if (wheelDetails.deltaY > 0)
        {
            // upwards
            
            setVisibleRegion (
                visibleRegionStartTime + scrollAmmountLeft,
                visibleRegionEndTime - scrollAmmountRight
            );
        }
    }

    void mouseDoubleClick (const MouseEvent& event) override
    {
        if (audioSource.getLengthInSeconds () <= 0)
        {
            return;
        }

        float newPosition = xToSeconds (event.getMouseDownX ());

        hasSelectedRegion = false;
        audioSource.setPosition (newPosition);

        sendChangeMessage ();
    }

    void mouseDrag (const MouseEvent &event) override
    {
        if (audioSource.getLengthInSeconds () <= 0)
        {
            return;
        }

        if (hasSelectedRegion)
        {
            float mouseDownX = event.getMouseDownX ()
                    + event.getDistanceFromDragStartX (),
                mouseDownSeconds = xToSeconds (mouseDownX);

            updateDragRegion (mouseDownSeconds);
        }
        else
        {
            float startOfDragX = secondsToX (
                    selectedRegionStartTime,
                    getLocalBounds ().toFloat ()
                ),
                endOfDragX = startOfDragX + event.getDistanceFromDragStartX ();

            hasSelectedRegion = true;

            selectedRegionEndTime = xToSeconds (endOfDragX);
            audioSource.setPosition (selectedRegionStartTime);
        }

        repaint ();
    }

    void mouseDown (const MouseEvent &event) override
    {
        if (audioSource.getLengthInSeconds () <= 0)
        {
            return;
        }

        float mouseDownX = event.getMouseDownX ();

        if (hasSelectedRegion)
        {
            float seconds = xToSeconds (mouseDownX);

            updateDragRegion (seconds);

            repaint ();
        }
        else
        {
            selectedRegionStartTime = xToSeconds (mouseDownX);
        }
    }

    bool loadAudio (File file)
    {
        if (audioSource.loadAudio (file))
        {
            thumbnail.setSource (new FileInputSource (file));
            setVisibleRegion (0.0, audioSource.getLengthInSeconds ());

            return true;
        }
        else
        {
            hasSelectedRegion = false;

            thumbnail.clear ();
            setVisibleRegion (0.0, 0.0);

            return false;
        }
    }

    void setVisibleRegion (
        float visibleRegionStartTime,
        float visibleRegionEndTime
    )
    {
        if (visibleRegionStartTime >= visibleRegionEndTime)
        {
            return;
        }

        this->visibleRegionStartTime = flattenTime (visibleRegionStartTime);
        this->visibleRegionEndTime = flattenTime (visibleRegionEndTime);

        sendChangeMessage ();

        repaint ();
    }

    void clearSelectedRegion ()
    {
        hasSelectedRegion = false;

        repaint ();
    }

    AudioFileTransportSource& getAudioSource ()
    {
        return audioSource;
    }

    float getVisibleRegionStartTime ()
    {
        return visibleRegionStartTime;
    }

    float getVisibleRegionEndTime ()
    {
        return visibleRegionEndTime;
    }

    float getSelectedRegionStartTime ()
    {
        return selectedRegionStartTime;
    }

    float getSelectedRegionEndTime ()
    {
        return selectedRegionEndTime;
    }

    bool getHasSelectedRegion ()
    {
        return hasSelectedRegion;
    }

private:
    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == &thumbnail)
        {
            repaint ();
        }
    }

    void paintIfNoFileLoaded (Graphics& g, juce::Rectangle<int> thumbnailBounds)
    {
        g.setColour (Colours::darkgrey);
        g.fillRect (thumbnailBounds);
        g.setColour (Colours::white);
        g.drawFittedText (
            "No File Loaded",
            thumbnailBounds,
            Justification::centred,
            1.0f
        );
    }

    void paintIfFileLoaded (Graphics& g, juce::Rectangle<float> thumbnailBounds)
    {
        const Colour backgroundColour = Colours::white,
            selectedRegionBackgroundColour = Colours::lightblue,
            waveformColour = Colours::red;

        if (hasSelectedRegion)
        {
            if (selectedRegionStartTime > selectedRegionEndTime)
            {
                std::swap (selectedRegionStartTime, selectedRegionEndTime);
            }

            float startOfDragX = secondsToX (selectedRegionStartTime, thumbnailBounds),
                endOfDragX = secondsToX (selectedRegionEndTime, thumbnailBounds);
            juce::Rectangle<float> notSelectedRegionLeft =
                    thumbnailBounds.removeFromLeft (startOfDragX),
                selectedRegion = thumbnailBounds.removeFromLeft (endOfDragX - startOfDragX),
                notSelectedRegionRight = thumbnailBounds;

            g.setColour (backgroundColour);
            g.fillRect (notSelectedRegionLeft);
            g.setColour (waveformColour);
            thumbnail.drawChannels (
                g,
                notSelectedRegionLeft.toNearestInt (),
                visibleRegionStartTime,
                selectedRegionStartTime,
                1.0f
            );

            g.setColour (selectedRegionBackgroundColour);
            g.fillRect (selectedRegion);
            g.setColour (waveformColour);
            thumbnail.drawChannels (
                g,
                selectedRegion.toNearestInt (),
                selectedRegionStartTime,
                selectedRegionEndTime,
                1.0f
            );

            g.setColour (backgroundColour);
            g.fillRect (notSelectedRegionRight);
            g.setColour (waveformColour);
            thumbnail.drawChannels (
                g,
                notSelectedRegionRight.toNearestInt (),
                selectedRegionEndTime,
                visibleRegionEndTime,
                1.0f
            );
        }
        else
        {
            g.setColour (backgroundColour);
            g.fillRect (thumbnailBounds);
            g.setColour (waveformColour);
            thumbnail.drawChannels (
                g,
                thumbnailBounds.toNearestInt (),
                visibleRegionStartTime,
                visibleRegionEndTime,
                1.0f
            );
        }
    }

    float getVisibleRegionLengthSeconds ()
    {
        return visibleRegionEndTime - visibleRegionStartTime;
    }

    float xToSeconds (float x)
    {
        float visibleRegionLengthSeconds = getVisibleRegionLengthSeconds ();

        return (x / getLocalBounds ().getWidth ()) * visibleRegionLengthSeconds
            + visibleRegionStartTime;
    }

    float secondsToX (float s, const juce::Rectangle<float> thumbnailBounds)
    {
        float visibleRegionLengthSeconds = getVisibleRegionLengthSeconds ();

        return ((s - visibleRegionStartTime) / visibleRegionLengthSeconds)
            * thumbnailBounds.getWidth ();
    }

    void updateDragRegion (float mouseDownSeconds)
    {
        float distanceFromStartOfDragSeconds =
                std::abs (mouseDownSeconds - selectedRegionStartTime),
            distanceFromEndOfDragSeconds =
                std::abs (mouseDownSeconds - selectedRegionEndTime);

        if (distanceFromStartOfDragSeconds < distanceFromEndOfDragSeconds)
        {
            selectedRegionStartTime = mouseDownSeconds;
        }
        else
        {
            selectedRegionEndTime = mouseDownSeconds;
        }
    }

    float flattenTime (float timeSeconds)
    {
        if (timeSeconds < 0.0f)
        {
            return 0.0f;
        }

        if (timeSeconds > audioSource.getLengthInSeconds ())
        {
            return audioSource.getLengthInSeconds ();
        }

        return timeSeconds;
    }

private:
    AudioFormatManager formatManager;
    AudioFileTransportSource audioSource;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    float visibleRegionStartTime;
    float visibleRegionEndTime;
    bool hasSelectedRegion;
    float selectedRegionStartTime;
    float selectedRegionEndTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformComponent)
};
