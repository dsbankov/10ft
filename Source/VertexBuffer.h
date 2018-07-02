/*
  ==============================================================================

    VertexBuffer.h
    Created: 2 Jul 2018 5:49:54pm
    Author:  DBANKOV

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct Vertex
{
    GLfloat x;
    GLfloat y;
};

class VertexBuffer
{
public:
    VertexBuffer (OpenGLContext& openGLContext);

    ~VertexBuffer ();

    void bind (Array<Vertex>& buffer);

    void unbind ();

private:
    OpenGLContext& openGLContext;
    GLuint id;

};