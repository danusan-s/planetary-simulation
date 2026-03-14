#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "model_renderer.h"
#include "particle_renderer.h"
#include "skybox_renderer.h"
#include "world.h"
#include <glm/glm.hpp>
#include <memory>

class RenderSystem {
public:
  RenderSystem();
  ~RenderSystem();

  void renderWorld(World *world);
  void renderSkybox(World *world);
  void renderObjects(World *world);
  void renderParticles(World *world);

  glm::mat4 viewProj;

private:
  std::unique_ptr<ModelRenderer> modelRenderer;
  std::unique_ptr<SkyboxRenderer> skyboxRenderer;
  std::unique_ptr<ParticleRenderer> particleRenderer;

  // Empty VAO for attribute-less trail draws (positions come from SSBOtrail).
  GLuint emptyVAO = 0;

  void getSunLight(World *world, Vec3 &outPos, Vec3 &outColor);
  void updateViewProjection(World *world);
};

#endif // !RENDER_SYSTEM_H
