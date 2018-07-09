/*
  ==============================================================================

    AudioWaveformComponent.cpp
    Created: 29 May 2018 11:13:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "AudioWaveformComponent.h"


AudioWaveformComponent::AudioWaveformComponent ()
{
    openGLContext.setComponentPaintingEnabled (true);
    openGLContext.setContinuousRepainting (true);
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*this);

    addAndMakeVisible (&waveform);

    setInterceptsMouseClicks (true, false);
}

AudioWaveformComponent::~AudioWaveformComponent ()
{
    openGLContext.detach ();
}

void AudioWaveformComponent::newOpenGLContextCreated ()
{
    waveform.initialise (openGLContext);
}

void AudioWaveformComponent::openGLContextClosing ()
{
    waveform.shutdown (openGLContext);
}

void AudioWaveformComponent::renderOpenGL ()
{
    waveform.render (openGLContext);
}

void AudioWaveformComponent::paint (Graphics& g)
{
    if (getTotalLength () <= 0)
    {
        paintIfNoFileLoaded (g);
    }
}

void AudioWaveformComponent::resized ()
{
    waveform.setBounds (getLocalBounds ());
}

void AudioWaveformComponent::mouseWheelMove (
    const MouseEvent& event,
    const MouseWheelDetails& wheelDetails
)
{
    if (getTotalLength () <= 0)
    {
        return;
    }

    juce::Rectangle<float> bounds = getLocalBounds ().toFloat ();
    float leftRelativeAmmount =
        (float) (event.getMouseDownX () - bounds.getX ())
        / bounds.getWidth (),
        rightRelativeAmmount = 1.0f - leftRelativeAmmount;
    double visibleRegionLengthInSeconds = getVisibleRegionLengthInSeconds ();

    const double scrollAmmount = 0.1f * visibleRegionLengthInSeconds,
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
    if (getTotalLength () <= 0)
    {
        return;
    }

    double newPosition = util::xToSeconds (
        (float) event.getMouseDownX (),
        visibleRegionStartTime,
        visibleRegionEndTime,
        getLocalBounds ().toFloat ()
    );

    clearSelectedRegion ();

    onPositionChange (newPosition);
}

void AudioWaveformComponent::mouseDrag (const MouseEvent &event)
{
    if (getTotalLength () <= 0)
    {
        return;
    }

    juce::Rectangle<float> bounds = getLocalBounds ().toFloat ();

    if (hasSelectedRegion)
    {
        int mouseDownX = event.getMouseDownX () +
            event.getDistanceFromDragStartX ();
        double mouseDownSeconds = util::xToSeconds (
            (float) mouseDownX,
            visibleRegionStartTime,
            visibleRegionEndTime,
            bounds
        );

        updateSelectedRegion (mouseDownSeconds);
    }
    else
    {
        float startOfDragX = util::secondsToX (
            selectedRegionStartTime,
            visibleRegionStartTime,
            visibleRegionEndTime,
            bounds
        ),
            endOfDragX = startOfDragX + event.getDistanceFromDragStartX ();
        double newStartTime = selectedRegionStartTime,
            newEndTime = util::xToSeconds (
                endOfDragX,
                visibleRegionStartTime,
                visibleRegionEndTime,
                bounds
            );

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
    if (getTotalLength () <= 0)
    {
        return;
    }

    int mouseDownX = event.getMouseDownX ();
    juce::Rectangle<float> bounds = getLocalBounds ().toFloat ();

    if (hasSelectedRegion)
    {
        double seconds = util::xToSeconds (
            (float) mouseDownX,
            visibleRegionStartTime,
            visibleRegionEndTime,
            bounds
        );

        updateSelectedRegion (seconds);

        repaint ();
    }
    else
    {
        setSelectedRegionStartTime (
            util::xToSeconds (
            (float) mouseDownX,
                visibleRegionStartTime,
                visibleRegionEndTime,
                bounds
            )
        );
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

void AudioWaveformComponent::addListener (Listener * newListener)
{
    listeners.add (newListener);
}

void AudioWaveformComponent::removeListener (Listener * listener)
{
    listeners.remove (listener);
}

bool AudioWaveformComponent::load (AudioFormatReader* reader)
{
    audioReader = reader;
    if (audioReader != nullptr)
    {
        waveform.load (audioReader);
        updateVisibleRegion (0.0f, getTotalLength());
        return true;
    }
    else
    {
        clear ();
        return false;
    }
}

void AudioWaveformComponent::clear ()
{
    audioReader = nullptr;
    clearSelectedRegion ();
    updateVisibleRegion (0.0f, 0.0f);
    listeners.call ([this] (Listener& l) { l.thumbnailCleared (this); });
}

double AudioWaveformComponent::getTotalLength ()
{
    return audioReader != nullptr ? audioReader->lengthInSamples / audioReader->sampleRate : 0.0;
}

void AudioWaveformComponent::updateVisibleRegion (
    double newStartTime,
    double newEndTime
)
{
    double totalLength = getTotalLength (),
        startTimeFlattened = util::flattenSeconds (
            newStartTime, totalLength
        ),
        endTimeFlattened = util::flattenSeconds (
            newEndTime, totalLength
        );

    jassert (isVisibleRegionCorrect (startTimeFlattened, endTimeFlattened));

    if (endTimeFlattened - startTimeFlattened < 0.05f)
    {
        return;
    }

    visibleRegionStartTime = startTimeFlattened;
    visibleRegionEndTime = endTimeFlattened;

    int64 startSample = visibleRegionStartTime * audioReader->sampleRate,
        endSample = visibleRegionEndTime * audioReader->sampleRate,
        numSamples = endSample - startSample;

    // TODO use listener instead?
    waveform.display (startSample, numSamples);

    listeners.call ([this] (Listener& l) { l.visibleRegionChanged (this); });

    repaint ();
}

void AudioWaveformComponent::updateSelectedRegion (
    double newStartTime, double newEndTime
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

double AudioWaveformComponent::getVisibleRegionStartTime ()
{
    return visibleRegionStartTime;
}

double AudioWaveformComponent::getVisibleRegionEndTime ()
{
    return visibleRegionEndTime;
}

double AudioWaveformComponent::getSelectedRegionStartTime ()
{
    return selectedRegionStartTime;
}

double AudioWaveformComponent::getSelectedRegionEndTime ()
{
    return selectedRegionEndTime;
}

bool AudioWaveformComponent::getHasSelectedRegion ()
{
    return hasSelectedRegion;
}

// ==============================================================================

void AudioWaveformComponent::paintIfNoFileLoaded (Graphics& g)
{
    juce::Rectangle<float> thumbnailBounds = getLocalBounds ().toFloat ();
    g.setColour (findColour(
        AudioWaveformOpenGLComponent::ColourIds::waveformBackgroundColour)
    );
    g.fillRect (thumbnailBounds);
    g.setColour (findColour (
        AudioWaveformOpenGLComponent::ColourIds::waveformColour)
    );
    g.drawFittedText (
        "No File Loaded",
        thumbnailBounds.toNearestInt (),
        Justification::centred,
        1
    );
}

void AudioWaveformComponent::setSelectedRegionStartTime (double newStartTime)
{
    updateSelectedRegion (newStartTime, selectedRegionEndTime);
}

void AudioWaveformComponent::setSelectedRegionEndTime (double newEndTime)
{
    updateSelectedRegion (selectedRegionStartTime, newEndTime);
}

double AudioWaveformComponent::getVisibleRegionLengthInSeconds ()
{
    return visibleRegionEndTime - visibleRegionStartTime;
}

void AudioWaveformComponent::updateSelectedRegion (double mouseDownSeconds)
{
    double distanceFromStartOfDragSeconds =
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
    double startTime, double endTime
)
{
    bool isAudioLoaded = getTotalLength () > 0.0;
    return
        (!isAudioLoaded &&
            startTime == 0.0f && endTime == 0.0f) ||
        (isAudioLoaded &&
            startTime < endTime &&
            startTime >= 0 &&
            endTime <= getTotalLength ());
}