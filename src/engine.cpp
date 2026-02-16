#include "engine.h"
#include "resource_manager.h"
#include <iostream>

Engine::Engine() {
  this->world = World();
  this->physics = PhysicsSystem();
  this->renderer = nullptr;
  this->objectFactory = nullptr;
  this->clickState = NO_CLICK;

  float mouseX = 0.0f;
  float mouseY = 0.0f;
}

Engine::~Engine() {
  std::cout << "Attempting to delete Engine Object" << std::endl;

  delete this->renderer;
  delete this->objectFactory;

  std::cout << "Game Object successfully deleted" << std::endl;
}

void Engine::Init() {
  // Seed the random number generator
  srand(static_cast<unsigned int>(time(0)));

  // load shaders
  std::cout << "Loading Shaders" << std::endl;
  ResourceManager::LoadShader("../shaders/diffuse.vert",
                              "../shaders/diffuse.frag", nullptr, "diffuse");

  this->renderer = new RenderSystem();
  this->objectFactory = new ObjectFactory(this->world);
  this->world.Init();

  // load textures
  std::cout << "Loading Textures" << std::endl;
  ResourceManager::LoadTexture("../textures/plain.png", false, "solid");

  // load models
  std::cout << "Loading Models" << std::endl;
  ResourceManager::LoadModel("../models/quad_sphere.obj", "sphere");
  ResourceManager::LoadModel("../models/cube.obj", "cube");

  // create objects
  std::cout << "Creating Objects" << std::endl;
  objectFactory->spawnPlanet(Vec3(1.0f), 1.0f, Vec3(1.0f), 1.0f, 0.0f);
}

void Engine::Update(float timeStep) {
  this->physics.step(this->world, timeStep);
}

void Engine::ProcessInput() {
  float xoffset = this->mouseX - this->lastMouseX;
  float yoffset =
      this->lastMouseY -
      this->mouseY; // reversed since y-coordinates go from bottom to top
  this->world.camera->ProcessMouseMovement(xoffset, yoffset);
}

void Engine::Render(float alpha) {
  this->renderer->renderWorld(this->world, alpha);
}
