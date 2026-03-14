#version 430 core

// Trail positions live in the SSBO — no vertex attribute needed.
// slot = objectIndex * MAX_TRAIL + gl_VertexID
layout(std430, binding = 3) readonly buffer TrailBuffer {
    vec4 trailData[];
};

uniform mat4 viewProj;
uniform int  objectIndex;
uniform int  trailHead;
uniform int  maxTrail;

out float vAlpha;

void main()
{
    int ringIdx = (trailHead + 1 + gl_VertexID) % maxTrail;
    int slot    = objectIndex * maxTrail + ringIdx;
    vec3 pos    = trailData[slot].xyz;

    gl_Position = viewProj * vec4(pos, 1.0);

    // vertex 0 = oldest (transparent), vertex (maxTrail-1) = newest (opaque)
    vAlpha = float(gl_VertexID) / float(maxTrail - 1);
}
