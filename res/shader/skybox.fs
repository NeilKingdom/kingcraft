#version 330 core

in vec3 a_tex_coords;
out vec4 frag_color;

uniform samplerCube texels;

void main()
{
    frag_color = texture(texels, a_tex_coords);
}
