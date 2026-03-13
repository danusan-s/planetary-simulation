#include "engine.h"
#include "camera.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "resource_manager.h"
#include "utils.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <iostream>

Engine::Engine() = default;

Engine::~Engine() {
  std::cout << "Engine destroyed" << std::endl;
}

void Engine::Shutdown() {
  std::cout << "Attempting to Delete ImGui Contexts" << std::endl;
  if (ImGui::GetCurrentContext()) {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    std::cout << "ImGui Contexts successfully deleted" << std::endl;
  }

  std::cout << "Attempting to clear ResourceManager" << std::endl;
  ResourceManager::Clear();
  std::cout << "ResourceManager successfully cleared" << std::endl;
}

void Engine::Init() {
  // load shaders
  std::cout << ">> Loading Shaders" << std::endl;

  ResourceManager::LoadShader(
      Utils::GetAssetPath("shaders/diffuse.vert").c_str(),
      Utils::GetAssetPath("shaders/diffuse.frag").c_str(), nullptr, "diffuse");

  ResourceManager::LoadShader(Utils::GetAssetPath("shaders/trail.vert").c_str(),
                              Utils::GetAssetPath("shaders/trail.frag").c_str(),
                              nullptr, "trail");

  ResourceManager::LoadShader(
      Utils::GetAssetPath("shaders/diffuse.vert").c_str(),
      Utils::GetAssetPath("shaders/sun.frag").c_str(), nullptr, "sun");

  ResourceManager::LoadShader(
      Utils::GetAssetPath("shaders/skybox.vert").c_str(),
      Utils::GetAssetPath("shaders/skybox.frag").c_str(), nullptr, "skybox");

  this->physics = std::make_unique<PhysicsSystem>();
  this->world = std::make_unique<World>();
  this->renderer = std::make_unique<RenderSystem>();
  this->objectFactory = std::make_unique<ObjectFactory>(this->world.get());

  // load textures
  std::cout << ">> Loading Textures" << std::endl;
  ResourceManager::LoadTexture(
      Utils::GetAssetPath("textures/white.png").c_str(), false, "solid");
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
  ResourceManager::LoadModel(Utils::GetAssetPath("models/quad.obj").c_str(),
                             "quad");

  // create objects
  std::cout << "Creating Objects" << std::endl;

  // To use presets:
  // this->physics->G = this->objectFactory->parsePreset(
  //     Utils::GetAssetPath("presets/empty.txt").c_str());

  // To use random generation:
  this->objectFactory->generateRandomSystem(30);

  // initialize GUI
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  this->guiIO = &ImGui::GetIO();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
  ImGui_ImplOpenGL3_Init("#version 330 core");
}

void Engine::Update(float timeStep) {
  this->physics->step(this->world.get(), timeStep, this->objectFactory.get());
}

void Engine::ProcessInput(float deltaTime) {
  if (this->inputState.keys[GLFW_KEY_ESCAPE]) {
    glfwSetWindowShouldClose(glfwGetCurrentContext(), true);
  }

  static bool capsLockPressed = false;
  if (this->inputState.keys[GLFW_KEY_CAPS_LOCK]) {
    if (!capsLockPressed) {
      this->inputState.cursorLocked = !this->inputState.cursorLocked;
      capsLockPressed = true;
    }
  } else {
    capsLockPressed = false;
  }

  if (this->inputState.cursorLocked) {
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
    if (this->inputState.keys[GLFW_KEY_SPACE]) {
      this->world->camera.ProcessKeyboard(UP, deltaTime);
      isMoving = true;
    }
    if (this->inputState.keys[GLFW_KEY_LEFT_CONTROL]) {
      this->world->camera.ProcessKeyboard(DOWN, deltaTime);
      isMoving = true;
    }

    this->world->camera.UpdateSpeed(isMoving, deltaTime);
  } else {
    glfwSetInputMode(glfwGetCurrentContext(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  this->inputState.lastMouseX = this->inputState.mouseX;
  this->inputState.lastMouseY = this->inputState.mouseY;
}

struct SpawnParams {
  float mass = 1.0f;
  float radius = 1.0f;
  Vec3 position = Vec3(0.0f);
  Vec3 initialSpeed = Vec3(0.0f);
  Vec3 color = Vec3(1.0f);
};

SpawnParams spawnParams;

void Engine::Render(float alpha) {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();
  this->renderer->renderWorld(this->world.get(), alpha);

  ImGui::Begin("Debug Info");
  ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
  ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", world->camera.Position.x,
              world->camera.Position.y, world->camera.Position.z);
  ImGui::End();

  ImGui::Begin("Spawner");
  if (ImGui::Button("Spawn Random Planet")) {
    this->objectFactory->generateRandomPlanet();
  }

  ImGui::SliderFloat("Mass", &spawnParams.mass, 1.0f, 1000.0f);
  ImGui::SliderFloat("Radius", &spawnParams.radius, 0.1f, 10.0f);
  ImGui::InputFloat3("Position", &spawnParams.position.x);
  ImGui::InputFloat3("Initial Speed", &spawnParams.initialSpeed.x);
  ImGui::ColorEdit3("Color", &spawnParams.color.x);
  if (ImGui::Button("Spawn Custom Planet")) {
    this->objectFactory->spawnPlanet(spawnParams.position, spawnParams.radius,
                                     spawnParams.mass, spawnParams.initialSpeed,
                                     spawnParams.color, "solid");
  }
  ImGui::End();

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
