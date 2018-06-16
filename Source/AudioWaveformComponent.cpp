/*
  ==============================================================================

    AudioWaveformComponent.cpp
    Created: 29 May 2018 11:13:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "AudioWaveformComponent.h"


AudioWaveformComponent::AudioWaveformComponent ()
    :
        formatManager (),
        thumbnailCache (5),
        thumbnail (2048, formatManager, thumbnailCache),
        openGLContext()
{
    formatManager.registerBasicFormats ();
    openGLContext.attachTo (*this);
    thumbnail.addChangeListener (this);
}

AudioWaveformComponent::~AudioWaveformComponent ()
{
    thumbnail.clear ();
    openGLContext.detach ();
}

void AudioWaveformComponent::paint (Graphics& g)
{
    if (thumbnail.getNumChannels () == 0)
    {
        paintIfNoFileLoaded (g);
    }
    else
    {
        paintIfFileLoaded (g);
    }
}

void AudioWaveformComponent::addListener (Listener * newListener)
{
    listeners.add (newListener);
}

void AudioWaveformComponent::removeListener (Listener * listener)
{
    listeners.remove (listener);
}

void AudioWaveformComponent::mouseWheelMove (
    const MouseEvent& event,
    const MouseWheelDetails& wheelDetails
)
{
    if (thumbnail.getTotalLength () <= 0)
    {
        return;
    }

    juce::Rectangle<float> bounds = getLocalBounds ().toFloat ();
    float leftRelativeAmmount =
            float (event.getMouseDownX () - bounds.getX ())
                / bounds.getWidth (),
        rightRelativeAmmount = 1.0f - leftRelativeAmmount,
        visibleRegionLengthInSeconds = getVisibleRegionLengthInSeconds ();

    const float scrollAmmount = 0.1f * visibleRegionLengthInSeconds,
        scrollAmmountLeft = scrollAmmount * leftRelativeAmmount,
        scrollAmmountRight = scrollAmmount * rightRelativeAmmount;

    if (wheelDetails.deltaY < 0)
    {
        // downwards
        
        updateVisibleRegion (
            visibleRegionStartTime - scrollAmmountLeft,
            visibleRegionEndTime + scrollAmmountRight
        );
    }
    else if (wheelDetails.deltaY > 0)
    {
        // upwards
        
        updateVisibleRegion (
            visibleRegionStartTime + scrollAmmountLeft,
            visibleRegionEndTime - scrollAmmountRight
        );
    }
}

void AudioWaveformComponent::mouseDoubleClick (const MouseEvent& event)
{
    if (thumbnail.getTotalLength () <= 0)
    {
        return;
    }

    float newPosition = xToSeconds (event.getMouseDownX ());

    clearSelectedRegion ();
    
    onPositionChange (newPosition);
}

void AudioWaveformComponent::mouseDrag (const MouseEvent &event)
{
    if (thumbnail.getTotalLength () <= 0)
    {
        return;
    }

    if (hasSelectedRegion)
    {
        float mouseDownX = event.getMouseDownX ()
                + event.getDistanceFromDragStartX (),
            mouseDownSeconds = xToSeconds (mouseDownX);

        updateSelectedRegion (mouseDownSeconds);
    }
    else
    {
        float startOfDragX = secondsToX (selectedRegionStartTime),
            endOfDragX = startOfDragX + event.getDistanceFromDragStartX (),
            newStartTime = selectedRegionStartTime,
            newEndTime = xToSeconds (endOfDragX);

        hasSelectedRegion = true;

        // swap the values when we drag left
        if (newStartTime > newEndTime)
        {
            std::swap (newStartTime, newEndTime);
        }

        updateSelectedRegion (newStartTime, newEndTime);

        onPositionChange (newStartTime);
    }

    repaint ();
}

void AudioWaveformComponent::mouseDown (const MouseEvent &event)
{
    if (thumbnail.getTotalLength () <= 0)
    {
        return;
    }

    float mouseDownX = event.getMouseDownX ();

    if (hasSelectedRegion)
    {
        float seconds = xToSeconds (mouseDownX);

        updateSelectedRegion (seconds);

        repaint ();
    }
    else
    {
        setSelectedRegionStartTime (xToSeconds (mouseDownX));
    }
}

void AudioWaveformComponent::sliderValueChanged (Slider* slider)
{
    double minValue = slider->getMinValue (),
        maxValue = slider->getMaxValue ();

    if (minValue == maxValue)
    {
        return;
    }

    double leftPositionSeconds = (minValue / 100.0) * getTotalLength (),
        rightPositionSeconds = (maxValue / 100.0) * getTotalLength ();

    updateVisibleRegion (leftPositionSeconds, rightPositionSeconds);
}

bool AudioWaveformComponent::loadThumbnail (File file)
{
    if (thumbnail.setSource (new FileInputSource (file)))
    {
        updateVisibleRegion (0.0f, thumbnail.getTotalLength());
        return true;
    }
    else
    {
        clearThumbnail ();
        return false;
    }
}

void AudioWaveformComponent::clearThumbnail ()
{
    thumbnail.clear ();
    clearSelectedRegion ();
    updateVisibleRegion (0.0f, 0.0f);
    listeners.call ([this] (Listener& l) { l.thumbnailCleared (this); });
}

float AudioWaveformComponent::getTotalLength ()
{
    return thumbnail.getTotalLength ();
}

void AudioWaveformComponent::updateVisibleRegion (
    float newStartTime,
    float newEndTime
)
{
    float startTimeFlattened = flattenSeconds (newStartTime),
        endTimeFlattened = flattenSeconds (newEndTime);

    if (!isVisibleRegionCorrect(startTimeFlattened, endTimeFlattened))
    {
        throw std::logic_error (
            "Incorrect visible region [" +
                std::to_string(newStartTime) + ", " +
                std::to_string (newEndTime) + "].");
    }

    if (endTimeFlattened - startTimeFlattened < 0.05f)
    {
        return;
    }

    visibleRegionStartTime = startTimeFlattened;
    visibleRegionEndTime = endTimeFlattened;

    listeners.call ([this] (Listener& l) { l.visibleRegionChanged (this); });

    repaint ();
}

void AudioWaveformComponent::updateSelectedRegion (
    float newStartTime, float newEndTime
)
{
    selectedRegionStartTime = newStartTime;
    selectedRegionEndTime = newEndTime;

    listeners.call ([this] (Listener& l) { l.selectedRegionChanged (this); });
}

void AudioWaveformComponent::clearSelectedRegion ()
{
    hasSelectedRegion = false;

    listeners.call ([this] (Listener& l) { l.selectedRegionChanged (this); });
}

float AudioWaveformComponent::getVisibleRegionStartTime ()
{
    return visibleRegionStartTime;
}

float AudioWaveformComponent::getVisibleRegionEndTime ()
{
    return visibleRegionEndTime;
}

float AudioWaveformComponent::getSelectedRegionStartTime ()
{
    return selectedRegionStartTime;
}

float AudioWaveformComponent::getSelectedRegionEndTime ()
{
    return selectedRegionEndTime;
}

bool AudioWaveformComponent::getHasSelectedRegion ()
{
    return hasSelectedRegion;
}

// ==============================================================================

void AudioWaveformComponent::changeListenerCallback (
    ChangeBroadcaster* source
)
{
    if (source == &thumbnail)
    {
        repaint ();
    }
}

void AudioWaveformComponent::paintIfNoFileLoaded (Graphics& g)
{
    juce::Rectangle<float> thumbnailBounds = getLocalBounds ().toFloat ();
    g.setColour (findColour(ColourIds::waveformBackgroundColour));
    g.fillRect (thumbnailBounds);
    g.setColour (findColour (ColourIds::waveformColour));
    g.drawFittedText (
        "No File Loaded",
        thumbnailBounds.toNearestInt (),
        Justification::centred,
        1.0f
    );
}

void AudioWaveformComponent::paintIfFileLoaded (Graphics& g)
{
    juce::Rectangle<float> thumbnailBounds = getLocalBounds ().toFloat ();
    bool hasIntersectionWithSelectedRegion = !(
        selectedRegionEndTime < visibleRegionStartTime ||
        selectedRegionStartTime > visibleRegionEndTime
    );

    if (hasSelectedRegion && hasIntersectionWithSelectedRegion)
    {
        float notSelectedRegionLeftWidth =
                flattenX (secondsToX (selectedRegionStartTime)),
            notSelectedRegionRightWidth =
                flattenX (secondsToX (selectedRegionEndTime)),
            selectedRegionWidth =
                flattenX (
                    notSelectedRegionRightWidth - notSelectedRegionLeftWidth
                );
        juce::Rectangle<float> notSelectedRegionLeft =
                thumbnailBounds.removeFromLeft (notSelectedRegionLeftWidth),
            selectedRegion =
                thumbnailBounds.removeFromLeft (selectedRegionWidth),
            notSelectedRegionRight = thumbnailBounds;

        if (visibleRegionStartTime < selectedRegionStartTime)
        {
            g.setColour (findColour (ColourIds::waveformBackgroundColour));
            g.fillRect (notSelectedRegionLeft);
            g.setColour (findColour (ColourIds::waveformColour));
            thumbnail.drawChannels (
                g,
                notSelectedRegionLeft.toNearestInt (),
                visibleRegionStartTime,
                selectedRegionStartTime,
                1.0f
            );
        }

        g.setColour (findColour (
            ColourIds::waveformSelectedRegionBackgroundColour
        ));
        g.fillRect (selectedRegion);
        g.setColour (findColour (ColourIds::waveformColour));
        thumbnail.drawChannels (
            g,
            selectedRegion.toNearestInt (),
            jmax(selectedRegionStartTime, visibleRegionStartTime),
            jmin(selectedRegionEndTime, visibleRegionEndTime),
            1.0f
        );

        if (selectedRegionEndTime < visibleRegionEndTime)
        {
            g.setColour (findColour (ColourIds::waveformBackgroundColour));
            g.fillRect (notSelectedRegionRight);
            g.setColour (findColour (ColourIds::waveformColour));
            thumbnail.drawChannels (
                g,
                notSelectedRegionRight.toNearestInt (),
                selectedRegionEndTime,
                visibleRegionEndTime,
                1.0f
            );
        }
    }
    else
    {
        g.setColour (findColour (ColourIds::waveformBackgroundColour));
        g.fillRect (thumbnailBounds);
        g.setColour (findColour (ColourIds::waveformColour));
        thumbnail.drawChannels (
            g,
            thumbnailBounds.toNearestInt (),
            visibleRegionStartTime,
            visibleRegionEndTime,
            1.0f
        );
    }
}

void AudioWaveformComponent::setSelectedRegionStartTime (float newStartTime)
{
    updateSelectedRegion (newStartTime, selectedRegionEndTime);
}

void AudioWaveformComponent::setSelectedRegionEndTime (float newEndTime)
{
    updateSelectedRegion (selectedRegionStartTime, newEndTime);
}

float AudioWaveformComponent::getVisibleRegionLengthInSeconds ()
{
    return visibleRegionEndTime - visibleRegionStartTime;
}

void AudioWaveformComponent::updateSelectedRegion (float mouseDownSeconds)
{
    float distanceFromStartOfDragSeconds =
            std::abs (mouseDownSeconds - selectedRegionStartTime),
        distanceFromEndOfDragSeconds =
            std::abs (mouseDownSeconds - selectedRegionEndTime);

    if (distanceFromStartOfDragSeconds < distanceFromEndOfDragSeconds)
    {
        setSelectedRegionStartTime (mouseDownSeconds);
    }
    else
    {
        setSelectedRegionEndTime (mouseDownSeconds);
    }

}

bool AudioWaveformComponent::isVisibleRegionCorrect (
    float visibleRegionStartTime,
    float visibleRegionEndTime)
{
    bool isAudioLoaded = thumbnail.getTotalLength () > 0.0;
    return
        (!isAudioLoaded &&
            visibleRegionStartTime == 0.0f && visibleRegionEndTime == 0.0f) ||
        (isAudioLoaded &&
            visibleRegionStartTime < visibleRegionEndTime &&
            visibleRegionStartTime >= 0 &&
            visibleRegionEndTime <= thumbnail.getTotalLength ());
}

float AudioWaveformComponent::xToSeconds (float x)
{
    float visibleRegionLengthSeconds = getVisibleRegionLengthInSeconds ();

    return (x / getLocalBounds ().getWidth ()) * visibleRegionLengthSeconds
        + visibleRegionStartTime;
}

float AudioWaveformComponent::secondsToX (float s)
{
    juce::Rectangle<float> thumbnailBounds = getLocalBounds ().toFloat ();
    float visibleRegionLengthSeconds = getVisibleRegionLengthInSeconds ();

    return ((s - visibleRegionStartTime) / visibleRegionLengthSeconds)
        * thumbnailBounds.getWidth ();
}

float AudioWaveformComponent::flattenSeconds (float s)
{
    if (s < 0.0f)
    {
        return 0.0f;
    }

    if (s > thumbnail.getTotalLength ())
    {
        return thumbnail.getTotalLength ();
    }

    return s;
}

float AudioWaveformComponent::flattenX (float x)
{
    if (x < 0)
    {
        return 0;
    }
    if (x > getLocalBounds ().getWidth ())
    {
        return getLocalBounds ().getWidth ();
    }
    return x;
}