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

#include "AudioWaveformComponent.h"


class TenFtAudioTransportSource :    public AudioTransportSource,
                                     public AudioWaveformComponent::Listener,
                                     private ChangeListener,
                                     private Timer
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

    class Listener
    {
    public:
        virtual ~Listener () {}

        virtual void currentPositionChanged (TenFtAudioTransportSource*) {}

        virtual void stateChanged (TenFtAudioTransportSource*) {}
    };

    std::function<void (
        TenFtAudioTransportSource::State
    )> onStateChange;

public:
    TenFtAudioTransportSource ();

    ~TenFtAudioTransportSource ();

    bool loadAudio (AudioFormatReader* reader);

    bool isAudioLoaded ();

    void playAudio ();

    void stopAudio ();

    void pauseAudio ();

    void setupLooping (double startTime, double endTime);

    void disableLooping ();

    void addListener (Listener* newListener);

    void removeListener (Listener* listener);

    void selectedRegionChanged (AudioWaveformComponent* waveform) override;

private:
    void unloadAudio ();

    void changeState (
        TenFtAudioTransportSource::State newState
    );

    void changeListenerCallback (
        ChangeBroadcaster* broadcaster
    ) override;

    void timerCallback () override;

private:
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    State state = NoFileLoaded;
    bool shouldLoop = false;
    bool hasSelectedRegion = false;
    double selectedRegionStartTime;
    double selectedRegionEndTime;
    ListenerList<Listener> listeners;

};
