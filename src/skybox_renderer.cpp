#include "skybox_renderer.h"

SkyboxRenderer::SkyboxRenderer() {
  float skyboxVertices[] = {                     //   Coordinates
                            -1.0f, -1.0f, 1.0f,  //        7--------6
                            1.0f,  -1.0f, 1.0f,  //       /|       /|
                            1.0f,  -1.0f, -1.0f, //      4--------5 |
                            -1.0f, -1.0f, -1.0f, //      | |      | |
                            -1.0f, 1.0f,  1.0f,  //      | 3------|-2
                            1.0f,  1.0f,  1.0f,  //      |/       |/
                            1.0f,  1.0f,  -1.0f, //      0--------1
                            -1.0f, 1.0f,  -1.0f};

  unsigned int skyboxIndices[] = {// Right
                                  1, 2, 6, 6, 5, 1,
                                  // Left
                                  0, 4, 7, 7, 3, 0,
                                  // Top
                                  4, 5, 6, 6, 7, 4,
                                  // Bottom
                                  0, 3, 2, 2, 1, 0,
                                  // Back
                                  0, 1, 5, 5, 4, 0,
                                  // Front
                                  3, 7, 6, 6, 2, 3};

  glGenVertexArrays(1, &skyboxVAO);
  glGenBuffers(1, &skyboxVBO);
  glGenBuffers(1, &skyboxEBO);
  glBindVertexArray(skyboxVAO);
  glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices,
               GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, skyboxEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(skyboxIndices), &skyboxIndices,
               GL_STATIC_DRAW);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

SkyboxRenderer::~SkyboxRenderer() {
  glDeleteVertexArrays(1, &skyboxVAO);
  glDeleteBuffers(1, &skyboxVBO);
  glDeleteBuffers(1, &skyboxEBO);
}

void SkyboxRenderer::render(Camera &camera, const Cubemap &cubemap,
                            const Shader &shader) {
  shader.Use();
  glm::mat4 viewMat =
      glm::mat4(glm::mat3(camera.GetViewMatrix())); // Remove translation
  shader.SetMatrix4("view", viewMat);
  shader.SetMatrix4("projection", camera.GetProjectionMatrix());

  glDisable(GL_CULL_FACE);
  glDepthFunc(GL_LEQUAL);

  glBindVertexArray(skyboxVAO);

  // Bind cubemap texture
  glActiveTexture(GL_TEXTURE0);
  cubemap.Bind();
  glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  glEnable(GL_CULL_FACE);
  glDepthFunc(GL_LESS);
}
