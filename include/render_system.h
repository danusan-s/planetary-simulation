#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "model_renderer.h"
#include "skybox_renderer.h"
#include "world.h"

class RenderSystem {
public:
  RenderSystem();
  ~RenderSystem();

  void renderWorld(World *world, float alpha);
  void renderSkybox(World *world);
  void renderObjects(World *world);
  void renderGUI(World *world);
  void renderParticles(World *world);

private:
  ModelRenderer *modelRenderer;
  SkyboxRenderer *skyboxRenderer;
};

#endif // !RENDER_SYSTEM_H
