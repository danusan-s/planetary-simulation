#include <glad/glad.h>

#include "engine.h"
#include "resource_manager.h"
#include <GLFW/glfw3.h>

#include <iostream>

// GLFW function declarations
Viewport letterbox_viewport(int windowWidth, int windowHeight);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mode);
void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods);
void mouse_callback(GLFWwindow *window, double xposIn, double yposIn);

// The Width of the screen
// Modify only width if you want to change size
const unsigned int INIT_SCREEN_WIDTH = 1920;
// The height of the screen
const unsigned int INIT_SCREEN_HEIGHT = 1080;

Engine engineObj = Engine();

int main(int argc, char *argv[]) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  // Temporary fix to deal with resizing of window
  // glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  GLFWwindow *window = glfwCreateWindow(INIT_SCREEN_WIDTH, INIT_SCREEN_HEIGHT,
                                        "Engine", nullptr, nullptr);
  glfwMakeContextCurrent(window);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_SCISSOR_TEST);
  glEnable(GL_BLEND);

  glfwSetKeyCallback(window, key_callback);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  engineObj.viewport =
      letterbox_viewport(INIT_SCREEN_WIDTH, INIT_SCREEN_HEIGHT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // initialize game
  // ---------------
  engineObj.Init();

  // deltaTime variables
  // -------------------
  float deltaTime = 0.0f;
  float lastFrame = 0.0f;
  float accumulatedTime = 0.0f;

  //  60 fps simulation
  const float timeStep = 1 / 240.0f;

  while (!glfwWindowShouldClose(window)) {
    // calculate delta time
    // --------------------
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    accumulatedTime += deltaTime;
    lastFrame = currentFrame;
    glfwPollEvents();

    // process input
    // -------------
    engineObj.ProcessInput(deltaTime);

    while (accumulatedTime >= timeStep) {
      // compute physics
      // ---------------
      engineObj.Update(timeStep);
      accumulatedTime -= timeStep;
    }

    // render
    // ------
    float alpha = accumulatedTime / timeStep;
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    engineObj.Render(alpha);

    glfwSwapBuffers(window);

    while (GLenum err = glGetError())
      std::cout << "GL ERROR: " << err << std::endl;
  }

  // delete all resources as loaded using the resource manager
  // ---------------------------------------------------------
  ResourceManager::Clear();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mode) {
  // when a user presses the escape key, we set the WindowShouldClose property
  // to true, closing the application
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
  if (key >= 0 && key < 1024) {
    if (action == GLFW_PRESS)
      engineObj.keys[key] = true;
    else if (action == GLFW_RELEASE)
      engineObj.keys[key] = false;
  }
}

Viewport letterbox_viewport(int windowWidth, int windowHeight) {
  float targetAspectRatio = 16.0f / 9.0f;
  float currentAspectRatio = static_cast<float>(windowWidth) / windowHeight;

  if (currentAspectRatio > targetAspectRatio) {
    int newWidth = static_cast<int>(windowHeight * targetAspectRatio);
    int xOffset = (windowWidth - newWidth) / 2;
    glScissor(xOffset, 0, newWidth, windowHeight);
    glViewport(xOffset, 0, newWidth, windowHeight);
    return Viewport{xOffset, 0, newWidth, windowHeight};
  } else {
    int newHeight = static_cast<int>(windowWidth / targetAspectRatio);
    int yOffset = (windowHeight - newHeight) / 2;
    glScissor(0, yOffset, windowWidth, newHeight);
    glViewport(0, yOffset, windowWidth, newHeight);
    return Viewport{0, yOffset, windowWidth, newHeight};
  }
  return Viewport{0, 0, windowWidth, windowHeight};
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width
  // and height will be significantly larger than specified on retina displays.
  engineObj.viewport = letterbox_viewport(width, height);
}

void mouse_button_callback(GLFWwindow *window, int button, int action,
                           int mods) {
  if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
    if (engineObj.clickState == RIGHT_CLICK) {
      engineObj.clickState = BOTH_CLICK;
    } else {
      engineObj.clickState = LEFT_CLICK;
    }
  } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
    if (engineObj.clickState == LEFT_CLICK) {
      engineObj.clickState = BOTH_CLICK;
    } else {
      engineObj.clickState = RIGHT_CLICK;
    }
  }
}

void mouse_callback(GLFWwindow *window, double xposIn, double yposIn) {
  float xpos = static_cast<float>(xposIn);
  float ypos = static_cast<float>(yposIn);

  engineObj.mouseX = xpos;
  engineObj.mouseY = ypos;
}
