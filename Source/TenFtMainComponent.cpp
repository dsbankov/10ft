/*
  ==============================================================================

    TenFtMainComponent.cpp
    Created: 29 May 2018 11:11:55am
    Author:  Nikolay Tsenkov

  ==============================================================================
*/


#include "TenFtMainComponent.h"


TenFtMainComponent::TenFtMainComponent ()
{
    setLookAndFeel (&tenFtLookAndFeel);

    addAndMakeVisible (&openButton);
    openButton.setButtonText ("Open...");
    openButton.onClick = [this] {
        openButtonClicked ();
    };

    addAndMakeVisible (&playButton);
    playButton.setButtonText ("Play");
    playButton.onClick = [this] {
        audioSource.playAudio ();
    };
    playButton.setEnabled (false);

    addAndMakeVisible (&stopButton);
    stopButton.setButtonText ("Stop");
    stopButton.onClick = [this] {
        audioSource.stopAudio ();
    };
    stopButton.setEnabled (false);

    addAndMakeVisible (&loopButton);
    loopButton.setButtonText ("Loop");
    loopButton.changeWidthToFitText ();
    loopButton.setToggleState (false, 
        NotificationType::dontSendNotification
    );
    loopButton.onClick = [this] {
        loopButtonClicked ();
    };
    loopButton.setEnabled (false);

    addAndMakeVisible (&muteButton);
    muteButton.setButtonText ("Mute");
    muteButton.onClick = [this] {
        audioSource.muteAudio ();
        waveform.refresh ();
    };
    muteButton.setEnabled (false);

    addAndMakeVisible (&fadeInButton);
    fadeInButton.setButtonText ("Fade In");
    fadeInButton.onClick = [this] {
        audioSource.fadeInAudio ();
        waveform.refresh ();
    };
    fadeInButton.setEnabled (false);

    addAndMakeVisible (&fadeOutButton);
    fadeOutButton.setButtonText ("Fade Out");
    fadeOutButton.onClick = [this] {
        audioSource.fadeOutAudio ();
        waveform.refresh ();
    };
    fadeOutButton.setEnabled (false);

    addAndMakeVisible (&normalizeButton);
    normalizeButton.setButtonText ("Normalize");
    normalizeButton.onClick = [this] {
        audioSource.normalizeAudio ();
        waveform.refresh ();
    };
    normalizeButton.setEnabled (false);

    formatManager.registerBasicFormats ();

    audioSource.addListener (&clock);
    audioSource.addListener (&playbackPosition);
    audioSource.onStateChange = [this] (
        TenFtAudioSource::State state
    ) {
        onAudioSourceStateChange (state);
    };

    addAndMakeVisible (&waveform);
    addAndMakeVisible (&scroller);
    addAndMakeVisible (&clock);
    waveform.addAndMakeVisible (&selectedRegion);
    waveform.addAndMakeVisible (&playbackPosition);

    waveform.addListener (&audioSource);
    waveform.addListener (&scroller);
    waveform.addListener (&selectedRegion);
    waveform.addListener (&playbackPosition);
    waveform.onPositionChange = [this] (double newPosition) {
        audioSource.setPosition (newPosition);
    };

    scroller.addListener (&waveform);
    scroller.onMouseWheelMove = [this] (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) {
        waveform.mouseWheelMove (event, wheelDetails);
    };

    setSize (1000, 800);
    setAudioChannels (0, 2);
}

TenFtMainComponent::~TenFtMainComponent ()
{
    shutdownAudio ();
    setLookAndFeel (nullptr);
}

void TenFtMainComponent::prepareToPlay (
    int samplesPerBlockExpected,
    double sampleRate
)
{
    audioSource.prepareToPlay (samplesPerBlockExpected, sampleRate);
}

void TenFtMainComponent::getNextAudioBlock (
    const AudioSourceChannelInfo& bufferToFill
)
{
    if (!audioSource.isAudioLoaded ())
    {
        bufferToFill.clearActiveBufferRegion ();

        return;
    }

    audioSource.getNextAudioBlock (bufferToFill);
}

void TenFtMainComponent::releaseResources ()
{
    audioSource.releaseResources ();
}

void TenFtMainComponent::resized ()
{
    juce::Rectangle<float> bounds =
        getLocalBounds ().toFloat ().reduced (10.0f);
    float width = bounds.getWidth (),
        height = bounds.getHeight (),
        delta = 5.0f;
    juce::Rectangle<float> row1 = bounds.removeFromTop (0.05f * height),
        row2 = bounds.removeFromTop (0.05f * height),
        row3 = bounds.removeFromTop (0.05f * height),
        row5 = bounds.removeFromBottom (0.05f * height),
        row4 = bounds;

    openButton.setBounds (
        row1.reduced (delta).toNearestInt ()
    );
    playButton.setBounds (
        row2.removeFromLeft (width * 0.42f).reduced (delta).toNearestInt ()
    );
    stopButton.setBounds (
        row2.removeFromLeft (width * 0.42f).reduced (delta).toNearestInt ()
    );
    loopButton.setBounds (
        row2.removeFromLeft (width * 0.07f).reduced (delta).toNearestInt ()
    );
    clock.setBounds (
        row2.reduced (delta).toNearestInt ()
    );
    muteButton.setBounds (
        row3.removeFromLeft(width / 4.0f).reduced (delta).toNearestInt ()
    );
    fadeInButton.setBounds (
        row3.removeFromLeft (width / 4.0f).reduced (delta).toNearestInt ()
    );
    fadeOutButton.setBounds (
        row3.removeFromLeft (width / 4.0f).reduced (delta).toNearestInt ()
    );
    normalizeButton.setBounds (
        row3.removeFromLeft (width / 4.0f).reduced (delta).toNearestInt ()
    );
    waveform.setBounds (
        row4.reduced (delta).toNearestInt ()
    );
    selectedRegion.setBounds (
        waveform.getBounds ()
    );
    playbackPosition.setBounds (
        waveform.getBounds ()
    );
    scroller.setBounds (
        row5.reduced (delta).toNearestInt ()
    );
}

void TenFtMainComponent::paint (Graphics & g)
{
    g.fillAll (findColour (
        AudioWaveformOpenGLComponent::ColourIds::waveformBackgroundColour
    ).contrasting(0.2f));
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
        File file = chooser.getResult ();

        std::unique_ptr<AudioFormatReader> audioReader(formatManager.createReaderFor (file));

        if (audioReader != nullptr)
        {
            std::unique_ptr<AudioSampleBuffer> tempAudioBuffer (
                new AudioSampleBuffer (
                    audioReader->numChannels,
                    audioReader->lengthInSamples
                )
            );
            audioReader->read (tempAudioBuffer.get (), 0, audioReader->lengthInSamples, 0, true, true);

            audioSource.loadAudio (tempAudioBuffer.get (), audioReader->sampleRate);
            waveform.loadWaveform (tempAudioBuffer.get (), audioReader->sampleRate);

            audioBuffer.swap (tempAudioBuffer);

            setupButton (playButton, "Play", true);
            setupButton (stopButton, "Stop", false);
            loopButton.setEnabled (true);
            muteButton.setEnabled (true);
            fadeInButton.setEnabled (true);
            fadeOutButton.setEnabled (true);
            normalizeButton.setEnabled (true);
        }
        else
        {
            waveform.clearWaveform ();
            audioSource.unloadAudio ();

            scroller.disable ();
            setupButton (playButton, "Play", false);
            setupButton (stopButton, "Stop", false);
            loopButton.setEnabled (false);
            loopButton.setToggleState (
                false,
                NotificationType::dontSendNotification
            );
            muteButton.setEnabled (false);
            fadeInButton.setEnabled (false);
            fadeOutButton.setEnabled (false);
            normalizeButton.setEnabled (false);
        }
    }
}

void TenFtMainComponent::loopButtonClicked ()
{
    audioSource.setLooping (loopButton.getToggleState ());
}

void TenFtMainComponent::onAudioSourceStateChange (
    TenFtAudioSource::State state
)
{
    if (state == TenFtAudioSource::Stopped)
    {
        setupButton (playButton, "Play", true);
        setupButton (stopButton, "Stop", false);
        waveform.clearSelectedRegion ();
    }
    else if (state == TenFtAudioSource::Playing)
    {
        setupButton (playButton, "Pause", true);
        setupButton (stopButton, "Stop", true);
    }
    else if (state == TenFtAudioSource::Paused)
    {
        setupButton (playButton, "Play", true);
        setupButton (stopButton, "Return To Zero", true);
    }
}

void TenFtMainComponent::setupButton (
    TextButton& button, std::string buttonText, bool enabled
)
{
    button.setButtonText (buttonText);
    button.setEnabled (enabled);
}
