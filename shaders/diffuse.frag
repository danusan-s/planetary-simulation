#version 330 core

in vec2 TexCoords;
uniform sampler2D Tex;

out vec4 FragColor;

void main()
{
    // FragColor = texture(Tex, TexCoords);
    FragColor = vec4(1.0, 1.0, 1.0, 1.0);
}

