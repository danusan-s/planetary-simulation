#ifndef TYPES_H
#define TYPES_H

#include "camera.h"
#include <cmath>
#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <string>

using ShaderID = std::string;
using TextureID = std::string;
using ModelID = std::string;
using SpriteID = uint32_t;
using BodyID = uint32_t;
using ObjectID = uint32_t;
using ParticleID = uint32_t;

constexpr uint32_t INVALID_ID = UINT32_MAX;
constexpr int MAX_TRAIL = 512;
constexpr int MAX_OBJECTS = 512;
constexpr int MAX_PARTICLES = 5192;
constexpr int MAX_COLLISIONS = 256;

// GPU-side body layout (std430 — 48 bytes, no padding needed).
// Indexed by ObjectID; both compute shaders and vertex shaders read this.
struct GPUBody {
  glm::vec3 position; // 12
  float mass;         //  4
  glm::vec3 velocity; // 12
  float radius;       //  4
  glm::vec3 scale;    // 12
  float alive;        //  4  (1.0 = alive, 0.0 = dead)
};

// GPU-side particle layout (std430 — 48 bytes, no padding needed).
struct GPUParticle {
  glm::vec3 position; // 12
  float lifetime;     //  4
  glm::vec3 velocity; // 12
  float elapsedTime;  //  4
  glm::vec3 scale;    // 12
  float alive;        //  4
};

// Written by the compute shader when a collision is detected.
struct CollisionPair {
  uint32_t survivorIdx;
  uint32_t destroyedIdx;
};

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
  Vec3 scale;

  Transform() : position(Vec3(0.0f)), scale(Vec3(1.0f)) {
  }
};

struct Sprite {
  TextureID textureID;
  ShaderID shaderID;
  ModelID modelID;
  Vec3 color;
  bool active;

  Sprite()
      : textureID(""), shaderID(""), modelID(""), color(Vec3(1.0f, 1.0f, 1.0f)),
        active(false) {
  }
};

struct Body {
  Vec3 velocity;
  float mass;
  float
      radius; // Physics collision/gravity radius (independent of render scale)
  bool active;

  Body() : velocity(Vec3()), mass(1.0f), radius(1.0f), active(false) {
  }
};

struct Particle {
  Transform transform;
  Vec3 velocity;
  Vec3 color;
  float lifetime;
  float elapsedTime;

  bool active;

  Particle()
      : transform(Transform()), velocity(Vec3()), color(Vec3(1.0f)),
        lifetime(0.0f), elapsedTime(0.0f), active(false) {
  }
};

struct Object {
  Transform transform; // CPU mirror — updated only on collision response/spawn
  SpriteID spriteID = INVALID_ID;
  BodyID bodyID = INVALID_ID;
  bool active = false;

  void destroyObj() {
    active = false;
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
