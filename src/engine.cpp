#include "engine.h"
#include "camera.h"
#include "resource_manager.h"
#include <GLFW/glfw3.h>
#include <fstream>
#include <iostream>

Engine::Engine() {
  this->physics = nullptr;
  this->renderer = nullptr;
  this->objectFactory = nullptr;
}

Engine::~Engine() {
  std::cout << "Attempting to delete Engine Object" << std::endl;

  if (this->physics) {
    std::cout << "Deleting Physics System" << std::endl;
    delete this->physics;
  }
  if (this->renderer) {
    std::cout << "Deleting Render System" << std::endl;
    delete this->renderer;
  }
  if (this->objectFactory) {
    std::cout << "Deleting Object Factory" << std::endl;
    delete this->objectFactory;
  }

  std::cout << "Game Object successfully deleted" << std::endl;
}

void parsePreset(ObjectFactory *factory, PhysicsSystem *physics,
                 const char *filePath) {
  std::ifstream presetFile(filePath);
  if (!presetFile.is_open()) {
    std::cerr << "Failed to open preset file: " << filePath << std::endl;
    return;
  }

  std::cout << "Parsing preset file: " << filePath << std::endl;

  std::string line;
  while (std::getline(presetFile, line)) {
    std::cout << "Read line: " << line << std::endl;
    if (line == "constants") {
      std::cout << "Parsing constants" << std::endl;
      while (std::getline(presetFile, line)) {
        if (line.empty())
          break; // End of constants definition

        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;

        if (prefix == "G") {
          iss >> physics->G;
          std::cout << "Set G constant to: " << physics->G << std::endl;
        }
      }
    } else {
      // Treat as planet name/texture ID or "sun"
      std::string type = line;
      if (type.empty())
        continue;

      std::cout << "Parsing object definition: " << type << std::endl;
      float posX = 0, posY = 0, posZ = 0, radius = 1, mass = 1, velX = 0,
            velY = 0, velZ = 0, colorR = 1, colorG = 1, colorB = 1;
      while (std::getline(presetFile, line)) {
        if (line.empty())
          break; // End of definition

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
      if (type == "sun")
        factory->spawnSun(Vec3(posX, posY, posZ), radius, mass,
                          Vec3(velX, velY, velZ), Vec3(colorR, colorG, colorB));
      else {
        if (!ResourceManager::TextureExists(type)) {
          std::cerr << "Texture for type '" << type
                    << "' not found. Using default texture." << std::endl;
          type = "solid"; // Fallback to a default texture
        }
        factory->spawnPlanet(Vec3(posX, posY, posZ), radius, mass,
                             Vec3(velX, velY, velZ),
                             Vec3(colorR, colorG, colorB), type);
      }
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
  ResourceManager::LoadShader("../shaders/trail.vert", "../shaders/trail.frag",
                              nullptr, "trail");
  ResourceManager::LoadShader("../shaders/diffuse.vert", "../shaders/sun.frag",
                              nullptr, "sun");
  ResourceManager::LoadShader("../shaders/skybox.vert",
                              "../shaders/skybox.frag", nullptr, "skybox");

  this->physics = new PhysicsSystem();
  this->world = new World();
  this->renderer = new RenderSystem();
  this->objectFactory = new ObjectFactory(this->world);

  // load textures
  std::cout << "Loading Textures" << std::endl;
  ResourceManager::LoadTexture("../textures/plain.png", false, "solid");
  ResourceManager::LoadTexture("../textures/sun.jpg", false, "sun");
  ResourceManager::LoadTexture("../textures/earth.jpg", false, "earth");
  ResourceManager::LoadTexture("../textures/mercury.jpg", false, "mercury");
  ResourceManager::LoadTexture("../textures/venus.jpg", false, "venus");
  ResourceManager::LoadTexture("../textures/mars.jpg", false, "mars");
  ResourceManager::LoadTexture("../textures/jupiter.jpg", false, "jupiter");
  ResourceManager::LoadTexture("../textures/saturn.jpg", false, "saturn");
  ResourceManager::LoadTexture("../textures/uranus.jpg", false, "uranus");
  ResourceManager::LoadTexture("../textures/neptune.jpg", false, "neptune");

  // load cubemap
  std::cout << "Loading Cubemap" << std::endl;
  ResourceManager::LoadCubemap({"../textures/_px.jpg", "../textures/_nx.jpg",
                                "../textures/_py.jpg", "../textures/_ny.jpg",
                                "../textures/_pz.jpg", "../textures/_nz.jpg"},
                               false, "space");

  // load models
  std::cout << "Loading Models" << std::endl;
  ResourceManager::LoadModel("../models/cube.obj", "cube");
  ResourceManager::LoadModel("../models/sphere_smooth.obj", "sphere");

  // create objects
  std::cout << "Creating Objects" << std::endl;
  parsePreset(this->objectFactory, this->physics,
              "../presets/solar_system.txt");

  // std::cout << "Creating GUI" << std::endl;
  // To be implemented later
}

void Engine::Update(float timeStep) {
  this->physics->step(this->world, timeStep);
}

void Engine::ProcessInput(float deltaTime) {
  if (this->inputState.keys[GLFW_KEY_ESCAPE]) {
    glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
  }
  if (this->inputState.keys[GLFW_KEY_LEFT_CONTROL]) {
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    for (auto &widget : this->world->widgets) {
      widget.update(this->inputState);
    }
  } else {
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR,
                     GLFW_CURSOR_DISABLED);
    float xoffset = this->inputState.mouseX - this->inputState.lastMouseX;
    float yoffset =
        this->inputState.lastMouseY -
        this->inputState
            .mouseY; // reversed since y-coordinates go from bottom to top
    this->world->camera.ProcessMouseMovement(xoffset, yoffset);

    if (this->inputState.keys[GLFW_KEY_W])
      this->world->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (this->inputState.keys[GLFW_KEY_S])
      this->world->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (this->inputState.keys[GLFW_KEY_A])
      this->world->camera.ProcessKeyboard(LEFT, deltaTime);
    if (this->inputState.keys[GLFW_KEY_D])
      this->world->camera.ProcessKeyboard(RIGHT, deltaTime);
  }

  this->inputState.lastMouseX = this->inputState.mouseX;
  this->inputState.lastMouseY = this->inputState.mouseY;
}

void Engine::Render(float alpha) {
  this->renderer->renderWorld(this->world, alpha);
  this->renderer->renderGUI(this->world, this->viewport);
}
