/*
==============================================================================

TenFtMainComponent.h
Created: 20 May 2018 3:33:07pm
Author:  DBANKOV

==============================================================================
*/


#pragma once


#include "TenFtAudioTransportSource.h"
#include "AudioWaveformComponent.h"
#include "AudioWaveformSelectedRegionComponent.h"
#include "AudioPlaybackPositionComponent.h"
#include "AudioClockComponent.h"
#include "AudioScrollerComponent.h"
#include "TenFtLookAndFeel.h"


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

    void paint (Graphics& g) override;

private:
    void openButtonClicked ();

    void loopButtonClicked ();

    void onAudioSourceStateChange (
        TenFtAudioTransportSource::State state
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

    AudioFormatManager formatManager;
    std::unique_ptr<AudioFormatReader> audioReader;

    TenFtAudioTransportSource audioSource;

    AudioWaveformComponent waveform;
    AudioWaveformSelectedRegionComponent selectedRegion;
    AudioPlaybackPositionComponent playbackPosition;
    AudioClockComponent clock;
    AudioScrollerComponent scroller;

    TenFtLookAndFeel tenFtLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TenFtMainComponent)
};
