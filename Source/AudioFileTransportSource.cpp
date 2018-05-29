/*
  ==============================================================================

    AudioFileTransportSource.cpp
    Created: 29 May 2018 11:17:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "AudioFileTransportSource.h"


AudioFileTransportSource::AudioFileTransportSource (
    AudioFormatManager& formatManager
)
    :
        AudioTransportSource (),
        formatManager (formatManager),
        state (NoFileLoaded)
{
    formatManager.registerBasicFormats ();
    addChangeListener (this);
}

AudioFileTransportSource::~AudioFileTransportSource ()
{
    setSource (nullptr);
}

bool AudioFileTransportSource::loadAudio (File& file)
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

bool AudioFileTransportSource::isFileLoaded ()
{
    return state != NoFileLoaded;
}

void AudioFileTransportSource::playAudio ()
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

void AudioFileTransportSource::stopAudio ()
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

void AudioFileTransportSource::pauseAudio ()
{
    changeState (Pausing);
}

void AudioFileTransportSource::setLooping (bool shouldLoop)
{
    readerSource.get ()->setLooping (shouldLoop);
}

// ==============================================================================

void AudioFileTransportSource::unloadAudio ()
{
    changeState (NoFileLoaded);
}

void AudioFileTransportSource::changeState (
    AudioFileTransportSource::AudioPlayerState newState
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

void AudioFileTransportSource::changeListenerCallback (
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
