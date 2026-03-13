#ifndef TYPES_H
#define TYPES_H

#include "camera.h"
#include <cmath>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <string>

using ShaderID = std::string;
using TextureID = std::string;
using ModelID = std::string;
using SpriteID = uint32_t;
using BodyID = uint32_t;
using ObjectID = uint32_t;
using ParticleID = uint32_t;
using SunID = ObjectID;

constexpr uint32_t INVALID_ID = UINT32_MAX;
constexpr int MAX_TRAIL = 512;

struct Vec3 {
  float x;
  float y;
  float z;

  Vec3() : x(0.0f), y(0.0f), z(0.0f) {
  }

  Vec3(float value) : x(value), y(value), z(value) {
  }

  Vec3(float x, float y, float z) : x(x), y(y), z(z) {
  }

  float dot(const Vec3 &other) const {
    return x * other.x + y * other.y + z * other.z;
  }

  Vec3 normalized() const {
    float length = sqrt(x * x + y * y + z * z);
    if (length == 0.0f)
      return Vec3(0.0f);
    return Vec3(x / length, y / length, z / length);
  }

  Vec3 operator+(const Vec3 &other) const {
    return Vec3(x + other.x, y + other.y, z + other.z);
  }

  Vec3 operator+=(const Vec3 &other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
  }

  Vec3 operator-(const Vec3 &other) const {
    return Vec3(x - other.x, y - other.y, z - other.z);
  }

  Vec3 operator-=(const Vec3 &other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
  }

  Vec3 operator*(float scalar) const {
    return Vec3(x * scalar, y * scalar, z * scalar);
  }

  Vec3 operator/(float scalar) {
    if (scalar == 0.0f)
      return Vec3(0.0f);
    return Vec3(x / scalar, y / scalar, z / scalar);
  }

  operator glm::vec3() const {
    return glm::vec3(x, y, z);
  }
};

struct Transform {
  Vec3 position;
  float radius;

  Transform() : position(Vec3(0.0f, 0.0f, 0.0f)), radius(1.0f) {
  }
};

struct Sprite {
  TextureID textureID;
  ShaderID shaderID;
  ModelID modelID;
  Vec3 color;

  Sprite()
      : textureID(""), shaderID(""), modelID(""),
        color(Vec3(1.0f, 1.0f, 1.0f)) {
  }
};

struct Body {
  Vec3 velocity;
  float mass;

  Body() : velocity(Vec3()), mass(1.0f) {
  }
};

struct Particle {
  Vec3 position;
  Vec3 velocity;
  float lifetime;
  float size;
  SpriteID spriteID;

  bool active;

  Particle()
      : position(Vec3()), velocity(Vec3()), lifetime(0.0f), size(1.0f),
        spriteID(INVALID_ID), active(false) {
  }
};

struct Object {
  Transform transform;
  Vec3 trail[MAX_TRAIL];
  int trailHead = 0;
  GLuint trailVAO, trailVBO = 0;
  SpriteID spriteID = INVALID_ID;
  BodyID bodyID = INVALID_ID;

  bool active = false;

  void updateTrail(const Vec3 &newPosition) {
    trailHead = (trailHead + 1) % MAX_TRAIL;
    trail[trailHead] = newPosition;
    glBindBuffer(GL_ARRAY_BUFFER, trailVBO);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vec3) * trailHead, sizeof(Vec3),
                    &trail[trailHead]);
  }
};

struct Viewport {
  int x;
  int y;
  int width;
  int height;
};

struct InputState {
  bool keys[1024];
  bool mouseButtons[8];
  bool cursorLocked = true;
  float mouseX, mouseY;
  float lastMouseX, lastMouseY;

  InputState()
      : mouseX(0.0f), mouseY(0.0f), lastMouseX(0.0f), lastMouseY(0.0f),
        cursorLocked(true) {
    for (int i = 0; i < 1024; ++i) {
      keys[i] = false;
    }
    for (int i = 0; i < 8; ++i) {
      mouseButtons[i] = false;
    }
  }
};

#endif // !TYPES_H
