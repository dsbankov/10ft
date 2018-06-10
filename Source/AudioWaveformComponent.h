/*
  ==============================================================================

    AudioWaveformComponent.h
    Created: 20 May 2018 3:33:07pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"

#include "TenFtAudioTransportSource.h"
#include "AudioPlaybackPositionComponent.h"


class AudioWaveformComponent :    public Component,
                                  public ChangeBroadcaster,
                                  private ChangeListener
{
public:
    enum ColourIds
    {
        waveformColour = 0,
        waveformBackgroundColour = 1,
        waveformSelectedRegionBackgroundColour = 2
    };

public:
    AudioWaveformComponent ();

    ~AudioWaveformComponent ();

    void paint (Graphics& g) override;

    void resized () override;

    void mouseWheelMove (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) override;

    void mouseDoubleClick (const MouseEvent& event) override;

    void mouseDrag (const MouseEvent& event) override;

    void mouseDown (const MouseEvent& event) override;

    bool loadAudio (File file);

    void updateVisibleRegion (
        float visibleRegionStartTime,
        float visibleRegionEndTime
    );

    void clearSelectedRegion ();

    TenFtAudioTransportSource& getAudioSource ();

    float getVisibleRegionStartTime ();

    float getVisibleRegionEndTime ();

    float getSelectedRegionStartTime ();

    float getSelectedRegionEndTime ();

    bool getHasSelectedRegion ();

    AudioPlaybackPositionComponent& getPlaybackPositionComponent ();

private:
    void changeListenerCallback (
        ChangeBroadcaster* source
    ) override;

    void paintIfNoFileLoaded (Graphics& g);

    void paintIfFileLoaded (Graphics& g);

    float getVisibleRegionLengthSeconds ();

    void updateSelectedRegion (float mouseDownSeconds);

    bool isVisibleRegionCorrect (float visibleRegionStartTime, float visibleRegionEndTime);

    float xToSeconds (float x);

    float secondsToX (float s);

    float flattenSeconds (float s);

    float flattenX (float x);

private:
    AudioFormatManager formatManager;
    TenFtAudioTransportSource audioSource;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    float visibleRegionStartTime;
    float visibleRegionEndTime;
    bool hasSelectedRegion;
    float selectedRegionStartTime;
    float selectedRegionEndTime;
    AudioPlaybackPositionComponent playbackPosition;
    OpenGLContext openGLContext;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformComponent)
};
