#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

out vec3 myColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform mat4 u_MVP;

void main()
{
   myColor = aColor;
   gl_Position = proj * view * model * vec4(aPos, 1.0);
}
