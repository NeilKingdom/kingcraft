#version 330 core

in vec2 a_tex_coords;
out vec4 frag_color;

uniform sampler2D texels;

void main()
{
    frag_color = texture(texels, a_tex_coords);
}
