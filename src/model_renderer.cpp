#include "model_renderer.h"
#include "camera.h"
#include "texture.h"
#include <glm/ext/matrix_float4x4.hpp>

ModelRenderer::ModelRenderer() {
}

ModelRenderer::~ModelRenderer() {
}

void ModelRenderer::renderModel(const glm::mat4 &modelMat, Camera &camera,
                                const Model &model, const Texture2D &texture,
                                const Shader &shader) {

  shader.Use();

  // Set your model/view/projection uniforms here
  shader.SetMatrix4("model", modelMat);
  shader.SetMatrix4("view", camera.GetViewMatrix());
  shader.SetMatrix4("projection", camera.GetProjectionMatrix());

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  texture.Bind();

  // Draw
  glBindVertexArray(model.VAO);
  glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
