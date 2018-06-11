/*
  ==============================================================================

    AudioFileTransportSource.cpp
    Created: 29 May 2018 11:17:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "TenFtAudioTransportSource.h"


TenFtAudioTransportSource::TenFtAudioTransportSource (
    AudioFormatManager& formatManager,
    bool& hasSelectedRegion,
    float& selectedRegionStartTime,
    float& selectedRegionEndTime
) :
    AudioTransportSource (),
    formatManager (formatManager),
    state (NoFileLoaded),
    hasSelectedRegion (hasSelectedRegion),
    selectedRegionStartTime (selectedRegionStartTime),
    selectedRegionEndTime (selectedRegionEndTime)
{
    formatManager.registerBasicFormats ();
    addChangeListener (this);
}

TenFtAudioTransportSource::~TenFtAudioTransportSource ()
{
    setSource (nullptr);
}

void TenFtAudioTransportSource::getNextAudioBlock (
    const AudioSourceChannelInfo& info
)
{
    AudioTransportSource::getNextAudioBlock (info);

    double currentPosition = getCurrentPosition (),
        loopEndTime = hasSelectedRegion ?
            selectedRegionEndTime : getLengthInSeconds ();

    if (currentPosition >= loopEndTime)
    {
        double loopStartTime =
            hasSelectedRegion ? selectedRegionStartTime : 0.0f;

        setPosition (loopStartTime);

        if (!hasSelectedRegion)
        {
            start ();
        }
        if (!shouldLoop)
        {
            pauseAudio ();
        }
    }
}

bool TenFtAudioTransportSource::loadAudio (File& file)
{
    auto* reader = formatManager.createReaderFor (file);

    if (reader != nullptr)
    {
        std::unique_ptr<AudioFormatReaderSource> tempReaderSource (
            new AudioFormatReaderSource (reader, true)
        );

        setSource (
            tempReaderSource.get (),
            0,
            nullptr,
            reader->sampleRate
        );

        readerSource.swap (tempReaderSource);

        changeState (Stopped);
        setPosition (0.0);

        return true;
    }
    else
    {
        unloadAudio ();
        return false;
    }
}

bool TenFtAudioTransportSource::isAudioLoaded ()
{
    return state != NoFileLoaded;
}

void TenFtAudioTransportSource::playAudio ()
{
    if (state == Playing)
    {
        changeState (Pausing);
    }
    else
    {
        changeState (Starting);
    }
}

void TenFtAudioTransportSource::stopAudio ()
{
    if (state == Paused || state == NoFileLoaded)
    {
        changeState (Stopped);
    }
    else
    {
        changeState (Stopping);
    }
}

void TenFtAudioTransportSource::pauseAudio ()
{
    changeState (Pausing);
}

void TenFtAudioTransportSource::setLooping (bool shouldLoop)
{
    this->shouldLoop = shouldLoop;
}

bool TenFtAudioTransportSource::isLooping () const
{
    return shouldLoop;
}

// ==============================================================================

void TenFtAudioTransportSource::unloadAudio ()
{
    changeState (NoFileLoaded);
}

void TenFtAudioTransportSource::changeState (
    TenFtAudioTransportSource::State newState
)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
            case Starting:
                start ();
                break;

            case Playing:
                break;

            case Stopping:
                stop ();
                break;

            case Stopped:
                setPosition (0.0);
                break;

            case Pausing:
                stop ();
                break;

            case Paused:
                break;

            case NoFileLoaded:
                setSource (nullptr);
                break;
        }

        onStateChange (newState);
    }
}

void TenFtAudioTransportSource::changeListenerCallback (
    ChangeBroadcaster* broadcaster
)
{
    if (broadcaster == this)
    {
        if (isPlaying ())
        {
            changeState (Playing);
        }
        else if (state == Pausing)
        {
            changeState (Paused);
        }
        else
        {
            changeState (Stopped);
        }
    }
}
