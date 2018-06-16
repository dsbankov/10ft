/*
  ==============================================================================

    AudioFileTransportSource.cpp
    Created: 29 May 2018 11:17:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "TenFtAudioTransportSource.h"


TenFtAudioTransportSource::TenFtAudioTransportSource () :
    formatManager ()
{
    formatManager.registerBasicFormats ();
    selectedRegionStartTime = 0.0f;
    selectedRegionEndTime = getLengthInSeconds ();
    addChangeListener (this);
    startTimer (100);
}

TenFtAudioTransportSource::~TenFtAudioTransportSource ()
{
    stopTimer ();
    setSource (nullptr);
}

bool TenFtAudioTransportSource::loadAudio (File& file)
{
    AudioFormatReader* reader = formatManager.createReaderFor (file);

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

void TenFtAudioTransportSource::setupLooping (double startTime, double endTime)
{
    readerSource.get ()->setLooping (true);
    shouldLoop = true;
    selectedRegionStartTime = startTime;
    selectedRegionEndTime = endTime;
}

void TenFtAudioTransportSource::disableLooping ()
{
    readerSource.get ()->setLooping (false);
    shouldLoop = false;
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

void TenFtAudioTransportSource::timerCallback ()
{
    if (hasSelectedRegion && getCurrentPosition () >= selectedRegionEndTime)
    {
        setPosition (selectedRegionStartTime);
        if (!shouldLoop)
        {
            pauseAudio ();
        }
    }

    listeners.call ([this] (Listener& l) { l.currentPositionChanged (this); });
}

void TenFtAudioTransportSource::selectedRegionChanged (
    AudioWaveformComponent* waveform
)
{
    hasSelectedRegion = waveform->getHasSelectedRegion ();
    selectedRegionStartTime = waveform->getSelectedRegionStartTime ();
    selectedRegionEndTime = waveform->getSelectedRegionEndTime ();
}
