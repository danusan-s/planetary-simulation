#include "render_system.h"
#include "model_renderer.h"
#include "resource_manager.h"
#include <glm/ext/matrix_float4x4.hpp>

RenderSystem::RenderSystem() {
  this->modelRenderer = new ModelRenderer();
  this->widgetRenderer = new WidgetRenderer();
}

RenderSystem::~RenderSystem() {
  delete this->modelRenderer;
  delete this->widgetRenderer;
}

void RenderSystem::renderWorld(World *world, float alpha) {
  for (const auto &obj : world->objects) {
    if (obj.spriteID == INVALID_ID || obj.spriteID >= world->sprites.size())
      continue;

    const Sprite &sprite = world->sprites[obj.spriteID];
    const Texture2D &texture = ResourceManager::GetTexture(sprite.textureID);
    const Model &model = ResourceManager::GetModel(sprite.modelID);
    const Shader &shader = ResourceManager::GetShader(sprite.shaderID);
    const glm::vec3 color = sprite.color;

    if (world->sunID == INVALID_ID)
      continue;

    const Object &sun = world->objects[world->sunID];
    const Sprite &sunSprite = world->sprites[sun.spriteID];

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat,
                              static_cast<glm::vec3>(obj.transform.position));
    modelMat = glm::scale(modelMat, glm::vec3(obj.transform.radius));

    this->modelRenderer->renderModel(modelMat, world->camera, model, texture,
                                     shader, color, sun.transform.position,
                                     sunSprite.color);

    const Shader &trailShader = ResourceManager::GetShader("trail");
    trailShader.Use();
    trailShader.SetMatrix4("view", world->camera.GetViewMatrix());
    trailShader.SetMatrix4("projection", world->camera.GetProjectionMatrix());
    trailShader.SetVector3f("trailColor", color * 2.0f);

    glBindVertexArray(obj.trailVAO);
    int start = (obj.trailHead + 1) % MAX_TRAIL;

    if (start == 0) {
      glDrawArrays(GL_LINE_STRIP, 0, MAX_TRAIL);
    } else {
      glDrawArrays(GL_LINE_STRIP, start, MAX_TRAIL - start);
      glDrawArrays(GL_LINE_STRIP, 0, start);
    }
  }
}

void RenderSystem::renderGUI(World *world, Viewport &viewport) {
  glDisable(GL_DEPTH_TEST);
  for (const auto &widget : world->widgets) {
    const Texture2D &texture = ResourceManager::GetTexture("solid");
    const Shader &shader = ResourceManager::GetShader("debug");
    this->widgetRenderer->renderWidget(widget, texture, shader, glm::vec3(1.0f),
                                       viewport.width, viewport.height);
  }
  glEnable(GL_DEPTH_TEST);
}
