#include "render_system.h"
#include "model_renderer.h"
#include "resource_manager.h"
#include <glm/ext/matrix_float4x4.hpp>

RenderSystem::RenderSystem() {
  this->renderer = new ModelRenderer();
}

RenderSystem::~RenderSystem() {
  delete this->renderer;
}

void RenderSystem::renderWorld(World &world, float alpha) {
  for (const auto &obj : world.objects) {
    if (obj.spriteID == INVALID_ID || obj.spriteID >= world.sprites.size())
      continue;

    const Sprite &sprite = world.sprites[obj.spriteID];
    const Texture2D &texture = ResourceManager::GetTexture(sprite.textureID);
    const Model &model = ResourceManager::GetModel(sprite.modelID);
    const Shader &shader = ResourceManager::GetShader(sprite.shaderID);
    Camera *camera = world.camera;

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat,
                              static_cast<glm::vec3>(obj.transform.position));
    modelMat = glm::scale(modelMat, glm::vec3(obj.transform.radius));

    this->renderer->renderModel(modelMat, camera, model, texture, shader);
  }
}
