#ifndef ENGINE_H
#define ENGINE_H

#include "glad/glad.h"

#include "imgui.h"
#include "object_factory.h"
#include "physics_system.h"
#include "render_system.h"
#include "types.h"
#include "world.h"
#include <GLFW/glfw3.h>
#include <memory>

// Game holds all game-related state and functionality.
// Combines all game-related data into a single class for
// easy access to each of the components and manageability.
class Engine {
private:
  struct SpawnParams {
    float mass = 1.0f;
    float radius = 1.0f;
    Vec3 position = Vec3(0.0f);
    Vec3 initialSpeed = Vec3(0.0f);
    Vec3 color = Vec3(1.0f);
  };

  std::unique_ptr<World> world;
  std::unique_ptr<PhysicsSystem> physics;
  std::unique_ptr<RenderSystem> renderer;
  std::unique_ptr<ObjectFactory> objectFactory;
  ImGuiIO *guiIO = nullptr;
  SpawnParams spawnParams;

public:
  // game state
  InputState inputState;
  Viewport viewport;

  // constructor/destructor
  Engine();
  ~Engine();

  // initialize game state (load all shaders/textures/levels)
  void Init();
  // shutdown game state
  void Shutdown();
  // game loop
  void ProcessInput(float deltaTime);
  void Update(float timeStep);
  void Render(float alpha);

private:
  void loadShaders();
  void loadTextures();
  void loadModels();
  void initSystems();
  void initScene();
  void initImGui();
};

#endif
