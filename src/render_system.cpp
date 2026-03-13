#include "render_system.h"
#include "model_renderer.h"
#include "resource_manager.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

// Speed (units/s) at which particle stretch saturates at its maximum (2x).
static constexpr float PARTICLE_STRETCH_REF_SPEED = 10.0f;

RenderSystem::RenderSystem()
    : modelRenderer(std::make_unique<ModelRenderer>()),
      skyboxRenderer(std::make_unique<SkyboxRenderer>()) {
}

RenderSystem::~RenderSystem() = default;

// Stretches baseScale along the velocity direction.
// stretch = 1 + clamp(speed/refSpeed, 0, 1)  -> range [1, 2]
// Perpendicular axes shrink by 1/sqrt(stretch) to conserve volume.
// Returns a combined rotation+scale matrix ready to be appended after
// translate.
glm::mat4 RenderSystem::velocityStretchMatrix(glm::vec3 baseScale,
                                              Vec3 velocity, float refSpeed) {
  glm::vec3 vel(velocity.x, velocity.y, velocity.z);
  float speed = glm::length(vel);

  if (speed < 1e-4f)
    return glm::scale(glm::mat4(1.0f), baseScale);

  float t = glm::clamp(speed / refSpeed, 0.0f, 1.0f);
  float stretch = 1.0f + t;                 // [1, 2]
  float squash = 1.0f / std::sqrt(stretch); // volume-preserving

  glm::vec3 stretchedScale = baseScale * glm::vec3(squash, squash, stretch);

  // Rotate so the model's +Z axis aligns with the velocity direction
  glm::vec3 dir = glm::normalize(vel);
  glm::vec3 refDir = glm::vec3(0.0f, 0.0f, 1.0f);
  glm::quat rot;
  if (glm::abs(glm::dot(dir, refDir)) > 0.9999f) {
    // Nearly parallel or anti-parallel — use a safe fallback rotation
    rot = glm::rotation(refDir, dir);
  } else {
    rot = glm::rotation(refDir, dir);
  }

  return glm::toMat4(rot) * glm::scale(glm::mat4(1.0f), stretchedScale);
}

void RenderSystem::updateViewProjection(World *world) {
  this->viewProj =
      world->camera.GetProjectionMatrix() * world->camera.GetViewMatrix();
}

void RenderSystem::renderWorld(World *world) {
  updateViewProjection(world);
  renderSkybox(world);
  renderObjects(world);
  renderParticles(world);
}

void RenderSystem::renderSkybox(World *world) {
  const Cubemap &skybox = ResourceManager::GetCubemap("space");
  const Shader &skyboxShader = ResourceManager::GetShader("skybox");
  this->skyboxRenderer->render(world->camera, skybox, skyboxShader);
}

void RenderSystem::getSunLight(World *world, Vec3 &outPos, Vec3 &outColor) {
  outPos = Vec3(100.0f, 100.0f, 100.0f);
  outColor = Vec3(1.0f);
  if (world->sunID != INVALID_ID) {
    const Object &sun = world->objects[world->sunID];
    const Sprite &sunSprite = world->sprites[sun.spriteID];
    outPos = sun.transform.position;
    outColor = sunSprite.color;
  }
}

void RenderSystem::renderObjects(World *world) {
  Vec3 lightPos, lightColor;
  getSunLight(world, lightPos, lightColor);

  for (size_t i = 0; i < world->objects.size(); i++) {
    const auto &obj = world->objects[i];
    if (!obj.active || obj.spriteID == INVALID_ID ||
        obj.spriteID >= world->sprites.size())
      continue;

    const Sprite &sprite = world->sprites[obj.spriteID];
    const Texture2D &texture = ResourceManager::GetTexture(sprite.textureID);
    const Model &model = ResourceManager::GetModel(sprite.modelID);
    const Shader &shader = ResourceManager::GetShader(sprite.shaderID);
    const glm::vec3 color = sprite.color;

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat,
                              static_cast<glm::vec3>(obj.transform.position));
    modelMat =
        glm::scale(modelMat, static_cast<glm::vec3>(obj.transform.scale));

    shader.SetMatrix4("viewProj", this->viewProj, true);

    this->modelRenderer->renderModel(modelMat, world->camera, model, texture,
                                     shader, color, lightPos, lightColor);

    const Shader &trailShader = ResourceManager::GetShader("trail");
    trailShader.SetMatrix4("viewProj", this->viewProj, true);
    glBindBuffer(GL_ARRAY_BUFFER, obj.trailVBO);
    glBufferData(GL_ARRAY_BUFFER, MAX_TRAIL * sizeof(Vec3), obj.trail,
                 GL_DYNAMIC_DRAW);
    this->modelRenderer->renderTrail(trailShader, obj.trailHead, obj.trailVAO,
                                     color);
  }
}

void RenderSystem::renderParticles(World *world) {
  Vec3 lightPos, lightColor;
  getSunLight(world, lightPos, lightColor);

  for (const auto &particle : world->particles) {
    if (!particle.active || particle.spriteID == INVALID_ID ||
        particle.spriteID >= world->sprites.size())
      continue;

    const Sprite &sprite = world->sprites[particle.spriteID];
    const Texture2D &texture = ResourceManager::GetTexture(sprite.textureID);
    const Model &model = ResourceManager::GetModel(sprite.modelID);
    const Shader &particleShader = ResourceManager::GetShader(sprite.shaderID);
    const glm::vec3 color = sprite.color;

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(
        modelMat, static_cast<glm::vec3>(particle.transform.position));
    modelMat *=
        velocityStretchMatrix(static_cast<glm::vec3>(particle.transform.scale),
                              particle.velocity, PARTICLE_STRETCH_REF_SPEED);

    particleShader.SetMatrix4("viewProj", this->viewProj, true);

    this->modelRenderer->renderModel(modelMat, world->camera, model, texture,
                                     particleShader, color, lightPos,
                                     lightColor);
  }
}
