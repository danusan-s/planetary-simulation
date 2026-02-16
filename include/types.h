#ifndef TYPES_H
#define TYPES_H

#include <cstdint>
#include <glm/ext/vector_float3.hpp>
#include <string>

using ShaderID = std::string;
using TextureID = std::string;
using ModelID = std::string;
using SpriteID = uint32_t;
using BodyID = uint32_t;

constexpr uint32_t INVALID_ID = UINT32_MAX;

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

  Sprite() : textureID(""), shaderID(""), color(Vec3(1.0f, 1.0f, 1.0f)) {
  }
};

struct Body {
  Vec3 velocity;
  float mass;
  float invMass;

  Body() : velocity(Vec3()), mass(1.0f), invMass(1.0f) {
  }
};

struct Object {
  Transform transform = Transform();
  Transform previousTransform = Transform();
  SpriteID spriteID = INVALID_ID;
  BodyID bodyID = INVALID_ID;
};

#endif // !TYPES_H
