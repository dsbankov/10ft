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
    openGLContext.setContinuousRepainting (true);
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*getTopLevelComponent ());

    setInterceptsMouseClicks (true, false);

    addAndMakeVisible (&waveformChannel);
}

AudioWaveformComponent::~AudioWaveformComponent ()
{
    openGLContext.detach ();
    openGLContext.setRenderer (nullptr);
}

void AudioWaveformComponent::newOpenGLContextCreated ()
{
    waveformChannel.initialise (openGLContext);
}

void AudioWaveformComponent::openGLContextClosing ()
{
    waveformChannel.shutdown (openGLContext);
}

void AudioWaveformComponent::renderOpenGL ()
{
    waveformChannel.render (openGLContext);
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
    waveformChannel.setBounds (getLocalBounds ());
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

bool AudioWaveformComponent::loadWaveform (AudioFormatReader* reader)
{
    this->reader = reader;
    if (reader != nullptr)
    {
        readerBuffer.setSize (reader->numChannels, reader->lengthInSamples);
        reader->read (&readerBuffer, 0, reader->lengthInSamples, 0, true, true);

        waveformChannel.loadBuffer (readerBuffer.getReadPointer(0));

        updateVisibleRegion (0.0f, getTotalLength());
        return true;
    }
    else
    {
        readerBuffer.clear ();
        clearWaveform ();
        return false;
    }
}

void AudioWaveformComponent::clearWaveform ()
{
    clearSelectedRegion ();
    updateVisibleRegion (0.0f, 0.0f);
    listeners.call ([this] (Listener& l) { l.thumbnailCleared (this); });
}

double AudioWaveformComponent::getTotalLength ()
{
    return reader != nullptr ? reader->lengthInSamples / reader->sampleRate : 0.0;
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

    //const float* channelBuffer = readerBuffer.getReadPointer (0);
    int64 startSample = visibleRegionStartTime * reader->sampleRate,
        endSample = visibleRegionEndTime * reader->sampleRate,
        numSamples = endSample - startSample + 1;

    waveformChannel.redraw (startSample, numSamples); // TODO use the listener instead?

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
    g.setColour (findColour(ColourIds::waveformBackgroundColour));
    g.fillRect (thumbnailBounds);
    g.setColour (findColour (ColourIds::waveformColour));
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