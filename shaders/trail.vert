#version 330 core

// Vertex attributes
layout(location = 0) in vec3 aPos;    // x, y, z

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0f);
}

