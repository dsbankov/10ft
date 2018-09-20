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


class TenFtAudioSource :    public AudioSource,
                            public AudioWaveformComponent::Listener,
                            private ChangeListener,
                            private Timer
{
public:
    enum State
    {
        NoAudioLoaded,
        StartRecording,
        Recording,
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

        virtual void currentPositionChanged (TenFtAudioSource*) {}

        virtual void stateChanged (TenFtAudioSource*) {}
    };

    std::function<void (State)> onStateChange;

public:
    TenFtAudioSource ();

    ~TenFtAudioSource ();

    void prepareToPlay (
        int samplesPerBlockExpected,
        double sampleRate
    ) override;
    
    void releaseResources () override;

    void getNextAudioBlock (
        const AudioSourceChannelInfo& bufferToFill
    ) override;

    void loadAudio (
        AudioSampleBuffer* newAudioSampleBuffer,
        double newSampleRate
    );

    void unloadAudio ();

    AudioSampleBuffer* loadRecordingBuffer ();

    void stopRecording ();

    void playAudio ();

    void stopAudio ();

    void pauseAudio ();

    void muteAudio ();

    void fadeInAudio ();

    void fadeOutAudio ();

    void normalizeAudio ();

    State getState () const;

    double getCurrentPosition () const;

    double getLengthInSeconds () const;

    double getSampleRate () const;

    void setPosition (double newPosition);

    void setLooping (bool shouldLoop);

    void addListener (Listener* newListener);

    void removeListener (Listener* listener);

    const CriticalSection* getBufferUpdateLock () const noexcept;

private:
    void selectedRegionCreated (AudioWaveformComponent* waveform) override;

    void selectedRegionCleared (AudioWaveformComponent* waveform) override;

    void timerCallback () override;

    void changeListenerCallback (ChangeBroadcaster* broadcaster) override;

    void changeState (State newState);

    void loadAudioSubregion (
        double startTime,
        double endTime,
        bool subregionSelected,
        bool shouldLoop
    );

private:
    class BufferPreallocationThread;

private:
    AudioTransportSource masterSource;
    std::unique_ptr<AudioBufferSource> bufferSource;

    AudioSampleBuffer* buffer = nullptr;
    std::unique_ptr<AudioSampleBuffer> subregionBuffer;

    AudioSampleBuffer preallocatedRecordingBuffer;
    std::unique_ptr<BufferPreallocationThread> recordingBufferPreallocationThread;
    int numSamplesRecorded = 0;

    double sampleRate = 0.0;
    double inputSampleRate = 0.0;

    bool hasSubregion = false;
    double subregionStartTime = 0.0;
    double subregionEndTime = 0.0;

    State state = NoAudioLoaded;

    ListenerList<Listener> listeners;

private:
    class BufferPreallocationThread : public Thread
    {
    public:
        BufferPreallocationThread (
            AudioSampleBuffer& preallocatedRecordingBuffer,
            int& numSamplesRecorded,
            int numSamplesBuffer,
            int numSamplesToAllocate,
            AudioSampleBuffer& buffer
        );

        void run () override;

        const CriticalSection& getLock () const noexcept;

    private:
        AudioSampleBuffer& preallocatedRecordingBuffer;
        int& numSamplesRecorded;
        int numSamplesBuffer;
        int numSamplesToAllocate;
        AudioSampleBuffer& buffer;
        const CriticalSection bufferUpdateLock;
    };

};
