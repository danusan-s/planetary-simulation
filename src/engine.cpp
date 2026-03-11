#include "engine.h"
#include "camera.h"
#include "resource_manager.h"
#include "utils.h"
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
  std::cout << ">> Loading Shaders" << std::endl;
  ResourceManager::LoadShader(
      Utils::GetAssetPath("shaders/diffuse.vert").c_str(),
      Utils::GetAssetPath("shaders/diffuse.frag").c_str(), nullptr, "diffuse");
  ResourceManager::LoadShader(Utils::GetAssetPath("shaders/debug.vert").c_str(),
                              Utils::GetAssetPath("shaders/debug.frag").c_str(),
                              nullptr, "debug");
  ResourceManager::LoadShader(Utils::GetAssetPath("shaders/trail.vert").c_str(),
                              Utils::GetAssetPath("shaders/trail.frag").c_str(),
                              nullptr, "trail");
  ResourceManager::LoadShader(
      Utils::GetAssetPath("shaders/diffuse.vert").c_str(),
      Utils::GetAssetPath("shaders/sun.frag").c_str(), nullptr, "sun");
  ResourceManager::LoadShader(
      Utils::GetAssetPath("shaders/skybox.vert").c_str(),
      Utils::GetAssetPath("shaders/skybox.frag").c_str(), nullptr, "skybox");

  this->physics = new PhysicsSystem();
  this->world = new World();
  this->renderer = new RenderSystem();
  this->objectFactory = new ObjectFactory(this->world);

  // load textures
  std::cout << ">> Loading Textures" << std::endl;
  ResourceManager::LoadTexture(
      Utils::GetAssetPath("textures/plain.png").c_str(), false, "solid");
  ResourceManager::LoadTexture(Utils::GetAssetPath("textures/sun.jpg").c_str(),
                               false, "sun");
  ResourceManager::LoadTexture(
      Utils::GetAssetPath("textures/earth.jpg").c_str(), false, "earth");
  ResourceManager::LoadTexture(
      Utils::GetAssetPath("textures/mercury.jpg").c_str(), false, "mercury");
  ResourceManager::LoadTexture(
      Utils::GetAssetPath("textures/venus.jpg").c_str(), false, "venus");
  ResourceManager::LoadTexture(Utils::GetAssetPath("textures/mars.jpg").c_str(),
                               false, "mars");
  ResourceManager::LoadTexture(
      Utils::GetAssetPath("textures/jupiter.jpg").c_str(), false, "jupiter");
  ResourceManager::LoadTexture(
      Utils::GetAssetPath("textures/saturn.jpg").c_str(), false, "saturn");
  ResourceManager::LoadTexture(
      Utils::GetAssetPath("textures/uranus.jpg").c_str(), false, "uranus");
  ResourceManager::LoadTexture(
      Utils::GetAssetPath("textures/neptune.jpg").c_str(), false, "neptune");

  // load cubemap
  std::cout << ">> Loading Cubemaps" << std::endl;
  std::string px = Utils::GetAssetPath("textures/_px.jpg");
  std::string nx = Utils::GetAssetPath("textures/_nx.jpg");
  std::string py = Utils::GetAssetPath("textures/_py.jpg");
  std::string ny = Utils::GetAssetPath("textures/_ny.jpg");
  std::string pz = Utils::GetAssetPath("textures/_pz.jpg");
  std::string nz = Utils::GetAssetPath("textures/_nz.jpg");
  ResourceManager::LoadCubemap(
      {px.c_str(), nx.c_str(), py.c_str(), ny.c_str(), pz.c_str(), nz.c_str()},
      false, "space");

  // load models
  std::cout << ">> Loading Models" << std::endl;
  ResourceManager::LoadModel(Utils::GetAssetPath("models/cube.obj").c_str(),
                             "cube");
  ResourceManager::LoadModel(
      Utils::GetAssetPath("models/sphere_smooth.obj").c_str(), "sphere");

  // create objects
  std::cout << "Creating Objects" << std::endl;
  parsePreset(this->objectFactory, this->physics,
              Utils::GetAssetPath("presets/solar_system.txt").c_str());

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

    bool isMoving = false;
    if (this->inputState.keys[GLFW_KEY_W]) {
      this->world->camera.ProcessKeyboard(FORWARD, deltaTime);
      isMoving = true;
    }
    if (this->inputState.keys[GLFW_KEY_S]) {
      this->world->camera.ProcessKeyboard(BACKWARD, deltaTime);
      isMoving = true;
    }
    if (this->inputState.keys[GLFW_KEY_A]) {
      this->world->camera.ProcessKeyboard(LEFT, deltaTime);
      isMoving = true;
    }
    if (this->inputState.keys[GLFW_KEY_D]) {
      this->world->camera.ProcessKeyboard(RIGHT, deltaTime);
      isMoving = true;
    }

    this->world->camera.UpdateSpeed(isMoving, deltaTime);
  }

  this->inputState.lastMouseX = this->inputState.mouseX;
  this->inputState.lastMouseY = this->inputState.mouseY;
}

void Engine::Render(float alpha) {
  this->renderer->renderWorld(this->world, alpha);
  this->renderer->renderGUI(this->world, this->viewport);
}
