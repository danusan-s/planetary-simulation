#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include "model_renderer.h"
#include "widget_renderer.h"
#include "world.h"

class RenderSystem {
public:
  RenderSystem();
  ~RenderSystem();

  void renderWorld(World *world, float alpha);
  void renderGUI(World *world, Viewport &viewport);

private:
  ModelRenderer *modelRenderer;
  WidgetRenderer *widgetRenderer;
};

#endif // !RENDER_SYSTEM_H
