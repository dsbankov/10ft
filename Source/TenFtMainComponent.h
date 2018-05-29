/*
==============================================================================

TenFtMainComponent.h
Created: 20 May 2018 3:33:07pm
Author:  DBANKOV

==============================================================================
*/


#pragma once


#include "AudioFileTransportSource.h"
#include "AudioWaveformComponent.h"
#include "AudioPlaybackPositionComponent.h"
#include "AudioClockComponent.h"
#include "AudioScrollerComponent.h"


class TenFtMainComponent :    public AudioAppComponent
{
public:
    TenFtMainComponent ();

    ~TenFtMainComponent ();

    void prepareToPlay (
        int samplesPerBlockExpected,
        double sampleRate
    ) override;

    void getNextAudioBlock (
        const AudioSourceChannelInfo& bufferToFill
    ) override;

    void releaseResources () override;

    void resized () override;

private:
    void openButtonClicked ();

    void playButtonClicked ();

    void stopButtonClicked ();

    void loopButtonClicked ();

    void onAudioPlayerStateChange (
        AudioFileTransportSource::AudioPlayerState state
    );

    void setupButton (
        TextButton& button,
        std::string buttonText,
        bool enabled
    );

private:
    TextButton openButton;
    TextButton playButton;
    TextButton stopButton;
    ToggleButton loopButton;
    AudioWaveformComponent waveform;
    AudioClockComponent clock;
    AudioPlaybackPositionComponent playbackPosition;
    AudioScrollerComponent scroller;
    //OpenGLContext openGLContext;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TenFtMainComponent)
};
