/*
  ==============================================================================

    Attributes.h
    Created: 8 Jul 2018 11:00:01am
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "VertexBuffer.h"


class Attributes
{
public:
    Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram);

    ~Attributes ();

    void enable ();

    void disable ();

private:
    OpenGLContext& openGLContext;
    std::unique_ptr<OpenGLShaderProgram::Attribute> position;

};