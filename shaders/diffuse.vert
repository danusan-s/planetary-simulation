#version 330 core

// Vertex attributes
layout(location = 0) in vec3 aPos;    // x, y, z
layout(location = 1) in vec3 aNormal; // nx, ny, nz
layout(location = 2) in vec2 aUV;     // u, v

// Uniforms
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    TexCoords = aUV;
}

