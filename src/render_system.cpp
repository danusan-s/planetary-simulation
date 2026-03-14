#include "render_system.h"
#include "model_renderer.h"
#include "resource_manager.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>

RenderSystem::RenderSystem()
    : modelRenderer(std::make_unique<ModelRenderer>()),
      skyboxRenderer(std::make_unique<SkyboxRenderer>()),
      particleRenderer(std::make_unique<ParticleRenderer>()) {}

RenderSystem::~RenderSystem() = default;

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

  this->particleRenderer->render(this->viewProj, world->camera,
                                 world->particles,
                                 static_cast<glm::vec3>(lightPos),
                                 static_cast<glm::vec3>(lightColor));
}
