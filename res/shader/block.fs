#version 330 core

in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D myTexture;
uniform samplerCube skyBox;

void main()
{
    FragColor = texture(myTexture, texCoords);
}
