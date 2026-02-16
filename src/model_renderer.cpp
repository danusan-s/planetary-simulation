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
  texture.Bind();

  // Draw
  glBindVertexArray(model.VAO);
  glDrawArrays(GL_TRIANGLES, 0,
               model.vertices.size() / 8); // divide by 8 floats per vertex
  glBindVertexArray(0);
}
