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
    if (state == Recording)
    {
        recordingBufferPreallocationThread->stopThread (1000);
    }

    masterSource.releaseResources ();
}

void TenFtAudioSource::getNextAudioBlock (
    const AudioSourceChannelInfo& bufferToFill
)
{
    if (state == TenFtAudioSource::State::NoAudioLoaded)
    {
        bufferToFill.clearActiveBufferRegion ();
    }
    else
    {
        masterSource.getNextAudioBlock (bufferToFill);
    }
}

void TenFtAudioSource::getNextAudioBlock (
    const AudioSourceChannelInfo& bufferToFill,
    AudioDeviceManager& deviceManager
)
{
    if (state == Recording)
    {
        AudioIODevice* device = deviceManager.getCurrentAudioDevice ();
        BigInteger activeInputChannels = device->getActiveInputChannels (),
            activeOutputChannels = device->getActiveOutputChannels ();
        int maxInputChannels = activeInputChannels.getHighestBit () + 1,
            maxOutputChannels = activeOutputChannels.getHighestBit () + 1,
            destChannel = 0;

        for (int channel = 0; channel < maxOutputChannels; ++channel)
        {
            if ((!activeOutputChannels[channel]) || maxInputChannels == 0)
            {
                bufferToFill.buffer->clear (
                    channel, bufferToFill.startSample, bufferToFill.numSamples
                );
            }
            else
            {
                int actualInputChannel = channel % maxInputChannels;

                if (!activeInputChannels[channel])
                {
                    bufferToFill.buffer->clear (
                        channel, bufferToFill.startSample, bufferToFill.numSamples
                    );
                }
                else
                {
                    Logger::outputDebugString ("ENTER getNextAudioBlock");
                    preallocatedRecordingBuffer.copyFrom (
                        destChannel++,
                        numSamplesRecorded,
                        *bufferToFill.buffer,
                        actualInputChannel,
                        bufferToFill.startSample,
                        bufferToFill.numSamples
                    );

                    numSamplesRecorded += bufferToFill.numSamples;
                    
                    buffer->setDataToReferTo (
                        preallocatedRecordingBuffer.getArrayOfWritePointers (),
                        preallocatedRecordingBuffer.getNumChannels (),
                        numSamplesRecorded
                    );
                    Logger::outputDebugString ("EXIT getNextAudioBlock");
                }
            }
        }
    }

    getNextAudioBlock (bufferToFill);
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
    numSamplesRecorded = 0;

    preallocatedRecordingBuffer.setSize (
        buffer->getNumChannels(),
        (int)(60 * sampleRate)
    );
    recordingBufferPreallocationThread.reset (
        new BufferPreallocationThread (
            preallocatedRecordingBuffer,
            numSamplesRecorded,
            (int)(10 * sampleRate),
            (int)(30 * sampleRate),
            *buffer
        )
    );
    recordingBufferPreallocationThread->startThread ();

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
    recordingBufferPreallocationThread->stopThread (1000);
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

const CriticalSection* TenFtAudioSource::getBufferUpdateLock () const noexcept
{
    if (recordingBufferPreallocationThread)
    {
        return &recordingBufferPreallocationThread->getLock();
    }
    else
    {
        return nullptr;
    }
}

// ==============================================================================

void TenFtAudioSource::selectedRegionCreated (AudioWaveformComponent* waveform)
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

void TenFtAudioSource::timerCallback ()
{
    listeners.call ([this](Listener& l) { l.currentPositionChanged (this); });
}

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

// ==============================================================================

TenFtAudioSource::BufferPreallocationThread::BufferPreallocationThread (
    AudioSampleBuffer& preallocatedRecordingBuffer,
    int& numSamplesRecorded,
    int numSamplesBuffer,
    int numSamplesToAllocate,
    AudioSampleBuffer& buffer
) :
    Thread ("BufferPreallocationThread"),
    preallocatedRecordingBuffer (preallocatedRecordingBuffer),
    numSamplesRecorded (numSamplesRecorded),
    numSamplesBuffer (numSamplesBuffer),
    numSamplesToAllocate (numSamplesToAllocate),
    buffer(buffer)
{
}

void TenFtAudioSource::BufferPreallocationThread::run ()
{
    while (!threadShouldExit ())
    {
        int preallocatedSamples = preallocatedRecordingBuffer.getNumSamples ();
        if (preallocatedSamples - numSamplesRecorded < numSamplesBuffer)
        {
            int newNumSamples =
                preallocatedRecordingBuffer.getNumSamples () + numSamplesToAllocate;
            const ScopedLock scopedLock (bufferUpdateLock);
            Logger::outputDebugString ("ENTER BufferPreallocationThread");
            preallocatedRecordingBuffer.setSize (
                preallocatedRecordingBuffer.getNumChannels (),
                newNumSamples, true
            );
            buffer.setDataToReferTo (
                preallocatedRecordingBuffer.getArrayOfWritePointers (),
                preallocatedRecordingBuffer.getNumChannels (),
                numSamplesRecorded
            );
            Logger::outputDebugString ("EXIT BufferPreallocationThread");
        }
        wait (1000);
    }
}

const CriticalSection& TenFtAudioSource::BufferPreallocationThread::getLock () const noexcept
{
    return bufferUpdateLock;
}
