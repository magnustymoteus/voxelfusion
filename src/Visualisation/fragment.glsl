#version 330 core
out vec4 FragColor;
in vec4 color;
in vec3 normal;
in vec3 currentPos;
uniform vec4 sunColor;
uniform vec3 sunPosition;

void main()
{
   float ambient = 0.2;
   vec3 lightDir = normalize(sunPosition - currentPos);
   float diffuse = max(dot(normal, lightDir), 0.0f);
   FragColor = color * sunColor * min((diffuse + ambient), 1.0f);
}