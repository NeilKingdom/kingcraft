#version 330 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_tex_coords;

out vec2 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    tex_coords = a_tex_coords;
    gl_Position = proj * view * model * vec4(a_position, 1.0);
}
