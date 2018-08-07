/*
==============================================================================

AudioFileTransportSource.cpp
Created: 29 May 2018 11:17:55am
Author:  Nikolay Tsenkov

==============================================================================
*/


#include "TenFtAudioTransportSource.h"


TenFtAudioTransportSource::TenFtAudioTransportSource ()
{
    addChangeListener (this);
    startTimer (100);
}

TenFtAudioTransportSource::~TenFtAudioTransportSource ()
{
    stopTimer ();
    setSource (nullptr);
    audioBuffer = nullptr;
}

void TenFtAudioTransportSource::loadAudio (
    AudioSampleBuffer* newAudioSampleBuffer,
    double newSampleRate
)
{
    audioBuffer = newAudioSampleBuffer;
    sampleRate = newSampleRate;
    loadAudioSubsection (0.0, getLengthInSecondsGlobal (), false, false);
}

void TenFtAudioTransportSource::unloadAudio ()
{
    changeState (NoFileLoaded);
}

bool TenFtAudioTransportSource::isAudioLoaded ()
{
    return state != NoFileLoaded;
}

void TenFtAudioTransportSource::playAudio ()
{
    if (state == Playing)
    {
        changeState (Pausing);
    }
    else
    {
        changeState (Starting);
    }
}

void TenFtAudioTransportSource::stopAudio ()
{
    if (hasSubsection && state == Paused)
    {
        if (getCurrentPosition () == 0.0)
        {
            loadAudioSubsection (
                0.0,
                getLengthInSecondsGlobal (),
                false,
                audioSource->isLooping ()
            );
            changeState (Stopped);
            stop ();
        }
        else
        {
            setPosition (0.0);
        }
    }
    else if (state == Paused || state == NoFileLoaded)
    {
        changeState (Stopped);
    }
    else
    {
        changeState (Stopping);
    }
}

void TenFtAudioTransportSource::pauseAudio ()
{
    changeState (Pausing);
}

void TenFtAudioTransportSource::muteAudio ()
{
    int64 startSample = subsectionStartTime * sampleRate,
        numSamples = (subsectionEndTime - subsectionStartTime) * sampleRate;

    audioBuffer->clear (startSample, numSamples);
}

void TenFtAudioTransportSource::fadeInAudio ()
{
    int64 startSample = subsectionStartTime * sampleRate,
        numSamples = (subsectionEndTime - subsectionStartTime) * sampleRate;
    float magnitude = audioBuffer->getMagnitude (startSample, numSamples),
        gain = Decibels::decibelsToGain (magnitude);

    audioBuffer->applyGainRamp (startSample, numSamples, 0.0f, gain);
}

void TenFtAudioTransportSource::fadeOutAudio ()
{
    int64 startSample = subsectionStartTime * sampleRate,
        numSamples = (subsectionEndTime - subsectionStartTime) * sampleRate;
    float magnitude = audioBuffer->getMagnitude (startSample, numSamples),
        gain = Decibels::decibelsToGain (magnitude);

    audioBuffer->applyGainRamp (startSample, numSamples, gain, 0.0f);
}

void TenFtAudioTransportSource::normalizeAudio ()
{
    int64 startSample = subsectionStartTime * sampleRate,
        numSamples = (subsectionEndTime - subsectionStartTime) * sampleRate;
    float magnitude = audioBuffer->getMagnitude (startSample, numSamples),
        gain = Decibels::decibelsToGain (magnitude);

    audioBuffer->applyGain (startSample, numSamples, gain);
}

double TenFtAudioTransportSource::getCurrentPositionGlobal () const
{
    double currentPosition = getCurrentPosition ();
    if (hasSubsection)
    {
        currentPosition += subsectionStartTime;
    }
    return currentPosition;
}

double TenFtAudioTransportSource::getLengthInSecondsGlobal () const
{
    if (audioBuffer != nullptr)
    {
        return audioBuffer->getNumSamples() / sampleRate;
    }
    else
    {
        return 0.0;
    }
}

void TenFtAudioTransportSource::setLooping (bool shouldLoop)
{
    audioSource->setLooping (shouldLoop);
}

void TenFtAudioTransportSource::selectedRegionCreated (
    AudioWaveformComponent* waveform
)
{
    loadAudioSubsection (
        waveform->getSelectedRegionStartTime (), // TODO fix negative number here?
        waveform->getSelectedRegionEndTime (),
        true,
        audioSource->isLooping ()
    );

    if (state == Playing)
    {
        start ();
    }
}

void TenFtAudioTransportSource::selectedRegionCleared (AudioWaveformComponent* waveform)
{
    if (hasSubsection && !waveform->getHasSelectedRegion ())
    {
        loadAudioSubsection (
            0.0,
            getLengthInSecondsGlobal (),
            false,
            audioSource->isLooping ()
        );

        start ();
    }
}

void TenFtAudioTransportSource::addListener (Listener * newListener)
{
    listeners.add (newListener);
}

void TenFtAudioTransportSource::removeListener (Listener * listener)
{
    listeners.remove (listener);
}

// ==============================================================================

void TenFtAudioTransportSource::changeListenerCallback (
    ChangeBroadcaster*
)
{
    if (isPlaying ())
    {
        changeState (Playing);
    }
    else if (state == Pausing)
    {
        changeState (Paused);
    }
    else if (hasSubsection)
    {
        setPosition (0.0);
        changeState (Paused);
    }
    else
    {
        changeState (Stopped);
    }
}

void TenFtAudioTransportSource::timerCallback ()
{
    listeners.call ([this](Listener& l) { l.currentPositionChanged (this); });
}

void TenFtAudioTransportSource::changeState (
    TenFtAudioTransportSource::State newState
)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
        case Starting:
            start ();
            break;

        case Playing:
            break;

        case Stopping:
            stop ();
            break;

        case Stopped:
            setPosition (0.0);
            break;

        case Pausing:
            stop ();
            break;

        case Paused:
            break;

        case NoFileLoaded:
            setSource (nullptr);
            audioBuffer = nullptr;
            sampleRate = 0.0;
            break;
        }

        onStateChange (newState);

        listeners.call ([this](Listener& l) { l.stateChanged (this); });
    }
}

void TenFtAudioTransportSource::loadAudioSubsection (
    double startTime,
    double endTime,
    bool subsectionSelected,
    bool shouldLoop
)
{
    subsectionStartTime = startTime;
    subsectionEndTime = endTime;
    hasSubsection = subsectionSelected;

    double lengthInSeconds = subsectionEndTime - subsectionStartTime;
    int startSample = (int) (subsectionStartTime * sampleRate),
        numSamples = (int) (lengthInSeconds * sampleRate);

    subsectionAudioBuffer.reset (
        new AudioSampleBuffer (
            audioBuffer->getArrayOfWritePointers(),
            audioBuffer->getNumChannels(),
            startSample,
            numSamples
        )
    );
    
    std::unique_ptr<AudioBufferSource> tempAudioSource (
        new AudioBufferSource (subsectionAudioBuffer.get (), shouldLoop)
    );

    setSource (
        tempAudioSource.get (),
        0,
        nullptr,
        sampleRate
    );

    audioSource.swap (tempAudioSource);
}