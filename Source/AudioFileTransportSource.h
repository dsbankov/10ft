/*
  ==============================================================================

    AudioPlayer.h
    Created: 18 May 2018 9:53:34pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"
#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>


class AudioFileTransportSource :    public AudioTransportSource,
                                    private ChangeListener
{
public:
    enum AudioPlayerState
    {
        NoFileLoaded,
        Starting,
        Playing,
        Stopping,
        Stopped,
        Pausing,
        Paused
    };
    std::function<void (
        AudioFileTransportSource::AudioPlayerState
    )> onStateChange;

public:
    AudioFileTransportSource (
        AudioFormatManager& formatManager
    );

    ~AudioFileTransportSource ();

    bool loadAudio (File& file);

    bool isFileLoaded ();

    void playAudio ();

    void stopAudio ();

    void pauseAudio ();

    void setLooping (bool shouldLoop) override;

private:
    void unloadAudio ();

    void changeState (
        AudioFileTransportSource::AudioPlayerState newState
    );

    void changeListenerCallback (
        ChangeBroadcaster* broadcaster
    ) override;

private:
    AudioFormatManager& formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    AudioPlayerState state;
};
