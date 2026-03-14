#ifndef PARTICLE_RENDERER_H
#define PARTICLE_RENDERER_H

#include "camera.h"
#include "glad/glad.h"
#include "model.h"
#include "shader.h"
#include "texture.h"
#include "types.h"
#include <glm/glm.hpp>
#include <vector>

// Per-instance data uploaded to the GPU for instanced particle rendering.
struct ParticleInstanceData {
  glm::mat4 modelMatrix; // 64 bytes
  glm::vec3 color;       // 12 bytes
  float _pad;            // 4 bytes padding (total 80 bytes per instance)
};

class ParticleRenderer {
public:
  ParticleRenderer();
  ~ParticleRenderer();

  // Renders all active particles in a single instanced draw call.
  void render(const glm::mat4 &viewProj, Camera &camera,
              const std::vector<Particle> &particles,
              const glm::vec3 &lightPos, const glm::vec3 &lightColor);

private:
  GLuint instanceVBO = 0; // VBO holding per-instance data
  GLuint particleVAO = 0; // VAO combining mesh + instance attributes
  size_t instanceBufferCapacity = 0;

  std::vector<ParticleInstanceData> instanceData; // CPU staging buffer

  bool vaoInitialized = false;

  void setupVAO(const Model &model);

  // Compute velocity-stretch model matrix for a particle.
  static glm::mat4 velocityStretchMatrix(glm::vec3 baseScale, Vec3 velocity,
                                         float refSpeed);
};

#endif // !PARTICLE_RENDERER_H
