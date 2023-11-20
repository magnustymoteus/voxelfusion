#pragma once

#include <glad/glad.h>
class ElementBuffer
{

public:
	GLuint ID;
	ElementBuffer(GLuint* indices, GLsizeiptr size);

	void Bind();
	void Unbind();
    void redefine(GLuint* indices, GLsizeiptr size);
	void Delete();
};

