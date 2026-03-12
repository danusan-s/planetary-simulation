#include "render_system.h"
#include "model_renderer.h"
#include "resource_manager.h"
#include <glm/ext/matrix_float4x4.hpp>

RenderSystem::RenderSystem() {
  this->modelRenderer = new ModelRenderer();
  this->skyboxRenderer = new SkyboxRenderer();
}

RenderSystem::~RenderSystem() {
  delete this->modelRenderer;
  delete this->skyboxRenderer;
}

void RenderSystem::renderWorld(World *world, float alpha) {
  renderSkybox(world);
  renderObjects(world, alpha);
}

void RenderSystem::renderSkybox(World *world) {
  const Cubemap &skybox = ResourceManager::GetCubemap("space");
  const Shader &skyboxShader = ResourceManager::GetShader("skybox");
  this->skyboxRenderer->render(world->camera, skybox, skyboxShader);
}

void RenderSystem::renderObjects(World *world, float alpha) {
  for (const auto &obj : world->objects) {
    if (obj.spriteID == INVALID_ID || obj.spriteID >= world->sprites.size())
      continue;

    const Sprite &sprite = world->sprites[obj.spriteID];
    const Texture2D &texture = ResourceManager::GetTexture(sprite.textureID);
    const Model &model = ResourceManager::GetModel(sprite.modelID);
    const Shader &shader = ResourceManager::GetShader(sprite.shaderID);
    const glm::vec3 color = sprite.color;

    Vec3 lightPos(0.0f);
    Vec3 lightColor(1.0f);
    if (world->sunID != INVALID_ID) {
      const Object &sun = world->objects[world->sunID];
      const Sprite &sunSprite = world->sprites[sun.spriteID];
      lightPos = sun.transform.position;
      lightColor = sunSprite.color;
    }

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat,
                              static_cast<glm::vec3>(obj.transform.position));
    modelMat = glm::scale(modelMat, glm::vec3(obj.transform.radius));

    this->modelRenderer->renderModel(modelMat, world->camera, model, texture,
                                     shader, color, lightPos, lightColor);

    const Shader &trailShader = ResourceManager::GetShader("trail");
    this->modelRenderer->renderTrail(world->camera, trailShader, obj.trailHead,
                                     obj.trailVAO, color);
  }
}
