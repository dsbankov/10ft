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

#include "AudioBufferSource.h"
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

    void loadAudio (
        AudioSampleBuffer* newAudioSampleBuffer,
        double newSampleRate
    );

    void unloadAudio ();

    bool isAudioLoaded ();

    void playAudio ();

    void stopAudio ();

    void pauseAudio ();

    void muteAudio ();

    void fadeInAudio ();

    void fadeOutAudio ();

    void normalizeAudio ();

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

    void changeState (State newState);

    void loadAudioSubsection (
        double startTime,
        double endTime,
        bool subsectionSelected,
        bool shouldLoop
    );

private:
    State state = NoFileLoaded;

    double sampleRate = 0.0;

    AudioSampleBuffer* audioBuffer = nullptr;
    std::unique_ptr<AudioSampleBuffer> subsectionAudioBuffer;
    std::unique_ptr<AudioBufferSource> audioSource;

    bool hasSubsection = false;
    double subsectionStartTime;
    double subsectionEndTime;

    ListenerList<Listener> listeners;

};
