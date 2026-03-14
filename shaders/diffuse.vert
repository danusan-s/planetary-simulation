#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

// GPUBody layout must match the C++ struct exactly (std430, 48 bytes)
struct GPUBody {
    vec3  position;
    float mass;
    vec3  velocity;
    float radius;
    vec3  scale;
    float alive;
};

layout(std430, binding = 0) readonly buffer BodyBuffer {
    GPUBody bodies[];
};

uniform mat4 viewProj;
uniform int  objectIndex; // -1 means use the legacy "model" uniform (particles)
uniform mat4 model;       // used only when objectIndex < 0

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

void main()
{
    mat4 modelMat;

    if (objectIndex >= 0) {
        GPUBody body = bodies[objectIndex];

        // Build model matrix: scale then translate (no rotation for spheres)
        modelMat = mat4(1.0);
        modelMat[0][0] = body.scale.x;
        modelMat[1][1] = body.scale.y;
        modelMat[2][2] = body.scale.z;
        modelMat[3]    = vec4(body.position, 1.0);
    } else {
        // Particle path: use the pre-built matrix from the CPU
        modelMat = model;
    }

    FragPos    = vec3(modelMat * vec4(aPos, 1.0));
    Normal     = mat3(transpose(inverse(modelMat))) * aNormal;
    TexCoords  = aUV;

    gl_Position = viewProj * modelMat * vec4(aPos, 1.0);
}
