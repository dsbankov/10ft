/*
  ==============================================================================

    TrackProgressLineComponent.h
    Created: 20 May 2018 3:33:37pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"


class AudioPlaybackPositionComponent :    public Component,
                                          public Timer,
                                          public ChangeListener
{
public:
    AudioPlaybackPositionComponent (AudioWaveformComponent & waveform)
        :
            waveform (waveform),
            isLooping (false)
    {
        // so we can handle mouse events for the component behind it (AudioWaveformComponent)
        setInterceptsMouseClicks (false, true);

        waveform.addChangeListener (this);
    }

    ~AudioPlaybackPositionComponent ()
    {
    }

    void paint (Graphics& g) override
    {
        auto startTimeSeconds = waveform.getVisibleRegionStartTime (),
            endTimeSeconds = waveform.getVisibleRegionEndTime (),
            audioLengthSeconds = endTimeSeconds - startTimeSeconds;

        if (audioLengthSeconds > 0)
        {
            juce::Rectangle<float> localBounds = getLocalBounds ().toFloat ();
            auto currentPosition = waveform.getAudioSource ().getCurrentPosition (),
                drawPosition = ((currentPosition - startTimeSeconds) / audioLengthSeconds)
                    * localBounds.getWidth () + localBounds.getX ();

            g.setColour (Colours::green);
            g.drawLine (
                drawPosition,
                localBounds.getY (),
                drawPosition,
                localBounds.getBottom (),
                3.0f
            );
        } 
    }

    void stopTimer ()
    {
        Timer::stopTimer ();
        repaint ();
    }

    void setIsLooping (bool isLooping)
    {
        this->isLooping = isLooping;
    }

private:
    void timerCallback () override
    {
        respondToChange ();
    }

    void changeListenerCallback (ChangeBroadcaster *source) override
    {
        if (source == &waveform)
        {
            respondToChange ();
        }
    }

    void respondToChange ()
    {
        auto currentPosition = waveform.getAudioSource ().getCurrentPosition ();

        if (
            waveform.getHasSelectedRegion () &&
            currentPosition >= waveform.getSelectedRegionEndTime ()
        )
        {
            waveform.getAudioSource ().setPosition (waveform.getSelectedRegionStartTime ());
            if (! isLooping)
            {
                waveform.getAudioSource ().pauseAudio ();
            }
        }

        repaint ();
    }

private:
    AudioWaveformComponent& waveform;
    bool isLooping;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlaybackPositionComponent)
};
