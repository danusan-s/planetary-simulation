#ifndef WIDGET_RENDERER_H
#define WIDGET_RENDERER_H

#include "camera.h"
#include "shader.h"
#include "texture.h"
#include "widgets.h"
#include <glm/ext/matrix_float4x4.hpp>

class WidgetRenderer {
public:
  WidgetRenderer();
  ~WidgetRenderer();

  GLuint widgetVAO, widgetVBO;

  void renderWidget(const Widget &widget, const Texture2D &texture,
                    const Shader &shader, const glm::vec3 &color,
                    const float screenWidth, const float screenHeight);
};

#endif // !WIDGET_RENDERER_H
