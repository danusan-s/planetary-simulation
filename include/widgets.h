#ifndef WIDGETS_H
#define WIDGETS_H

#include "types.h"
#include <functional>
#include <iostream>
#include <string>

struct Widget {
  float x, y;
  float width, height;

  bool hovered;
  bool active;

  virtual void update(InputState &input) {
  }
};

struct Button : public Widget {
  std::string label;
  std::function<void()> onClick;

  void update(InputState &input) override {
    // Check if mouse is over the button
    hovered = (input.mouseX >= x && input.mouseX <= x + width &&
               input.mouseY >= y && input.mouseY <= y + height);

    std::cout << "Hovered: " << hovered << std::endl;

    // Check for click
    if (hovered && input.clickState == LEFT_CLICK) {
      active = true;
      if (onClick)
        onClick();
    } else {
      active = false;
    }
  }
};

#endif // !WIDGETS_H
