#version 330 core

// Vertex attributes
layout(location = 0) in vec3 aPos;    // x, y, z
layout(location = 1) in vec2 aUV;     // u, v

out vec2 TexCoords;

void main()
{
    gl_Position = vec4(aPos, 1.0f);
    TexCoords = aUV;
}

