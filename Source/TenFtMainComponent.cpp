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
        audioSource (),
        waveform (),
        selectedRegion (),
        playbackPosition (),
        clock (),
        scroller ()
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

    audioSource.addListener (&clock);
    audioSource.addListener (&playbackPosition);
    audioSource.onStateChange = [this] (
        TenFtAudioTransportSource::State state
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
        row4 = bounds.removeFromBottom (0.05f * height),
        row3 = bounds;

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
    waveform.setBounds (
        row3.reduced (delta).toNearestInt ()
    );
    selectedRegion.setBounds (
        waveform.getBounds ()
    );
    playbackPosition.setBounds (
        waveform.getBounds ()
    );
    scroller.setBounds (
        row4.reduced (delta).toNearestInt ()
    );
}

void TenFtMainComponent::paint (Graphics & g)
{
    g.fillAll (findColour (
        AudioWaveformComponent::ColourIds::waveformBackgroundColour
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

        if (audioSource.loadAudio (file))
        {
            waveform.loadThumbnail (file);
            setupButton (playButton, "Play", true);
            setupButton (stopButton, "Stop", false);
            loopButton.setEnabled (true);
        }
        else
        {
            waveform.clearThumbnail ();
            scroller.disable ();
            setupButton (playButton, "Play", false);
            setupButton (stopButton, "Stop", false);
            loopButton.setEnabled (false);
            loopButton.setToggleState (
                false,
                NotificationType::dontSendNotification
            );
        }
    }
}

void TenFtMainComponent::loopButtonClicked ()
{
    const bool shouldLoop = loopButton.getToggleState ();

    if (shouldLoop)
    {
        double startTime = waveform.getHasSelectedRegion () ?
            waveform.getSelectedRegionStartTime () : 0.0;
        double endTime = waveform.getHasSelectedRegion () ?
            waveform.getSelectedRegionEndTime () : waveform.getTotalLength ();
        audioSource.setupLooping (startTime, endTime);
    }
    else
    {
        audioSource.disableLooping ();
    }
}

void TenFtMainComponent::onAudioSourceStateChange (
    TenFtAudioTransportSource::State state
)
{
    if (state == TenFtAudioTransportSource::Stopped)
    {
        setupButton (playButton, "Play", true);
        setupButton (stopButton, "Stop", false);
        waveform.clearSelectedRegion ();
    }
    else if (state == TenFtAudioTransportSource::Playing)
    {
        setupButton (playButton, "Pause", true);
        setupButton (stopButton, "Stop", true);
    }
    else if (state == TenFtAudioTransportSource::Paused)
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
