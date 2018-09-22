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


class TenFtMainComponent :    public Component,
                              private Timer
{
public:
    TenFtMainComponent (TenFtAudioSource& audioSource);

    ~TenFtMainComponent ();

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

    void enableButtons (std::initializer_list<Button*> buttons, bool enable);

    void timerCallback () override;

private:
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
    TenFtAudioSource& audioSource;

    AudioWaveformComponent waveform;
    AudioWaveformSelectedRegionComponent selectedRegion;
    AudioPlaybackPositionComponent playbackPosition;
    AudioClockComponent clock;
    AudioScrollerComponent scroller;

    TenFtLookAndFeel tenFtLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TenFtMainComponent)
};
