#pragma once

#include <glad/glad.h>
#include "VertexBuffer.h"
class VertexArray
{
public:
	GLuint ID;
	VertexArray();

	void LinkVertexBuffer(VertexBuffer& VertexBuffer, GLuint layout, GLuint numComponents, GLenum type, GLsizeiptr stride, void* offset);
	void Bind();
	void Unbind();
	void Delete();
};

