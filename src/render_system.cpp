#include "render_system.h"
#include "model_renderer.h"
#include "resource_manager.h"
#include <glm/ext/matrix_float4x4.hpp>

RenderSystem::RenderSystem() {
  this->renderer = new ModelRenderer();
  setupDebugData();
}

RenderSystem::~RenderSystem() {
  delete this->renderer;
}

void RenderSystem::setupDebugData() {
  // Debug render
  float vertices[] = {-0.5f, -0.5f, -0.5f, 0.0f,  0.0f,  0.5f, -0.5f, -0.5f,
                      1.0f,  0.0f,  0.5f,  0.5f,  -0.5f, 1.0f, 1.0f,  0.5f,
                      0.5f,  -0.5f, 1.0f,  1.0f,  -0.5f, 0.5f, -0.5f, 0.0f,
                      1.0f,  -0.5f, -0.5f, -0.5f, 0.0f,  0.0f};

  GLuint VBO;

  glGenVertexArrays(1, &debugVAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(debugVAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Position (location = 0)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // UV (location = 1)
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
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

    glm::mat4 modelMat = glm::mat4(1.0f);
    modelMat = glm::translate(modelMat,
                              static_cast<glm::vec3>(obj.transform.position));
    modelMat = glm::scale(modelMat, glm::vec3(obj.transform.radius));

    this->renderer->renderModel(modelMat, world->camera, model, texture, shader,
                                color);

    const Shader &trailShader = ResourceManager::GetShader("trail");
    trailShader.Use();
    trailShader.SetMatrix4("view", world->camera.GetViewMatrix());
    trailShader.SetMatrix4("projection", world->camera.GetProjectionMatrix());
    trailShader.SetVector3f("trailColor", color * 0.5f);

    glBindVertexArray(obj.trailVAO);
    int start = (obj.trailHead + 1) % MAX_TRAIL;

    if (start == 0) {
      glDrawArrays(GL_LINE_STRIP, 0, MAX_TRAIL);
    } else {
      glDrawArrays(GL_LINE_STRIP, start, MAX_TRAIL - start);
      glDrawArrays(GL_LINE_STRIP, 0, start);
    }
  }

  // Debug render
  // ResourceManager::GetShader("debug").Use();
  // glBindVertexArray(debugVAO);
  // glDrawArrays(GL_TRIANGLES, 0, 6);
  // glBindVertexArray(0);
}
