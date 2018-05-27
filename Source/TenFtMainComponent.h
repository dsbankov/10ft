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

//==============================================================================
/*
*/
class TenFtMainComponent   :	public AudioAppComponent
{
public:
	TenFtMainComponent() : waveform(), playbackPosition(waveform), clock(waveform), scroller(waveform)
    {
        addAndMakeVisible (&openButton);
        openButton.setButtonText ("Open...");
        openButton.onClick = [this] { openButtonClicked(); };

        addAndMakeVisible (&playButton);
        playButton.setButtonText ("Play");
        playButton.onClick = [this] { playButtonClicked(); };
        playButton.setColour (TextButton::buttonColourId, Colours::green);
        playButton.setEnabled (false);

        addAndMakeVisible (&stopButton);
        stopButton.setButtonText ("Stop");
        stopButton.onClick = [this] { stopButtonClicked(); };
        stopButton.setColour (TextButton::buttonColourId, Colours::red);
        stopButton.setEnabled (false);

		addAndMakeVisible(&loopButton);
		loopButton.setButtonText("Loop");
		loopButton.setToggleState(false, NotificationType::dontSendNotification);
		loopButton.onClick = [this] { loopButtonClicked(); };
		loopButton.setEnabled(false);

		addAndMakeVisible(&waveform);		// order is important for mouseDown events! (will go to the most upfront component (last added))
		addAndMakeVisible(&playbackPosition);	// order is important for mouseDown events! (will go to the most upfront component (last added))
		addAndMakeVisible(&scroller);
		addAndMakeVisible(&clock);

		//openGLContext.attachTo(*this);
		//openGLContext.attachTo(waveform);
		//openGLContext.attachTo(progressLine);

        setSize (1000, 800);
        setAudioChannels (0, 2);

		waveform.getAudioSource().onStateChange = [this] (AudioFileTransportSource::AudioPlayerState state) { onAudioPlayerStateChange(state); };
    }

    ~TenFtMainComponent()
    {
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
		waveform.getAudioSource().prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (!waveform.getAudioSource().isFileLoaded())
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
		waveform.getAudioSource().getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
		waveform.getAudioSource().releaseResources();
    }

    void resized() override
    {
		auto bounds = getLocalBounds();
		auto width = bounds.getWidth();
		auto height = bounds.getHeight();
		auto row1 = bounds.removeFromTop(0.05 * height);
		auto row2 = bounds.removeFromTop(0.05 * height);
		auto row4 = bounds.removeFromBottom(0.05 * height);
		auto row3 = bounds;
		int delta = 5;
		openButton.setBounds(row1.reduced(delta));
		playButton.setBounds(row2.removeFromLeft(width * 0.42).reduced(delta));
		stopButton.setBounds(row2.removeFromLeft(width * 0.42).reduced(delta));
		loopButton.setBounds(row2.removeFromLeft(width * 0.07).reduced(delta));
		clock.setBounds(row2.reduced(delta));
		waveform.setBounds(row3.reduced(delta));
		playbackPosition.setBounds(row3.reduced(delta));
		scroller.setBounds(row4.reduced(delta));
    }

private:

    void openButtonClicked()
    {
        FileChooser chooser ("Select a Wave file to play...", File::getSpecialLocation(File::userMusicDirectory), "*.wav");
		bool fileSelected = chooser.browseForFileToOpen();
        if (fileSelected)
        {
            auto file = chooser.getResult();
			if (waveform.loadAudio(file))
			{
				setupButton(playButton, "Play", true);
				setupButton(stopButton, "Stop", false);
				loopButton.setEnabled(true);
				clock.startTimer(100);
			}
			else
			{
				setupButton(playButton, "Play", false);
				setupButton(stopButton, "Stop", false);
				loopButton.setEnabled(false);
				loopButton.setToggleState(false, NotificationType::dontSendNotification);
				clock.stopTimer();
			}
        }
    }

    void playButtonClicked()
    {
		waveform.getAudioSource().playAudio();
    }

    void stopButtonClicked()
    {
		waveform.getAudioSource().stopAudio();
    }

	void loopButtonClicked()
	{
		auto shouldLoop = loopButton.getToggleState();
		playbackPosition.setIsLooping(shouldLoop);
		waveform.getAudioSource().setLooping(shouldLoop);
	}

	void onAudioPlayerStateChange(AudioFileTransportSource::AudioPlayerState state)
	{
		if (state == AudioFileTransportSource::Stopped)
		{
			setupButton(playButton, "Play", true);
			setupButton(stopButton, "Stop", false);
			playbackPosition.stopTimer();
			clock.stopTimer();
			waveform.clearSelectedRegion();
		}
		else if (state == AudioFileTransportSource::Playing)
		{
			setupButton(playButton, "Pause", true);
			setupButton(stopButton, "Stop", true);
			playbackPosition.startTimer(100);
			clock.startTimer(100);
		}
		else if (state == AudioFileTransportSource::Paused)
		{
			setupButton(playButton, "Play", true);
			setupButton(stopButton, "Return To Zero", true);
			playbackPosition.stopTimer();
			clock.stopTimer();
		}
	}

	void setupButton(TextButton& button, std::string buttonText, bool enabled)
	{
		button.setButtonText(buttonText);
		button.setEnabled(enabled);
	}

    //==============================================================================================

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
