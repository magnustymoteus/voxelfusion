#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in vec3 aNormal;

uniform mat4 cameraMatrix;
out vec4 color;
out vec3 normal;
out vec3 currentPos;

void main()
{
   currentPos = aPos;
   gl_Position = cameraMatrix * vec4(currentPos, 1.0);
   color = aColor;
   normal = aNormal;
}