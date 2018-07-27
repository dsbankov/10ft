/*
  ==============================================================================

    AudioFileTransportSource.cpp
    Created: 29 May 2018 11:17:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "TenFtAudioTransportSource.h"


TenFtAudioTransportSource::TenFtAudioTransportSource ()
{
    addChangeListener (this);
    startTimer (100);
}

TenFtAudioTransportSource::~TenFtAudioTransportSource ()
{
    stopTimer ();
    setSource (nullptr);
}

bool TenFtAudioTransportSource::loadAudio (AudioFormatReader* newReader)
{
    if (newReader != nullptr)
    {
        reader = newReader;
        loadAudioSubsection (0.0, getLengthInSecondsGlobal (), false);
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
    if (hasSubsection && state == Paused)
    {
        if (getCurrentPosition () == 0.0)
        {
            loadAudioSubsection (0.0,
                getLengthInSecondsGlobal(),
                readerSource->isLooping ());
            changeState (Stopped);
            stop ();
        }
        else
        {
            setPosition (0.0);
        }
    }
    else if (state == Paused || state == NoFileLoaded)
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

double TenFtAudioTransportSource::getCurrentPositionGlobal () const
{
    double currentPosition = getCurrentPosition ();
    if (hasSubsection)
    {
        currentPosition += subsectionStartTime;
    }
    return currentPosition;
}

double TenFtAudioTransportSource::getLengthInSecondsGlobal () const
{
    if (reader != nullptr)
    {
        return reader->lengthInSamples / reader->sampleRate;
    }
    else
    {
        return 0.0;
    }
}

void TenFtAudioTransportSource::setLooping (bool shouldLoop)
{
    readerSource->setLooping (shouldLoop);
}

void TenFtAudioTransportSource::selectedRegionCreated (
    AudioWaveformComponent* waveform
)
{
    hasSubsection = true;
    subsectionStartTime = waveform->getSelectedRegionStartTime ();
    subsectionEndTime = waveform->getSelectedRegionEndTime ();
    loadAudioSubsection (subsectionStartTime,
        subsectionEndTime, readerSource->isLooping ());

    if (state == Playing)
    {
        start ();
    }
}

void TenFtAudioTransportSource::selectedRegionCleared (AudioWaveformComponent* waveform)
{
    if (hasSubsection && !waveform->getHasSelectedRegion ())
    {
        hasSubsection = false;
        subsectionStartTime = 0.0;
        subsectionEndTime = getLengthInSecondsGlobal ();
        loadAudioSubsection (subsectionStartTime,
            subsectionEndTime,
            readerSource->isLooping ());
        start ();
    }
}

void TenFtAudioTransportSource::addListener (Listener * newListener)
{
    listeners.add (newListener);
}

void TenFtAudioTransportSource::removeListener (Listener * listener)
{
    listeners.remove (listener);
}

// ==============================================================================

void TenFtAudioTransportSource::changeListenerCallback (
    ChangeBroadcaster*
)
{
    if (isPlaying ())
    {
        changeState (Playing);
    }
    else if (state == Pausing)
    {
        changeState (Paused);
    }
    else if (hasSubsection)
    {
        setPosition (0.0);
        changeState (Paused);
    }
    else
    {
        changeState (Stopped);
    }
}

void TenFtAudioTransportSource::timerCallback ()
{
    listeners.call ([this](Listener& l) { l.currentPositionChanged (this); });
}

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

        listeners.call ([this] (Listener& l) { l.stateChanged (this); });
    }
}

void TenFtAudioTransportSource::loadAudioSubsection (
    double startTime,
    double endTime,
    bool shouldLoop
)
{
    int64 startSample = (int64)(startTime * reader->sampleRate),
        numSamples = (int64)((endTime - startTime) * reader->sampleRate);
    AudioFormatReader* subsectionReader =
        new AudioSubsectionReader (reader, startSample, numSamples, false);

    swapReader (subsectionReader, true, shouldLoop);
}

void TenFtAudioTransportSource::swapReader (AudioFormatReader* newReader,
    bool deleteReaderWhenThisIsDeleted,
    bool shouldLoop)
{
    std::unique_ptr<AudioFormatReaderSource> tempReaderSource (
        new AudioFormatReaderSource (newReader, deleteReaderWhenThisIsDeleted)
    );

    tempReaderSource->setLooping (shouldLoop);

    setSource (
        tempReaderSource.get (),
        0,
        nullptr,
        newReader->sampleRate
    );

    readerSource.swap (tempReaderSource);
}