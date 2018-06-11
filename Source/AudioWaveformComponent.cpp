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
        audioSource (formatManager),
        thumbnailCache (5),
        thumbnail (2048, formatManager, thumbnailCache),
        visibleRegionStartTime (0.0),
        visibleRegionEndTime (0.0),
        hasSelectedRegion (false),
        selectedRegionStartTime (0.0),
        selectedRegionEndTime (0.0),
        playbackPosition (audioSource, visibleRegionStartTime,
            visibleRegionEndTime, hasSelectedRegion,
            selectedRegionStartTime, selectedRegionEndTime),
        openGLContext()
{
    openGLContext.attachTo (*this);
    thumbnail.addChangeListener (this);

    addAndMakeVisible (&playbackPosition);
    addChangeListener (&playbackPosition);
}

AudioWaveformComponent::~AudioWaveformComponent ()
{
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

void AudioWaveformComponent::resized ()
{
    playbackPosition.setBounds (getLocalBounds ());
}

void AudioWaveformComponent::mouseWheelMove (
    const MouseEvent& event,
    const MouseWheelDetails& wheelDetails
)
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
    if (audioSource.getLengthInSeconds () <= 0)
    {
        return;
    }

    float newPosition = xToSeconds (event.getMouseDownX ());

    hasSelectedRegion = false;
    audioSource.setPosition (newPosition);

    sendChangeMessage ();
}

void AudioWaveformComponent::mouseDrag (const MouseEvent &event)
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

        updateSelectedRegion (mouseDownSeconds);
    }
    else
    {
        float startOfDragX = secondsToX (selectedRegionStartTime),
            endOfDragX = startOfDragX + event.getDistanceFromDragStartX ();

        hasSelectedRegion = true;
        selectedRegionEndTime = xToSeconds (endOfDragX);

        // when we drag left
        if (selectedRegionStartTime > selectedRegionEndTime)
        {
            std::swap (selectedRegionStartTime, selectedRegionEndTime);
        }

        audioSource.setPosition (selectedRegionStartTime);
    }

    repaint ();
}

void AudioWaveformComponent::mouseDown (const MouseEvent &event)
{
    if (audioSource.getLengthInSeconds () <= 0)
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
        selectedRegionStartTime = xToSeconds (mouseDownX);
    }
}

bool AudioWaveformComponent::loadAudio (File file)
{
    if (audioSource.loadAudio (file))
    {
        thumbnail.setSource (new FileInputSource (file));
        updateVisibleRegion (0.0f, audioSource.getLengthInSeconds ());

        return true;
    }
    else
    {
        hasSelectedRegion = false;

        thumbnail.clear ();
        updateVisibleRegion (0.0f, 0.0f);

        return false;
    }
}

void AudioWaveformComponent::updateVisibleRegion (
    float visibleRegionStartTime,
    float visibleRegionEndTime
)
{
    float startTimeFlattened = flattenSeconds (visibleRegionStartTime),
        endTimeFlattened = flattenSeconds (visibleRegionEndTime);

    if (!isVisibleRegionCorrect(startTimeFlattened, endTimeFlattened))
    {
        throw std::logic_error (
            "Incorrect visible region [" +
                std::to_string(visibleRegionStartTime) + ", " +
                std::to_string (visibleRegionEndTime) + "].");
    }

    if (endTimeFlattened - startTimeFlattened < 0.05f)
    {
        return;
    }

    this->visibleRegionStartTime = startTimeFlattened;
    this->visibleRegionEndTime = endTimeFlattened;

    sendChangeMessage ();

    repaint ();
}

void AudioWaveformComponent::clearSelectedRegion ()
{
    hasSelectedRegion = false;

    repaint ();
}

TenFtAudioTransportSource& AudioWaveformComponent::getAudioSource ()
{
    return audioSource;
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

AudioPlaybackPositionComponent& AudioWaveformComponent::getPlaybackPositionComponent ()
{
    return playbackPosition;
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

        g.setColour (findColour (ColourIds::waveformSelectedRegionBackgroundColour));
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

float AudioWaveformComponent::getVisibleRegionLengthSeconds ()
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
        selectedRegionStartTime = mouseDownSeconds;
    }
    else
    {
        selectedRegionEndTime = mouseDownSeconds;
    }
}

bool AudioWaveformComponent::isVisibleRegionCorrect (
    float visibleRegionStartTime,
    float visibleRegionEndTime)
{
    bool isAudioLoaded = audioSource.isAudioLoaded ();
    return
        (!isAudioLoaded &&
            visibleRegionStartTime == 0.0f && visibleRegionEndTime == 0.0f) ||
        (isAudioLoaded &&
            visibleRegionStartTime < visibleRegionEndTime &&
            visibleRegionStartTime >= 0 &&
            visibleRegionEndTime <= audioSource.getLengthInSeconds ());
}

float AudioWaveformComponent::xToSeconds (float x)
{
    float visibleRegionLengthSeconds = getVisibleRegionLengthSeconds ();

    return (x / getLocalBounds ().getWidth ()) * visibleRegionLengthSeconds
        + visibleRegionStartTime;
}

float AudioWaveformComponent::secondsToX (float s)
{
    juce::Rectangle<float> thumbnailBounds = getLocalBounds ().toFloat ();
    float visibleRegionLengthSeconds = getVisibleRegionLengthSeconds ();

    return ((s - visibleRegionStartTime) / visibleRegionLengthSeconds)
        * thumbnailBounds.getWidth ();
}

float AudioWaveformComponent::flattenSeconds (float s)
{
    if (s < 0.0f)
    {
        return 0.0f;
    }

    if (s > audioSource.getLengthInSeconds ())
    {
        return audioSource.getLengthInSeconds ();
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