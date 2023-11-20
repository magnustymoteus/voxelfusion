#pragma once

#include <glad/glad.h>
#include "VertexBuffer.h"
class VertexArray
{
public:
	GLuint ID;
	VertexArray();

	void LinkVertexBuffer(VertexBuffer& VertexBuffer, GLuint layout);
	void Bind();
	void Unbind();
	void Delete();
};

