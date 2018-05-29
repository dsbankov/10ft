/*
  ==============================================================================

    TenFtMainComponent.cpp
    Created: 29 May 2018 11:11:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "TenFtMainComponent.h"



TenFtMainComponent::TenFtMainComponent ()
    :
        waveform (),
        clock (waveform),
        playbackPosition (waveform),
        scroller (waveform)
{
    addAndMakeVisible (&openButton);
    openButton.setButtonText ("Open...");
    openButton.onClick = [this] {
        openButtonClicked ();
    };

    addAndMakeVisible (&playButton);
    playButton.setButtonText ("Play");
    playButton.onClick = [this] {
        playButtonClicked ();
    };
    playButton.setColour (TextButton::buttonColourId, Colours::green);
    playButton.setEnabled (false);

    addAndMakeVisible (&stopButton);
    stopButton.setButtonText ("Stop");
    stopButton.onClick = [this] {
        stopButtonClicked ();
    };
    stopButton.setColour (TextButton::buttonColourId, Colours::red);
    stopButton.setEnabled (false);

    addAndMakeVisible (&loopButton);
    loopButton.setButtonText ("Loop");
    loopButton.setToggleState (false, NotificationType::dontSendNotification);
    loopButton.onClick = [this] {
        loopButtonClicked ();
    };
    loopButton.setEnabled (false);

    // order is important for mouseDown events!
    //  (will go to the most upfront component (last added))
    addAndMakeVisible (&waveform);
    addAndMakeVisible (&playbackPosition);
    addAndMakeVisible (&scroller);
    addAndMakeVisible (&clock);

    //openGLContext.attachTo (*this);
    //openGLContext.attachTo (waveform);
    //openGLContext.attachTo (progressLine);

    setSize (1000, 800);
    setAudioChannels (0, 2);

    waveform.getAudioSource ().onStateChange = [this] (
        AudioFileTransportSource::AudioPlayerState state
    ) {
        onAudioPlayerStateChange (state);
    };
}

TenFtMainComponent::~TenFtMainComponent ()
{
    shutdownAudio ();
}

void TenFtMainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    waveform
        .getAudioSource ()
        .prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void TenFtMainComponent::getNextAudioBlock (const AudioSourceChannelInfo& bufferToFill)
{
    if (! waveform.getAudioSource ().isFileLoaded ())
    {
        bufferToFill.clearActiveBufferRegion ();

        return;
    }

    waveform.getAudioSource ().getNextAudioBlock (bufferToFill);
}

void TenFtMainComponent::releaseResources ()
{
    waveform.getAudioSource ().releaseResources ();
}

void TenFtMainComponent::resized ()
{
    juce::Rectangle<float> bounds = getLocalBounds ().toFloat ();
    float width = bounds.getWidth (),
        height = bounds.getHeight (),
        delta = 5.0f;
    juce::Rectangle<float> row1 = bounds.removeFromTop (0.05f * height),
        row2 = bounds.removeFromTop (0.05f * height),
        row4 = bounds.removeFromBottom (0.05f * height),
        row3 = bounds;

    openButton.setBounds (
        row1.reduced (delta).toNearestInt ()
    );
    playButton.setBounds (
        row2.removeFromLeft (width * 0.42).reduced (delta).toNearestInt ()
    );
    stopButton.setBounds (
        row2.removeFromLeft (width * 0.42).reduced (delta).toNearestInt ()
    );
    loopButton.setBounds (
        row2.removeFromLeft (width * 0.07).reduced (delta).toNearestInt ()
    );
    clock.setBounds (
        row2.reduced (delta).toNearestInt ()
    );
    waveform.setBounds (
        row3.reduced (delta).toNearestInt ()
    );
    playbackPosition.setBounds (
        row3.reduced (delta).toNearestInt ()
    );
    scroller.setBounds (
        row4.reduced (delta).toNearestInt ()
    );
}

// ==============================================================================

void TenFtMainComponent::openButtonClicked ()
{
    FileChooser chooser (
        "Select a Wave file to play...",
        File::getSpecialLocation (File::userMusicDirectory),
        "*.wav"
    );
    bool fileSelected = chooser.browseForFileToOpen ();

    if (fileSelected)
    {
        auto file = chooser.getResult ();

        if (waveform.loadAudio (file))
        {
            setupButton (playButton, "Play", true);
            setupButton (stopButton, "Stop", false);
            loopButton.setEnabled (true);
            clock.startTimer (100);
        }
        else
        {
            setupButton (playButton, "Play", false);
            setupButton (stopButton, "Stop", false);
            loopButton.setEnabled (false);
            loopButton.setToggleState (
                false,
                NotificationType::dontSendNotification
            );
            clock.stopTimer ();
            scroller.disable ();
        }
    }
}

void TenFtMainComponent::playButtonClicked ()
{
    waveform
        .getAudioSource ()
        .playAudio ();
}

void TenFtMainComponent::stopButtonClicked ()
{
    waveform
        .getAudioSource ()
        .stopAudio ();
}

void TenFtMainComponent::loopButtonClicked ()
{
    const bool shouldLoop = loopButton.getToggleState ();

    playbackPosition.setIsLooping (shouldLoop);
    waveform.getAudioSource ().setLooping (shouldLoop);
}

void TenFtMainComponent::onAudioPlayerStateChange (AudioFileTransportSource::AudioPlayerState state)
{
    if (state == AudioFileTransportSource::Stopped)
    {
        setupButton (playButton, "Play", true);
        setupButton (stopButton, "Stop", false);
        playbackPosition.stopTimer ();
        clock.stopTimer ();
        waveform.clearSelectedRegion ();
    }
    else if (state == AudioFileTransportSource::Playing)
    {
        setupButton (playButton, "Pause", true);
        setupButton (stopButton, "Stop", true);
        playbackPosition.startTimer (100);
        clock.startTimer (100);
    }
    else if (state == AudioFileTransportSource::Paused)
    {
        setupButton (playButton, "Play", true);
        setupButton (stopButton, "Return To Zero", true);
        playbackPosition.stopTimer ();
        clock.stopTimer ();
    }
}

void TenFtMainComponent::setupButton (TextButton& button, std::string buttonText, bool enabled)
{
    button.setButtonText (buttonText);
    button.setEnabled (enabled);
}