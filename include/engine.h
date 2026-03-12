#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>

#include "object_factory.h"
#include "physics_system.h"
#include "render_system.h"
#include "types.h"
#include "world.h"
#include <GLFW/glfw3.h>

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Engine {
private:
  PhysicsSystem *physics;

  // Init render related stuff later as it needs OpenGL context
  RenderSystem *renderer;
  ObjectFactory *objectFactory;
  World *world;

public:
  // game state
  InputState inputState;
  Viewport viewport;

  // constructor/destructor
  Engine();
  ~Engine();

  // initialize game state (load all shaders/textures/levels)
  void Init();
  // game loop
  void ProcessInput(float deltaTime);
  void Update(float timeStep);
  void Render(float alpha);
};

#endif
