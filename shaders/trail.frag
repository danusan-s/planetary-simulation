#version 330 core

uniform vec3 trailColor;

out vec4 FragColor;

void main()
{
    FragColor = vec4(trailColor, 1.0);
}

