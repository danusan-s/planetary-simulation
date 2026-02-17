#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "model_renderer.h"
#include "world.h"

class RenderSystem {
public:
  RenderSystem();
  ~RenderSystem();

  void renderWorld(World *world, float alpha);

private:
  ModelRenderer *renderer;
  GLuint debugVAO;
  void setupDebugData();
};

#endif // !RENDER_SYSTEM_H
