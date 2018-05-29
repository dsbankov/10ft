/*
  ==============================================================================

    AudioWaveformComponent.h
    Created: 20 May 2018 3:33:07pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"

#include "AudioFileTransportSource.h"


class AudioWaveformComponent :    public Component,
                                  public ChangeBroadcaster,
                                  private ChangeListener
{
public:
    AudioWaveformComponent ();

    ~AudioWaveformComponent ();

    void paint (Graphics& g) override;

    void mouseWheelMove (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) override;

    void mouseDoubleClick (const MouseEvent& event) override;

    void mouseDrag (const MouseEvent &event) override;

    void mouseDown (const MouseEvent &event) override;

    bool loadAudio (File file);

    void setVisibleRegion (
        float visibleRegionStartTime,
        float visibleRegionEndTime
    );

    void clearSelectedRegion ();

    AudioFileTransportSource& getAudioSource ();

    float getVisibleRegionStartTime ();

    float getVisibleRegionEndTime ();

    float getSelectedRegionStartTime ();

    float getSelectedRegionEndTime ();

    bool getHasSelectedRegion ();

private:
    void changeListenerCallback (
        ChangeBroadcaster* source
    ) override;

    void paintIfNoFileLoaded (
        Graphics& g,
        juce::Rectangle<int> thumbnailBounds
    );

    void paintIfFileLoaded (
        Graphics& g,
        juce::Rectangle<float> thumbnailBounds
    );

    float getVisibleRegionLengthSeconds ();

    float xToSeconds (float x);

    float secondsToX (
        float s,
        const juce::Rectangle<float> thumbnailBounds
    );

    void updateDragRegion (float mouseDownSeconds);

    float flattenTime (float timeSeconds);

private:
    AudioFormatManager formatManager;
    AudioFileTransportSource audioSource;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    float visibleRegionStartTime;
    float visibleRegionEndTime;
    bool hasSelectedRegion;
    float selectedRegionStartTime;
    float selectedRegionEndTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformComponent)
};
