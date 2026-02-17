#include "engine.h"
#include "camera.h"
#include "resource_manager.h"
#include <fstream>
#include <iostream>

Engine::Engine() {
  this->physics = PhysicsSystem();
  this->renderer = nullptr;
  this->objectFactory = nullptr;
  this->clickState = NO_CLICK;

  this->mouseX = 0.0f;
  this->mouseY = 0.0f;
  this->lastMouseX = 0.0f;
  this->lastMouseY = 0.0f;
}

Engine::~Engine() {
  std::cout << "Attempting to delete Engine Object" << std::endl;

  delete this->renderer;
  delete this->objectFactory;

  std::cout << "Game Object successfully deleted" << std::endl;
}

void parsePreset(ObjectFactory *factory, const char *filePath) {
  std::ifstream presetFile(filePath);
  if (!presetFile.is_open()) {
    std::cerr << "Failed to open preset file: " << filePath << std::endl;
    return;
  }

  std::cout << "Parsing preset file: " << filePath << std::endl;

  std::string line;
  while (std::getline(presetFile, line)) {
    std::cout << "Read line: " << line << std::endl;
    if (line == "planet") {
      std::cout << "Parsing planet definition" << std::endl;
      float posX, posY, posZ, radius, mass, velX, velY, velZ, colorR, colorG,
          colorB;
      while (std::getline(presetFile, line)) {
        if (line.empty())
          break; // End of planet definition

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "pos") {
          iss >> posX >> posY >> posZ;
        } else if (prefix == "radius") {
          iss >> radius;
        } else if (prefix == "mass") {
          iss >> mass;
        } else if (prefix == "vel") {
          iss >> velX >> velY >> velZ;
        } else if (prefix == "color") {
          iss >> colorR >> colorG >> colorB;
        }
      }
      factory->spawnPlanet(Vec3(posX, posY, posZ), radius, mass,
                           Vec3(velX, velY, velZ),
                           Vec3(colorR, colorG, colorB));
    }
  }
}

void Engine::Init() {
  // Seed the random number generator
  srand(static_cast<unsigned int>(time(0)));

  // load shaders
  std::cout << "Loading Shaders" << std::endl;
  ResourceManager::LoadShader("../shaders/diffuse.vert",
                              "../shaders/diffuse.frag", nullptr, "diffuse");
  ResourceManager::LoadShader("../shaders/debug.vert", "../shaders/debug.frag",
                              nullptr, "debug");

  this->world = new World();
  this->renderer = new RenderSystem();
  this->objectFactory = new ObjectFactory(this->world);

  // load textures
  std::cout << "Loading Textures" << std::endl;
  ResourceManager::LoadTexture("../textures/plain.png", false, "solid");

  // load models
  std::cout << "Loading Models" << std::endl;
  ResourceManager::LoadModel("../models/quad_sphere.obj", "sphere");
  ResourceManager::LoadModel("../models/cube.obj", "cube");

  // create objects
  std::cout << "Creating Objects" << std::endl;
  parsePreset(this->objectFactory, "../presets/three_planets.txt");
}

void Engine::Update(float timeStep) {
  this->physics.step(this->world, timeStep);
}

void Engine::ProcessInput(float deltaTime) {
  float xoffset = this->mouseX - this->lastMouseX;
  float yoffset =
      this->lastMouseY -
      this->mouseY; // reversed since y-coordinates go from bottom to top
  this->world->camera.ProcessMouseMovement(xoffset, yoffset);

  this->lastMouseX = this->mouseX;
  this->lastMouseY = this->mouseY;

  if (keys[GLFW_KEY_W])
    this->world->camera.ProcessKeyboard(FORWARD, deltaTime);
  if (keys[GLFW_KEY_S])
    this->world->camera.ProcessKeyboard(BACKWARD, deltaTime);
  if (keys[GLFW_KEY_A])
    this->world->camera.ProcessKeyboard(LEFT, deltaTime);
  if (keys[GLFW_KEY_D])
    this->world->camera.ProcessKeyboard(RIGHT, deltaTime);
}

void Engine::Render(float alpha) {
  this->renderer->renderWorld(this->world, alpha);
}
