/*
  ==============================================================================

    VertexBuffer.cpp
    Created: 2 Jul 2018 5:49:54pm
    Author:  DBANKOV

  ==============================================================================
*/

#include "VertexBuffer.h"

VertexBuffer::VertexBuffer (OpenGLContext& openGLContext) :
    openGLContext (openGLContext)
{
    openGLContext.extensions.glGenBuffers (1, &id);
};

VertexBuffer::~VertexBuffer ()
{
    openGLContext.extensions.glDeleteBuffers (1, &id);
}

void VertexBuffer::bind (Array<Vertex>& buffer)
{
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, id);
    openGLContext.extensions.glBufferData (GL_ARRAY_BUFFER,
        static_cast<GLsizeiptr> (static_cast<size_t> (buffer.size ()) * sizeof (Vertex)),
        buffer.getRawDataPointer (), GL_STATIC_DRAW);

    /*openGLContext.extensions.glVertexAttribPointer (positionAttribute->attributeID, 2,
        GL_FLOAT, GL_FALSE, sizeof (Vertex), 0);
    openGLContext.extensions.glEnableVertexAttribArray (positionAttribute->attributeID);*/
}

void VertexBuffer::unbind ()
{
    openGLContext.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
}