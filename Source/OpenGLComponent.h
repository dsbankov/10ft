/*
  ==============================================================================

    OpenGLComponent.h
    Created: 5 Jul 2018 8:29:47pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class OpenGLComponent : public Component
{
public:
    OpenGLComponent () { }

    virtual ~OpenGLComponent () { }

    void paint (Graphics& g) override { }

    virtual void initialise (OpenGLContext& openGLContext) = 0;

    virtual void shutdown (OpenGLContext& openGLContext) = 0;

    virtual void render (OpenGLContext& openGLContext) = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLComponent)
};