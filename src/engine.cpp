#include "engine.h"
#include "camera.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "resource_manager.h"
#include "utils.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <filesystem>
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

  std::cout << "Releasing subsystems (GL cleanup)" << std::endl;
  renderer.reset(); // ~RenderSystem -> ~SkyboxRenderer / ~ParticleRenderer
  world.reset();    // ~World -> DestroyObject -> glDeleteBuffers (trails)
  objectFactory.reset();
  physics.reset();
  std::cout << "Subsystems released" << std::endl;

  std::cout << "Attempting to clear ResourceManager" << std::endl;
  ResourceManager::Clear();
  std::cout << "ResourceManager successfully cleared" << std::endl;
}

void Engine::loadShaders() {
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
  ResourceManager::LoadShader(
      Utils::GetAssetPath("shaders/particle.vert").c_str(),
      Utils::GetAssetPath("shaders/particle.frag").c_str(), nullptr,
      "particle");
}

void Engine::loadTextures() {
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
}

void Engine::loadModels() {
  std::cout << ">> Loading Models" << std::endl;
  ResourceManager::LoadModel(Utils::GetAssetPath("models/cube.obj").c_str(),
                             "cube");

  ResourceManager::LoadModel(
      Utils::GetAssetPath("models/sphere_smooth.obj").c_str(), "sphere");

  ResourceManager::LoadModel(
      Utils::GetAssetPath("models/icosphere_low_smooth.obj").c_str(), "debris");
}

void Engine::initSystems() {
  this->physics = std::make_unique<PhysicsSystem>();
  this->world = std::make_unique<World>();
  this->renderer = std::make_unique<RenderSystem>();
  this->objectFactory = std::make_unique<ObjectFactory>(this->world.get());
}

void Engine::initImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  this->guiIO = &ImGui::GetIO();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(this->window, true);
  ImGui_ImplOpenGL3_Init("#version 330 core");
}

void Engine::scanPresets() {
  std::string presetsDir = Utils::GetAssetPath("presets");
  presetFiles.clear();

  try {
    for (const auto &entry : std::filesystem::directory_iterator(presetsDir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".txt") {
        presetFiles.push_back(entry.path().filename().string());
      }
    }
  } catch (const std::filesystem::filesystem_error &e) {
    std::cerr << "Warning: could not scan presets directory: " << e.what()
              << std::endl;
  }

  std::sort(presetFiles.begin(), presetFiles.end());

  if (menuConfig.selectedPreset >= static_cast<int>(presetFiles.size()))
    menuConfig.selectedPreset = 0;
}

void Engine::startScene() {
  std::cout << "Creating Objects" << std::endl;

  switch (menuConfig.mode) {
    case SceneMode::Random:
      // Apply menu config to object factory
      this->objectFactory->positionRange = menuConfig.positionRange;
      this->objectFactory->minPlanetMass = menuConfig.minMass;
      this->objectFactory->maxPlanetMass = menuConfig.maxMass;
      this->objectFactory->colorMin = menuConfig.colorMin;
      this->objectFactory->colorMax = menuConfig.colorMax;
      this->objectFactory->generateRandomSystem(menuConfig.planetCount);
      break;
    case SceneMode::Preset:
      if (menuConfig.selectedPreset < static_cast<int>(presetFiles.size())) {
        std::string path = Utils::GetAssetPath(
            "presets/" + presetFiles[menuConfig.selectedPreset]);
        this->physics->G = this->objectFactory->parsePreset(path.c_str());
      }
      break;
  }

  state = EngineState::Loading;
  loadTimer = 0.0f;
}

void Engine::returnToMenu() {
  objectFactory.reset();
  world.reset();
  physics.reset();
  renderer.reset();

  initSystems();

  scanPresets();
  state = EngineState::Menu;
  loadTimer = 0.0f;
  timeScale = 1.0f;
  spawnParams = SpawnParams{};
}

void Engine::Init(GLFWwindow *win) {
  this->window = win;
  initSystems();
  loadShaders();
  loadTextures();
  loadModels();
  scanPresets();
  initImGui();
  state = EngineState::Menu;
}

void Engine::Update(float timeStep) {
  if (state == EngineState::Loading) {
    loadTimer += timeStep;
    if (loadTimer >= LOAD_DELAY) {
      state = EngineState::Running;
      std::cout << "Loading complete, starting simulation" << std::endl;
    }
    return;
  }

  if (state != EngineState::Running)
    return;

  this->physics->step(this->world.get(), timeStep, this->objectFactory.get());
}

void Engine::ProcessInput(float deltaTime) {
  if (this->inputState.keys[GLFW_KEY_ESCAPE]) {
    if (state == EngineState::Menu) {
      glfwSetWindowShouldClose(this->window, true);
    } else {
      returnToMenu();
      this->inputState.keys[GLFW_KEY_ESCAPE] = false; // consume the key
      return;
    }
  }

  // In Menu, keep cursor free for ImGui interaction
  if (state == EngineState::Menu) {
    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    this->inputState.lastMouseX = this->inputState.mouseX;
    this->inputState.lastMouseY = this->inputState.mouseY;
    return;
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
    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
    glfwSetInputMode(this->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  this->inputState.lastMouseX = this->inputState.mouseX;
  this->inputState.lastMouseY = this->inputState.mouseY;
}

void Engine::renderMenu() {
  ImVec2 displaySize = ImGui::GetIO().DisplaySize;

  // Full-screen background window
  ImGui::SetNextWindowPos(ImVec2(0, 0));
  ImGui::SetNextWindowSize(displaySize);
  ImGui::Begin("##MenuBackground", nullptr,
               ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                   ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                   ImGuiWindowFlags_NoCollapse |
                   ImGuiWindowFlags_NoBringToFrontOnFocus);

  // Center a panel in the middle of the screen
  float panelWidth = 420.0f;
  float panelHeight = 380.0f;
  float panelX = (displaySize.x - panelWidth) * 0.5f;
  float panelY = (displaySize.y - panelHeight) * 0.5f;

  ImGui::SetCursorPos(ImVec2(panelX, panelY));
  ImGui::BeginChild("##MenuPanel", ImVec2(panelWidth, panelHeight), true);

  // Title
  const char *title = "Planetary Simulation";
  float titleWidth = ImGui::CalcTextSize(title).x;
  ImGui::SetCursorPosX((panelWidth - titleWidth) * 0.5f);
  ImGui::TextUnformatted(title);
  ImGui::Separator();
  ImGui::Spacing();

  // Scene mode selection
  int mode = static_cast<int>(menuConfig.mode);
  ImGui::RadioButton("Random Generation", &mode, 0);
  ImGui::SameLine();
  ImGui::RadioButton("Load Preset", &mode, 1);
  menuConfig.mode = static_cast<SceneMode>(mode);

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Mode-specific options
  if (menuConfig.mode == SceneMode::Random) {
    ImGui::DragInt("Planet Count", &menuConfig.planetCount, 1.0f, 1, 500);

    ImGui::Spacing();
    ImGui::DragFloat("Position Range", &menuConfig.positionRange, 1.0f, 10.0f,
                     2000.0f, "%.0f");

    ImGui::Spacing();
    ImGui::Text("Mass Range:");
    ImGui::DragFloat("Min Mass", &menuConfig.minMass, 0.1f, 0.1f,
                     menuConfig.maxMass, "%.1f");
    ImGui::DragFloat("Max Mass", &menuConfig.maxMass, 1.0f, menuConfig.minMass,
                     100000.0f, "%.1f");

    ImGui::Spacing();
    ImGui::Text("Color Range:");
    ImGui::DragFloat("Color Min", &menuConfig.colorMin, 0.01f, 0.0f,
                     menuConfig.colorMax, "%.2f");
    ImGui::DragFloat("Color Max", &menuConfig.colorMax, 0.01f,
                     menuConfig.colorMin, 1.0f, "%.2f");
  } else {
    if (presetFiles.empty()) {
      ImGui::TextWrapped("No preset files found in presets/ directory.");
    } else {
      ImGui::Text("Select a preset:");
      ImGui::BeginChild("##PresetList", ImVec2(0, 150), true);
      for (int i = 0; i < static_cast<int>(presetFiles.size()); i++) {
        bool selected = (menuConfig.selectedPreset == i);
        if (ImGui::Selectable(presetFiles[i].c_str(), selected)) {
          menuConfig.selectedPreset = i;
        }
      }
      ImGui::EndChild();
    }
  }

  ImGui::Spacing();
  ImGui::Separator();
  ImGui::Spacing();

  // Start button — centered
  float buttonWidth = 200.0f;
  float buttonHeight = 30.0f;
  ImGui::SetCursorPosX((panelWidth - buttonWidth) * 0.5f);

  bool canStart = true;
  if (menuConfig.mode == SceneMode::Preset && presetFiles.empty())
    canStart = false;

  if (!canStart)
    ImGui::BeginDisabled();

  if (ImGui::Button("Start Simulation", ImVec2(buttonWidth, buttonHeight))) {
    startScene();
  }

  if (!canStart)
    ImGui::EndDisabled();

  ImGui::EndChild();
  ImGui::End();
}

void Engine::renderHUD() {
  ImGui::Begin("Controls");

  // -- Debug info --
  ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
  ImGui::Text("Camera Position: (%.2f, %.2f, %.2f)", world->camera.position.x,
              world->camera.position.y, world->camera.position.z);

  ImGui::Separator();

  // -- Simulation speed --
  ImGui::InputFloat("Time Scale", &timeScale, 0.01f, 0.1f, "%.2f");
  if (timeScale < 0.0f)
    timeScale = 0.0f;
  if (timeScale > 5.0f)
    timeScale = 5.0f;

  // -- Camera speed --
  ImGui::InputFloat("Camera Speed", &world->camera.maxSpeed, 10.0f, 50.0f,
                    "%.0f");
  if (world->camera.maxSpeed < 1.0f)
    world->camera.maxSpeed = 1.0f;

  ImGui::Separator();

  // -- Spawner --
  if (ImGui::Button("Spawn Random Planet")) {
    this->objectFactory->generateRandomPlanet();
  }

  ImGui::DragFloat("Mass", &spawnParams.mass, 1.0f, 1.0f, 1000.0f);
  ImGui::DragFloat("Radius", &spawnParams.radius, 0.01f, 0.1f, 10.0f);
  ImGui::InputFloat3("Position", &spawnParams.position.x);
  ImGui::InputFloat3("Initial Speed", &spawnParams.initialSpeed.x);
  ImGui::ColorEdit3("Color", &spawnParams.color.x);
  if (ImGui::Button("Spawn Custom Planet")) {
    this->objectFactory->spawnPlanet(spawnParams.position, spawnParams.radius,
                                     spawnParams.mass, spawnParams.initialSpeed,
                                     spawnParams.color, "solid");
  }

  ImGui::Separator();

  // -- Navigation --
  if (ImGui::Button("Back to Menu")) {
    returnToMenu();
  }

  ImGui::End();
}

void Engine::Render() {
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  if (state == EngineState::Menu) {
    renderMenu();
  } else {
    // Loading and Running both render the world
    this->renderer->renderWorld(this->world.get());
    renderHUD();
  }

  ImGui::Render();
  ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
