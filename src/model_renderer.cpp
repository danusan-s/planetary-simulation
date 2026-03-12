#include "model_renderer.h"
#include "camera.h"
#include "texture.h"
#include "types.h"
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

void ModelRenderer::renderTrail(Camera &camera, const Shader &shader,
                                int trailHead, GLuint trailVAO,
                                const glm::vec3 &color) {
  shader.Use();
  shader.SetMatrix4("view", camera.GetViewMatrix());
  shader.SetMatrix4("projection", camera.GetProjectionMatrix());
  shader.SetVector3f("trailColor", color);
  shader.SetInteger("trailHead", trailHead);
  shader.SetInteger("maxTrail", MAX_TRAIL);

  glBindVertexArray(trailVAO);
  int start = (trailHead + 1) % MAX_TRAIL;

  glDrawArrays(GL_LINE_STRIP, start, MAX_TRAIL - start);
  glDrawArrays(GL_LINE_STRIP, 0, start);
}
