#include "particle_renderer.h"
#include "resource_manager.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// Speed (units/s) at which particle stretch saturates at its maximum (2x).
static constexpr float PARTICLE_STRETCH_REF_SPEED = 10.0f;

ParticleRenderer::ParticleRenderer() {
  glGenBuffers(1, &instanceVBO);
}

ParticleRenderer::~ParticleRenderer() {
  if (instanceVBO)
    glDeleteBuffers(1, &instanceVBO);
  if (particleVAO)
    glDeleteVertexArrays(1, &particleVAO);
}

void ParticleRenderer::setupVAO(const Model &model) {
  glGenVertexArrays(1, &particleVAO);
  glBindVertexArray(particleVAO);

  // --- Bind the mesh geometry (positions, normals, UVs) ---
  glBindBuffer(GL_ARRAY_BUFFER, model.VBO);

  // location 0: vec3 position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);

  // location 1: vec3 normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(3 * sizeof(float)));

  // location 2: vec2 uv
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float),
                        (void *)(6 * sizeof(float)));

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBO);

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

  const size_t stride = sizeof(ParticleInstanceData); // 80 bytes

  // locations 3-6: mat4 modelMatrix (one vec4 column per location)
  for (int col = 0; col < 4; ++col) {
    GLuint loc = 3 + col;
    glEnableVertexAttribArray(loc);
    glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, stride,
                          (void *)(col * sizeof(glm::vec4)));
    glVertexAttribDivisor(loc, 1); // advance once per instance
  }

  // location 7: vec3 color
  glEnableVertexAttribArray(7);
  glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, stride,
                        (void *)(sizeof(glm::mat4))); // offset = 64 bytes
  glVertexAttribDivisor(7, 1);

  glBindVertexArray(0);
  vaoInitialized = true;
}

glm::mat4 ParticleRenderer::velocityStretchMatrix(glm::vec3 baseScale,
                                                  Vec3 velocity,
                                                  float refSpeed) {
  glm::vec3 vel(velocity.x, velocity.y, velocity.z);
  float speed = glm::length(vel);

  if (speed < 1e-4f)
    return glm::scale(glm::mat4(1.0f), baseScale);

  float t = glm::clamp(speed / refSpeed, 0.0f, 1.0f);
  float stretch = 1.0f + t;
  float squash = 1.0f / std::sqrt(stretch);

  glm::vec3 stretchedScale = baseScale * glm::vec3(squash, squash, stretch);

  glm::vec3 dir = glm::normalize(vel);
  glm::vec3 refDir = glm::vec3(0.0f, 0.0f, 1.0f);
  glm::quat rot = glm::rotation(refDir, dir);

  return glm::toMat4(rot) * glm::scale(glm::mat4(1.0f), stretchedScale);
}

void ParticleRenderer::render(const glm::mat4 &viewProj, Camera &camera,
                               const std::vector<Particle> &particles,
                               const glm::vec3 &lightPos,
                               const glm::vec3 &lightColor) {

  // --- 1. Build per-instance data on the CPU ---
  instanceData.clear();

  for (const auto &particle : particles) {
    if (!particle.active)
      continue;

    glm::mat4 modelMat = glm::translate(
        glm::mat4(1.0f), static_cast<glm::vec3>(particle.transform.position));
    modelMat *=
        velocityStretchMatrix(static_cast<glm::vec3>(particle.transform.scale),
                              particle.velocity, PARTICLE_STRETCH_REF_SPEED);

    ParticleInstanceData inst;
    inst.modelMatrix = modelMat;
    inst.color = static_cast<glm::vec3>(particle.color);
    inst._pad = 0.0f;
    instanceData.push_back(inst);
  }

  if (instanceData.empty())
    return;

  // --- 2. Lazy-init the VAO on first use ---
  const Model &model = ResourceManager::GetModel("debris");
  if (!vaoInitialized)
    setupVAO(model);

  // --- 3. Upload instance data to GPU ---
  const size_t requiredBytes =
      instanceData.size() * sizeof(ParticleInstanceData);

  glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
  if (instanceData.size() > instanceBufferCapacity) {
    // (Re-)allocate with some headroom so we don't reallocate every frame.
    instanceBufferCapacity = instanceData.size() * 2;
    glBufferData(GL_ARRAY_BUFFER,
                 instanceBufferCapacity * sizeof(ParticleInstanceData), nullptr,
                 GL_DYNAMIC_DRAW);
  }
  // Sub-upload only the used portion.
  glBufferSubData(GL_ARRAY_BUFFER, 0, requiredBytes, instanceData.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // --- 4. Set shared uniforms & bind state, then issue ONE draw call ---
  const Shader &shader = ResourceManager::GetShader("particle");
  const Texture2D &texture = ResourceManager::GetTexture("solid");

  shader.Use();
  shader.SetMatrix4("viewProj", viewProj);
  shader.SetVector3f("viewPos", camera.position);
  shader.SetVector3f("lightPos", lightPos);
  shader.SetVector3f("lightColor", lightColor);

  glActiveTexture(GL_TEXTURE0);
  texture.Bind();

  glBindVertexArray(particleVAO);
  glDrawElementsInstanced(
      GL_TRIANGLES, static_cast<GLsizei>(model.indices.size()), GL_UNSIGNED_INT,
      0, static_cast<GLsizei>(instanceData.size()));
  glBindVertexArray(0);
}
