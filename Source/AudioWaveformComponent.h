/*
  ==============================================================================

    AudioWaveformComponent.h
    Created: 20 May 2018 3:33:07pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once


#include "../JuceLibraryCode/JuceHeader.h"

#include <string>


class AudioWaveformComponent :    public Component,
                                  public Slider::Listener,
                                  private ChangeListener
{
public:
    enum ColourIds
    {
        waveformColour = 0,
        waveformBackgroundColour = 1,
        waveformSelectedRegionBackgroundColour = 2
    };

    class Listener
    {
    public:
        virtual ~Listener () {}

        virtual void selectedRegionChanged (AudioWaveformComponent*) {}

        virtual void visibleRegionChanged (AudioWaveformComponent*) {}

        virtual void thumbnailCleared (AudioWaveformComponent*) {}
    };
    
    std::function<void (double)> onPositionChange;

public:
    AudioWaveformComponent ();

    ~AudioWaveformComponent ();

    bool loadThumbnail (File file);

    void clearThumbnail ();

    double getTotalLength ();

    double getVisibleRegionStartTime ();

    double getVisibleRegionEndTime ();

    void updateVisibleRegion (double newStartTime, double newEndTime);

    bool getHasSelectedRegion ();

    double getSelectedRegionStartTime ();

    double getSelectedRegionEndTime ();

    void updateSelectedRegion (double newStartTime, double newRegionEndTime);

    void clearSelectedRegion ();

    void addListener (Listener* newListener);

    void removeListener (Listener* listener);

    void paint (Graphics& g) override;

    void mouseWheelMove (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) override;

    void mouseDoubleClick (const MouseEvent& event) override;

    void mouseDrag (const MouseEvent& event) override;

    void mouseDown (const MouseEvent& event) override;

    void sliderValueChanged (Slider* slider) override;

private:
    void changeListenerCallback (ChangeBroadcaster* source) override;

    void paintIfNoFileLoaded (Graphics& g);

    void paintIfFileLoaded (Graphics& g);

    void setSelectedRegionStartTime (double selectedRegionStartTime);

    void setSelectedRegionEndTime (double selectedRegionEndTime);

    void updateSelectedRegion (double mouseDownSeconds);

    double getVisibleRegionLengthInSeconds ();

    bool isVisibleRegionCorrect (
        double visibleRegionStartTime,
        double visibleRegionEndTime
    );

    double xToSeconds (float x);

    float secondsToX (double s);

    double flattenSeconds (double s);

    float flattenX (float x);

private:
    AudioFormatManager formatManager;
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    double visibleRegionStartTime;
    double visibleRegionEndTime;
    bool hasSelectedRegion = false;
    double selectedRegionStartTime;
    double selectedRegionEndTime;
    ListenerList<Listener> listeners;
    OpenGLContext openGLContext;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformComponent)
};
