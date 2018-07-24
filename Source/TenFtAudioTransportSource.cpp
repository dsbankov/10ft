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
        loadAudioSubsection (0.0, reader->lengthInSamples / reader->sampleRate, false);
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
                reader->lengthInSamples / reader->sampleRate, // TODO move in function
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

void TenFtAudioTransportSource::loadAudioSubsection (
    double startTime,
    double endTime,
    bool shouldLoop
)
{
    int64 startSample = (int64) (startTime * reader->sampleRate),
        numSamples = (int64) ((endTime - startTime) * reader->sampleRate);
    AudioFormatReader* subsectionReader =
        new AudioSubsectionReader (reader, startSample, numSamples, false);

    //Logger::outputDebugString ("loadAudioSubsection(" +
    //    String (startTime) + ", " + String (endTime) + ", " + String ((int)shouldLoop) +
    //    ")");
    
    swapReader (subsectionReader, true, shouldLoop);
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

    Logger::outputDebugString ("selectedRegionCreated - setPosition (0.0)");
    setPosition (0.0);

    if (state == Playing)
    {
        start ();
    }
    else
    {
        playAudio ();
    }
}

void TenFtAudioTransportSource::selectedRegionCleared (AudioWaveformComponent* waveform)
{
    if (hasSubsection && !waveform->getHasSelectedRegion ())
    {
        Logger::outputDebugString ("selectedRegionCleared");
        hasSubsection = false;
        subsectionStartTime = 0.0;
        subsectionEndTime = reader->lengthInSamples / reader->sampleRate;
        loadAudioSubsection (subsectionStartTime,
            subsectionEndTime, // TODO move in function
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
        Logger::outputDebugString ("changeListenerCallback - setPosition (0.0)");
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