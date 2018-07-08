/*
  ==============================================================================

    Attributes.cpp
    Created: 8 Jul 2018 11:00:01am
    Author:  DBANKOV

  ==============================================================================
*/

#include "Attributes.h"


Attributes::Attributes (OpenGLContext& openGLContext, OpenGLShaderProgram& shaderProgram) :
    openGLContext (openGLContext)
{
    position.reset (new OpenGLShaderProgram::Attribute (shaderProgram, "position"));
}

Attributes::~Attributes ()
{
    position.reset ();
}

void Attributes::enable ()
{
    openGLContext.extensions.glVertexAttribPointer (position->attributeID, 2,
        GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
    openGLContext.extensions.glEnableVertexAttribArray (position->attributeID);
}

void Attributes::disable ()
{
    openGLContext.extensions.glDisableVertexAttribArray (position->attributeID);
}