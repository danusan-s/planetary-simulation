#version 330 core

in vec2 TexCoords;
uniform sampler2D Tex;
uniform vec3 objectColor;

out vec4 FragColor;

void main()
{
    // FragColor = texture(Tex, TexCoords);
    FragColor = vec4(objectColor, 1.0);
}

