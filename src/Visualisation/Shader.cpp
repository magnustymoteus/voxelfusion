#include "Shader.h"
using namespace std;


void Shader::readGLSL(const char* path, string& output) {
    ifstream t(path);
    stringstream buffer;
    buffer << t.rdbuf();
    output = buffer.str();
}

Shader::Shader(const char* vertexFile, const char* fragmentFile) {
	string vertexCode;
	string fragmentCode;
    readGLSL(vertexFile, vertexCode);
    readGLSL(fragmentFile, fragmentCode);
    const char* vertexSource = vertexCode.c_str();
    const char* fragmentSource = fragmentCode.c_str();

	//compile vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);


	glShaderSource(vertexShader, 1, &vertexSource, NULL);
	glCompileShader(vertexShader);

	//compile fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
	glCompileShader(fragmentShader);

	// something to throw all shaders together in
	ID = glCreateProgram();

	// attach and link them
	glAttachShader(ID, vertexShader);
	glAttachShader(ID, fragmentShader);
	glLinkProgram(ID);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

}

void Shader::Activate()
{
	glUseProgram(ID);
}

void Shader::Delete()
{
	glDeleteProgram(ID);
}