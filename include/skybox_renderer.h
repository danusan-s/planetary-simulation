#ifndef SKYBOX_RENDERER_H
#define SKYBOX_RENDERER_H

#include "camera.h"
#include "cubemap.h"
#include "shader.h"

class SkyboxRenderer {
public:
  SkyboxRenderer();
  ~SkyboxRenderer();

  GLuint skyboxVAO, skyboxVBO, skyboxEBO;

  void render(Camera &camera, const Cubemap &cubemap, const Shader &shader);
};

#endif // !SKYBOX_RENDERER_H
