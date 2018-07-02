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
#include "VertexBuffer.h"


class AudioWaveformComponent :    public OpenGLAppComponent,
                                  public Slider::Listener
{
public:
    enum ColourIds
    {
        waveformColour = 0,
        waveformBackgroundColour = 1
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

    void initialise () override;

    void shutdown () override;

    void render () override;

    void paint (Graphics& g) override;

    void mouseWheelMove (
        const MouseEvent& event,
        const MouseWheelDetails& wheelDetails
    ) override;

    void mouseDoubleClick (const MouseEvent& event) override;

    void mouseDrag (const MouseEvent& event) override;

    void mouseDown (const MouseEvent& event) override;

    void sliderValueChanged (Slider* slider) override;

    void addListener (Listener* newListener);

    void removeListener (Listener* listener);

    bool loadWaveform (AudioFormatReader* reader);

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

    void fillVertices ();

private:
    AudioFormatReader* reader = nullptr;
    AudioBuffer<float> readerBuffer;
    double visibleRegionStartTime;
    double visibleRegionEndTime;
    bool hasSelectedRegion = false;
    double selectedRegionStartTime;
    double selectedRegionEndTime;
    ListenerList<Listener> listeners;

    std::unique_ptr<OpenGLShaderProgram> shaderProgram;
    std::unique_ptr<VertexBuffer> vertexBuffer;
    Array<Vertex> vertices;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioWaveformComponent)
};
