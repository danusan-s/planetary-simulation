#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "model_renderer.h"
#include "skybox_renderer.h"
#include "world.h"
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
};

#endif // !RENDER_SYSTEM_H
