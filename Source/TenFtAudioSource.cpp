/*
==============================================================================

AudioFileTransportSource.cpp
Created: 29 May 2018 11:17:55am
Author:  Nikolay Tsenkov

==============================================================================
*/


#include "TenFtAudioSource.h"


TenFtAudioSource::TenFtAudioSource ()
{
    masterSource.addChangeListener (this);
    startTimer (100);
}

TenFtAudioSource::~TenFtAudioSource ()
{
    stopTimer ();
    masterSource.setSource (nullptr);
    buffer = nullptr;
}

void TenFtAudioSource::prepareToPlay (
    int samplesPerBlockExpected,
    double masterSourceSampleRate
)
{
    masterSource.prepareToPlay (
        samplesPerBlockExpected, masterSourceSampleRate);
}

void TenFtAudioSource::releaseResources ()
{
    masterSource.releaseResources ();
}

void TenFtAudioSource::getNextAudioBlock (
    const AudioSourceChannelInfo& bufferToFill
)
{
    masterSource.getNextAudioBlock (bufferToFill);
}

void TenFtAudioSource::loadAudio (
    AudioSampleBuffer* newAudioSampleBuffer,
    double newSampleRate
)
{
    buffer = newAudioSampleBuffer;
    sampleRate = newSampleRate;
    loadAudioSubregion (0.0, getLengthInSeconds (), false, false);
}

void TenFtAudioSource::unloadAudio ()
{
    changeState (NoAudioLoaded);
}

void TenFtAudioSource::loadRecordingBuffer (
    AudioSampleBuffer* newAudioSampleBuffer,
    double newSampleRate
)
{
    buffer = newAudioSampleBuffer;
    sampleRate = newSampleRate;

    std::unique_ptr<AudioBufferSource> tempBufferSource (
        new AudioBufferSource (buffer, false)
    );

    masterSource.setSource (
        tempBufferSource.get (),
        0,
        nullptr,
        sampleRate
    );

    bufferSource.swap (tempBufferSource);

    changeState (StartRecording);
}

void TenFtAudioSource::stopRecording ()
{
    changeState (Stopping);
}

void TenFtAudioSource::playAudio ()
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

void TenFtAudioSource::stopAudio ()
{
    if (hasSubregion && state == Paused)
    {
        if (masterSource.getCurrentPosition () == 0.0)
        {
            loadAudioSubregion (
                0.0,
                getLengthInSeconds (),
                false,
                bufferSource->isLooping ()
            );
            changeState (Stopped);
            masterSource.stop ();
        }
        else
        {
            setPosition (0.0);
        }
    }
    else if (state == Paused || state == NoAudioLoaded)
    {
        changeState (Stopped);
    }
    else
    {
        changeState (Stopping);
    }
}

void TenFtAudioSource::pauseAudio ()
{
    changeState (Pausing);
}

void TenFtAudioSource::muteAudio ()
{
    int startSample = (int)(subregionStartTime * sampleRate),
        numSamples = (int)((subregionEndTime - subregionStartTime) * sampleRate);

    buffer->clear (startSample, numSamples);
}

void TenFtAudioSource::fadeInAudio ()
{
    int startSample = (int)(subregionStartTime * sampleRate),
        numSamples = (int)((subregionEndTime - subregionStartTime) * sampleRate);
    float magnitude = buffer->getMagnitude (startSample, numSamples),
        gain = Decibels::decibelsToGain (magnitude);

    buffer->applyGainRamp (startSample, numSamples, 0.0f, gain);
}

void TenFtAudioSource::fadeOutAudio ()
{
    int startSample = (int)(subregionStartTime * sampleRate),
        numSamples = (int)((subregionEndTime - subregionStartTime) * sampleRate);
    float magnitude = buffer->getMagnitude (startSample, numSamples),
        gain = Decibels::decibelsToGain (magnitude);

    buffer->applyGainRamp (startSample, numSamples, gain, 0.0f);
}

void TenFtAudioSource::normalizeAudio ()
{
    int startSample = (int)(subregionStartTime * sampleRate),
        numSamples = (int)((subregionEndTime - subregionStartTime) * sampleRate);
    float magnitude = buffer->getMagnitude (startSample, numSamples),
        gain = Decibels::decibelsToGain (magnitude);

    buffer->applyGain (startSample, numSamples, gain);
}

TenFtAudioSource::State TenFtAudioSource::getState () const
{
    return state;
}

double TenFtAudioSource::getCurrentPosition () const
{
    double currentPosition = masterSource.getCurrentPosition ();
    if (hasSubregion)
    {
        currentPosition += subregionStartTime;
    }
    return currentPosition;
}

double TenFtAudioSource::getLengthInSeconds () const
{
    if (buffer != nullptr)
    {
        return buffer->getNumSamples() / sampleRate;
    }
    else
    {
        return 0.0;
    }
}

void TenFtAudioSource::setPosition (double newPosition)
{
    masterSource.setPosition (newPosition);
}

void TenFtAudioSource::setLooping (bool shouldLoop)
{
    bufferSource->setLooping (shouldLoop);
}

void TenFtAudioSource::addListener (Listener * newListener)
{
    listeners.add (newListener);
}

void TenFtAudioSource::removeListener (Listener * listener)
{
    listeners.remove (listener);
}

// ==============================================================================

void TenFtAudioSource::changeListenerCallback (
    ChangeBroadcaster*
)
{
    if (masterSource.isPlaying () && state == StartRecording)
    {
        changeState (Recording);
    }
    else if (masterSource.isPlaying () && state != StartRecording)
    {
        changeState (Playing);
    }
    else if (state == Pausing)
    {
        changeState (Paused);
    }
    else if (hasSubregion)
    {
        setPosition (0.0);
        changeState (Paused);
    }
    else
    {
        changeState (Stopped);
    }
}

void TenFtAudioSource::timerCallback ()
{
    listeners.call ([this](Listener& l) { l.currentPositionChanged (this); });
}

void TenFtAudioSource::selectedRegionCreated (
    AudioWaveformComponent* waveform
)
{
    loadAudioSubregion (
        waveform->getSelectedRegionStartTime (),
        waveform->getSelectedRegionEndTime (),
        true,
        bufferSource->isLooping ()
    );

    if (state == Playing)
    {
        masterSource.start ();
    }
}

void TenFtAudioSource::selectedRegionCleared (AudioWaveformComponent* waveform)
{
    if (hasSubregion && !waveform->getHasSelectedRegion ())
    {
        loadAudioSubregion (
            0.0,
            getLengthInSeconds (),
            false,
            bufferSource->isLooping ()
        );

        masterSource.start ();
    }
}

void TenFtAudioSource::changeState (
    TenFtAudioSource::State newState
)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
        case StartRecording:
            masterSource.start ();
            break;

        case Recording:
            break;

        case Starting:
            masterSource.start ();
            break;

        case Playing:
            break;

        case Stopping:
            masterSource.stop ();
            break;

        case Stopped:
            setPosition (0.0);
            break;

        case Pausing:
            masterSource.stop ();
            break;

        case Paused:
            break;

        case NoAudioLoaded:
            masterSource.setSource (nullptr);
            buffer = nullptr;
            sampleRate = 0.0;
            break;
        }

        onStateChange (newState);

        listeners.call ([this](Listener& l) { l.stateChanged (this); });
    }
}

void TenFtAudioSource::loadAudioSubregion (
    double startTime,
    double endTime,
    bool subregionSelected,
    bool shouldLoop
)
{
    subregionStartTime = startTime;
    subregionEndTime = endTime;
    hasSubregion = subregionSelected;

    double lengthInSeconds = subregionEndTime - subregionStartTime;
    int startSample = (int) (subregionStartTime * sampleRate),
        numSamples = (int) (lengthInSeconds * sampleRate);

    subregionBuffer.reset (
        new AudioSampleBuffer (
            buffer->getArrayOfWritePointers(),
            buffer->getNumChannels(),
            startSample,
            numSamples
        )
    );
    
    std::unique_ptr<AudioBufferSource> tempBufferSource (
        new AudioBufferSource (subregionBuffer.get (), shouldLoop)
    );

    masterSource.setSource (
        tempBufferSource.get (),
        0,
        nullptr,
        sampleRate
    );

    bufferSource.swap (tempBufferSource);
}