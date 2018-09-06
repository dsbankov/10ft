/*
==============================================================================

TenFtMainComponent.h
Created: 20 May 2018 3:33:07pm
Author:  DBANKOV

==============================================================================
*/


#pragma once

#include "TenFtAudioSource.h"
#include "AudioBufferSource.h"
#include "AudioWaveformComponent.h"
#include "AudioWaveformSelectedRegionComponent.h"
#include "AudioPlaybackPositionComponent.h"
#include "AudioClockComponent.h"
#include "AudioScrollerComponent.h"
#include "TenFtLookAndFeel.h"


class TenFtMainComponent :    public AudioAppComponent,
                              private Timer
{
public:
    TenFtMainComponent ();

    ~TenFtMainComponent ();

    void prepareToPlay (
        int samplesPerBlockExpected,
        double currentSampleRate
    ) override;

    void getNextAudioBlock (
        const AudioSourceChannelInfo& bufferToFill
    ) override;

    void releaseResources () override;

    void resized () override;

    void paint (Graphics& g) override;

private:
    void openButtonClicked ();

    void loadFile (AudioFormatReader* audioReader);

    void unloadFile ();

    void recordButtonClicked ();

    void enableRecording ();

    void disableRecording ();

    void loopButtonClicked ();

    void onAudioSourceStateChange (
        TenFtAudioSource::State state
    );

    void setupButton (
        TextButton& button,
        std::string buttonText,
        bool enabled
    );

    void timerCallback () override;

private:
    class BufferPreallocationThread;

    TextButton openButton;
    TextButton recordButton;
    TextButton playButton;
    TextButton stopButton;
    ToggleButton loopButton;
    TextButton muteButton;
    TextButton fadeInButton;
    TextButton fadeOutButton;
    TextButton normalizeButton;

    AudioFormatManager formatManager;
    std::unique_ptr<AudioSampleBuffer> audioBuffer;

    TenFtAudioSource audioSource;
    double sampleRate = 0.0;

    AudioWaveformComponent waveform;
    AudioWaveformSelectedRegionComponent selectedRegion;
    AudioPlaybackPositionComponent playbackPosition;
    AudioClockComponent clock;
    AudioScrollerComponent scroller;

    TenFtLookAndFeel tenFtLookAndFeel;

    static const int MAX_INPUT_CHANNELS_ALLOWED = 1;
    static const int INTERVAL_RECORD_REPAINT_MILLIS = 100;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TenFtMainComponent)
};
