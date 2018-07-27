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

    std::function<void (State)> onStateChange;

public:
    TenFtAudioTransportSource ();

    ~TenFtAudioTransportSource ();

    bool loadAudio (AudioFormatReader* newReader);

    bool isAudioLoaded ();

    void playAudio ();

    void stopAudio ();

    void pauseAudio ();

    double getCurrentPositionGlobal () const;

    double getLengthInSecondsGlobal () const;

    void setLooping (bool shouldLoop) override;

    void selectedRegionCreated (AudioWaveformComponent* waveform) override;

    void selectedRegionCleared (AudioWaveformComponent* waveform) override;

    void addListener (Listener* newListener);

    void removeListener (Listener* listener);

private:
    void changeListenerCallback (ChangeBroadcaster* broadcaster) override;

    void timerCallback () override;

    void unloadAudio ();

    void changeState (State newState);

    void loadAudioSubsection (double startTime,
        double endTime,
        bool shouldLoop
    );

    void swapReader (AudioFormatReader* newReader,
        bool deleteReaderWhenThisIsDeleted,
        bool shouldLoop
    );

private:
    AudioFormatReader* reader;
    std::unique_ptr<AudioFormatReaderSource> readerSource;
    State state = NoFileLoaded;
    bool hasSubsection = false;
    double subsectionStartTime;
    double subsectionEndTime;
    ListenerList<Listener> listeners;

};
