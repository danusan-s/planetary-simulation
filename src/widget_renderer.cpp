#include "widget_renderer.h"

WidgetRenderer::WidgetRenderer() {
  // Debug render
  float vertices[] = {0.0f, 0.0f, -0.5f, 0.0f, 0.0f,  1.0f, 0.0f, -0.5f,
                      1.0f, 0.0f, 1.0f,  1.0f, -0.5f, 1.0f, 1.0f,

                      1.0f, 1.0f, -0.5f, 1.0f, 1.0f,  0.0f, 1.0f, -0.5f,
                      0.0f, 1.0f, 0.0f,  0.0f, -0.5f, 0.0f, 0.0f};

  glGenVertexArrays(1, &widgetVAO);
  glGenBuffers(1, &widgetVBO);

  glBindVertexArray(widgetVAO);

  glBindBuffer(GL_ARRAY_BUFFER, widgetVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Position (location = 0)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  // UV (location = 1)
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glBindVertexArray(0);
}

WidgetRenderer::~WidgetRenderer() {
  glDeleteVertexArrays(1, &widgetVAO);
  glDeleteBuffers(1, &widgetVBO);
}

void WidgetRenderer::renderWidget(const Widget &widget,
                                  const Texture2D &texture,
                                  const Shader &shader, const glm::vec3 &color,
                                  const float screenWidth,
                                  const float screenHeight) {
  shader.Use();

  // Set your model/view/projection uniforms here
  glm::mat4 modelMat = glm::mat4(1.0f);
  modelMat = glm::translate(modelMat, glm::vec3(widget.x, widget.y, 0.0f));
  modelMat = glm::scale(modelMat, glm::vec3(widget.width, widget.height, 1.0f));

  shader.SetMatrix4("model", modelMat);

  glm::mat4 projection =
      glm::ortho(0.0f, screenWidth, screenHeight, 0.0f, -1.0f, 1.0f);
  shader.SetMatrix4("projection", projection);

  shader.SetVector3f("color", color);

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  texture.Bind();

  // Draw
  glBindVertexArray(widgetVAO);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}
