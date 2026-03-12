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
  void renderObjects(World *world, float alpha);

private:
  ModelRenderer *modelRenderer;
  SkyboxRenderer *skyboxRenderer;
};

#endif // !RENDER_SYSTEM_H
