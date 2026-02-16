#ifndef MODEL_RENDERER_H
#define MODEL_RENDERER_H

#include "camera.h"
#include "model.h"
#include "shader.h"
#include "texture.h"
#include <glm/ext/matrix_float4x4.hpp>

class ModelRenderer {
public:
  ModelRenderer();
  ~ModelRenderer();

  void renderModel(const glm::mat4 &modelMat, Camera *camera,
                   const Model &model, const Texture2D &texture,
                   const Shader &shader);
};

#endif // !MODEL_RENDERER_H
