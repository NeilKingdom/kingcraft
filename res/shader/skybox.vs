#version 330 core

layout (location = 0) in vec3 a_position;

out vec3 tex_coords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
    tex_coords = a_position;
    vec4 pos = proj * view * model * vec4(a_position, 1.0);
    gl_Position = pos.xyww;
}
