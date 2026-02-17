#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>

#include "camera.h"
#include "object_factory.h"
#include "physics_system.h"
#include "render_system.h"
#include "world.h"
#include <GLFW/glfw3.h>

struct Viewport {
  int x;
  int y;
  int width;
  int height;
};

enum Click {
  NO_CLICK,
  LEFT_CLICK,
  RIGHT_CLICK,
  BOTH_CLICK,
};

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Engine {
private:
  PhysicsSystem physics;

  // Init render related stuff later as it needs OpenGL context
  RenderSystem *renderer;
  ObjectFactory *objectFactory;
  World *world;

public:
  // game state
  bool keys[1024];
  Click clickState;
  float mouseX, mouseY;
  float lastMouseX, lastMouseY;
  Viewport viewport;

  // constructor/destructor
  Engine();
  ~Engine();

  // initialize game state (load all shaders/textures/levels)
  void Init();
  // game loop
  void ProcessInput();
  void Update(float timeStep);
  void Render(float alpha);
};

#endif
