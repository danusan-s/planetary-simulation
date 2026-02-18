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
                                const Shader &shader, const glm::vec3 &color,
                                const glm::vec3 &lightPos,
                                const glm::vec3 &lightColor) {

  shader.Use();

  // Set your model/view/projection uniforms here
  shader.SetMatrix4("model", modelMat);
  shader.SetMatrix4("view", camera.GetViewMatrix());
  shader.SetMatrix4("projection", camera.GetProjectionMatrix());

  shader.SetVector3f("objectColor", color);
  shader.SetVector3f("lightPos", lightPos);
  shader.SetVector3f("lightColor", lightColor);

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  texture.Bind();

  // Draw
  glBindVertexArray(model.VAO);
  glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
