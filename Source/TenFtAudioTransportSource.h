/*
  ==============================================================================

    AudioPlayer.h
    Created: 18 May 2018 9:53:34pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once

#include <string>
#include <iostream>
#include <sstream>
#include <iomanip>

#include "../JuceLibraryCode/JuceHeader.h"


class TenFtAudioTransportSource :   public AudioTransportSource,
                                    private ChangeListener
{
public:
    enum State
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
        TenFtAudioTransportSource::State
    )> onStateChange;

public:
    TenFtAudioTransportSource (
        AudioFormatManager& formatManager,
        bool& hasSelectedRegion,
        float& selectedRegionStartTime,
        float& selectedRegionEndTime
    );

    ~TenFtAudioTransportSource ();

    void getNextAudioBlock (const AudioSourceChannelInfo& info) override;

    bool loadAudio (File& file);

    bool isAudioLoaded ();

    void playAudio ();

    void stopAudio ();

    void pauseAudio ();

    void setLooping (bool shouldLoop) override;

    bool isLooping () const override;

private:
    void unloadAudio ();

    void changeState (
        TenFtAudioTransportSource::State newState
    );

    void changeListenerCallback (
        ChangeBroadcaster* broadcaster
    ) override;

private:
    AudioFormatManager& formatManager;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    State state;
    bool shouldLoop = false;
    bool& hasSelectedRegion;
    float& selectedRegionStartTime;
    float& selectedRegionEndTime;

};
