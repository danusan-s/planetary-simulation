#include "render_system.h"
#include "model_renderer.h"
#include "resource_manager.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

// Temporary debug helper — drains all GL errors and prints them with a tag.
static void checkGL(const char *tag) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "[GL ERROR " << err << "] at " << tag << std::endl;
  }
}

RenderSystem::RenderSystem()
    : modelRenderer(std::make_unique<ModelRenderer>()),
      skyboxRenderer(std::make_unique<SkyboxRenderer>()),
      particleRenderer(std::make_unique<ParticleRenderer>()) {
  glGenVertexArrays(1, &emptyVAO);
}

RenderSystem::~RenderSystem() {
  if (emptyVAO)
    glDeleteVertexArrays(1, &emptyVAO);
}

void RenderSystem::updateViewProjection(World *world) {
  this->viewProj =
      world->camera.GetProjectionMatrix() * world->camera.GetViewMatrix();
}

void RenderSystem::renderWorld(World *world) {
  updateViewProjection(world);
  checkGL("renderWorld:beforeSkybox");
  renderSkybox(world);
  checkGL("renderWorld:afterSkybox");
  renderObjects(world);
  checkGL("renderWorld:afterObjects");
  renderParticles(world);
  checkGL("renderWorld:afterParticles");
}

void RenderSystem::renderSkybox(World *world) {
  const Cubemap &skybox = ResourceManager::GetCubemap("space");
  const Shader &skyboxShader = ResourceManager::GetShader("skybox");
  this->skyboxRenderer->render(world->camera, skybox, skyboxShader);
}

void RenderSystem::getSunLight(World *world, Vec3 &outPos, Vec3 &outColor) {
  outPos = Vec3(1000.0f, 1000.0f, 1000.0f);
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

  int objectCount = static_cast<int>(world->objects.size());

  checkGL("renderObjects:enter");

  // Bind body SSBO to binding 0 so diffuse.vert can read positions/scales.
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, world->SSBOobjA);
  // Bind trail SSBO to binding 3 so trail.vert can read trail positions.
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, world->SSBOtrail);

  for (int i = 0; i < objectCount; i++) {
    const Object &obj = world->objects[i];
    if (!obj.active || obj.spriteID == INVALID_ID ||
        obj.spriteID >= world->sprites.size())
      continue;

    const Sprite &sprite = world->sprites[obj.spriteID];
    const Texture2D &texture = ResourceManager::GetTexture(sprite.textureID);
    const Model &model = ResourceManager::GetModel(sprite.modelID);
    const Shader &shader = ResourceManager::GetShader(sprite.shaderID);
    const glm::vec3 color = sprite.color;

    shader.SetMatrix4("viewProj", this->viewProj, true);
    shader.SetInteger("objectIndex", i);

    glm::mat4 identity(1.0f);
    this->modelRenderer->renderModel(identity, world->camera, model, texture,
                                     shader, color, lightPos, lightColor);
    checkGL("renderObjects:model");

    // --- Trail (attribute-less draw: positions come from SSBOtrail) ---
    const Shader &trailShader = ResourceManager::GetShader("trail");
    trailShader.Use();
    trailShader.SetMatrix4("viewProj", this->viewProj);
    trailShader.SetInteger("objectIndex", i);
    trailShader.SetInteger("trailHead", world->trailHeads[i]);
    trailShader.SetInteger("maxTrail", MAX_TRAIL);
    trailShader.SetVector3f("trailColor", color);

    glBindVertexArray(emptyVAO);
    glDrawArrays(GL_LINE_STRIP, 0, MAX_TRAIL);
    glBindVertexArray(0);
    checkGL("renderObjects:trail");
  }

  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);
}

void RenderSystem::renderParticles(World *world) {
  Vec3 lightPos, lightColor;
  getSunLight(world, lightPos, lightColor);

  this->particleRenderer->render(
      this->viewProj, world->camera, world->particles,
      static_cast<glm::vec3>(lightPos), static_cast<glm::vec3>(lightColor));
}
