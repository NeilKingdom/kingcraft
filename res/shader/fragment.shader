#version 330 core

in vec2 texCoord;
out vec4 FragColor;

uniform sampler2D myTexture;

void main()
{
    FragColor = texture2D(myTexture, texCoord);
}
