#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 cameraMatrix;
out vec3 pos;

void main()
{
   gl_Position = cameraMatrix * vec4(aPos, 1.0);
   pos = aPos;
}