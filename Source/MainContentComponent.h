/*
==============================================================================

MainContentComponent.h
Created: 20 May 2018 3:33:07pm
Author:  DBANKOV

==============================================================================
*/


#pragma once
#include "AudioFileTransportSource.h"
#include "AudioWaveformComponent.h"
#include "TrackProgressLineComponent.h"
#include "TrackProgressLabelComponent.h"
#include "TrackProgressSliderComponent.h"
#include "TrackScrollerComponent.h"

//==============================================================================
/*
*/
class MainContentComponent   :	public AudioAppComponent
{
public:
	MainContentComponent() :
		formatManager(), audioSource(formatManager),
		waveform(formatManager, audioSource), progressLine(waveform), progressLabel(audioSource), scroller(waveform)
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
		addAndMakeVisible(&progressLine);	// order is important for mouseDown events! (will go to the most upfront component (last added))
		addAndMakeVisible(&scroller);
		addAndMakeVisible(&progressLabel);

        setSize (1000, 800);
        setAudioChannels (0, 2);

		audioSource.onStateChange = [this] (AudioFileTransportSource::AudioPlayerState state) { onAudioPlayerStateChange(state); };
    }

    ~MainContentComponent()
    {
        shutdownAudio();
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
		audioSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill) override
    {
        if (!audioSource.isFileLoaded())
        {
            bufferToFill.clearActiveBufferRegion();
            return;
        }
		audioSource.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
		audioSource.releaseResources();
    }

    void resized() override
    {
		auto bounds = getLocalBounds();
		auto row1 = bounds.removeFromTop(40);
		auto row2 = bounds.removeFromTop(40);
		auto row3 = bounds.removeFromTop(600);
		auto row4 = bounds.removeFromTop(40);
		auto width = bounds.getWidth();
		int delta = 5;
		openButton.setBounds(row1.reduced(delta));
		playButton.setBounds(row2.removeFromLeft(width * 0.42).reduced(delta));
		stopButton.setBounds(row2.removeFromLeft(width * 0.42).reduced(delta));
		loopButton.setBounds(row2.removeFromLeft(width * 0.07).reduced(delta));
		progressLabel.setBounds(row2.reduced(delta));
		waveform.setBounds(row3.reduced(delta));
		progressLine.setBounds(row3.reduced(delta));
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
			if (audioSource.loadAudio(file))
			{
				setupButton(playButton, "Play", true);
				setupButton(stopButton, "Stop", false);
				loopButton.setEnabled(true);
				waveform.setSource(new FileInputSource(file));
			}
			else
			{
				setupButton(playButton, "Play", false);
				setupButton(stopButton, "Stop", false);
				loopButton.setEnabled(false);
				waveform.clear();
			}
        }
    }

    void playButtonClicked()
    {
		audioSource.playAudio();
    }

    void stopButtonClicked()
    {
		audioSource.stopAudio();
    }

	void loopButtonClicked()
	{
		auto shouldLoop = loopButton.getToggleState();
		progressLine.setIsLooping(shouldLoop);
	}

	void onAudioPlayerStateChange(AudioFileTransportSource::AudioPlayerState state)
	{
		switch (state)
		{
		case AudioFileTransportSource::Stopped:
			setupButton(playButton, "Play", true);
			setupButton(stopButton, "Stop", false);
			progressLine.stopTimer();
			progressLabel.stopTimer();
			waveform.clearSelectedRegion();
			break;

		case AudioFileTransportSource::Playing:
			setupButton(playButton, "Pause", true);
			setupButton(stopButton, "Stop", true);
			progressLine.startTimer(100);
			progressLabel.startTimer(100);
			break;

		case AudioFileTransportSource::Paused:
			setupButton(playButton, "Play", true);
			setupButton(stopButton, "Return To Zero", true);
			progressLine.stopTimer();
			progressLabel.stopTimer();
			break;

		case AudioFileTransportSource::Starting:
			break;

		case AudioFileTransportSource::Pausing:
			break;

		case AudioFileTransportSource::Stopping:
			break;

		case AudioFileTransportSource::NoFileLoaded:
			break;
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
	AudioFormatManager formatManager;
	AudioFileTransportSource audioSource;
	AudioWaveformComponent waveform;
	TrackProgressLabelComponent progressLabel;
	TrackProgressLineComponent progressLine;
	TrackScrollerComponent scroller;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
