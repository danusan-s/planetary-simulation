#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "model_renderer.h"
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

private:
  std::unique_ptr<ModelRenderer> modelRenderer;
  std::unique_ptr<SkyboxRenderer> skyboxRenderer;

  // Returns the light position and color from the sun, or defaults if no sun.
  void getSunLight(World *world, Vec3 &outPos, Vec3 &outColor);

  // Builds a scale+rotation matrix that stretches baseScale along velocity.
  // refSpeed controls how quickly the stretch saturates (reach max at 1x refSpeed).
  static glm::mat4 velocityStretchMatrix(glm::vec3 baseScale, Vec3 velocity,
                                         float refSpeed);
};

#endif // !RENDER_SYSTEM_H
