/*
  ==============================================================================

    AudioWaveformComponent.h
    Created: 20 May 2018 3:33:07pm
    Author:  DBANKOV

  ==============================================================================
*/


#pragma once

#include <string>

#include "../JuceLibraryCode/JuceHeader.h"

#include "TenFtUtil.h"
#include "AudioWaveformOpenGLComponent.h"


class AudioWaveformComponent :    public Component,
                                  public OpenGLRenderer,
                                  public Slider::Listener
{
public:
    class Listener
    {
    public:
        virtual ~Listener () {}

        virtual void selectedRegionChanged (AudioWaveformComponent*) {}

        virtual void selectedRegionCreated (AudioWaveformComponent*) {}

        virtual void selectedRegionCleared (AudioWaveformComponent*) {}

        virtual void visibleRegionChanged (AudioWaveformComponent*) {}

        virtual void thumbnailCleared (AudioWaveformComponent*) {}
    };
    
    std::function<void (double)> onPositionChange;

public:
    AudioWaveformComponent ();

    ~AudioWaveformComponent ();

    void newOpenGLContextCreated () override;

    void openGLContextClosing () override;

    void renderOpenGL () override;

    void paint (Graphics& g) override;

    void resized () override;

    void mouseWheelMove (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) override;

    void mouseDoubleClick (const MouseEvent& event) override;

    void mouseDrag (const MouseEvent& event) override;

    void mouseDown (const MouseEvent& event) override;

    void mouseUp (const MouseEvent& event) override;

    void sliderValueChanged (Slider* slider) override;

    void addListener (Listener* newListener);

    void removeListener (Listener* listener);

    void loadWaveform (
        AudioSampleBuffer* newAudioBuffer,
        double newSampleRate,
        const CriticalSection* bufferUpdateLock = nullptr
    );

    void clearWaveform ();

    double getTotalLength ();

    double getVisibleRegionStartTime ();

    double getVisibleRegionEndTime ();

    void updateVisibleRegion (double newStartTime, double newEndTime);

    bool getHasSelectedRegion ();

    double getSelectedRegionStartTime ();

    double getSelectedRegionEndTime ();

    void updateSelectedRegion (double newStartTime, double newRegionEndTime);

    void clearSelectedRegion ();

    void refresh ();

private:
    void paintIfNoFileLoaded (Graphics& g);

    void setSelectedRegionStartTime (double selectedRegionStartTime);

    void setSelectedRegionEndTime (double selectedRegionEndTime);

    void updateSelectedRegion (double mouseDownSeconds);

    double getVisibleRegionLengthInSeconds ();

    bool isVisibleRegionCorrect (
        double visibleRegionStartTime,
        double visibleRegionEndTime
    );

    unsigned int getSamplesDiff (double startTime, double endTime);

private:
    OpenGLContext openGLContext;
    AudioSampleBuffer* audioBuffer = nullptr;
    double sampleRate = 0.0;
    AudioWaveformOpenGLComponent waveform;
    double visibleRegionStartTime = 0.0;
    double visibleRegionEndTime = 0.0;
    bool hasSelectedRegion = false;
    double selectedRegionStartTime = 0.0;
    double selectedRegionEndTime = 0.0;
    ListenerList<Listener> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformComponent)
};
