#include "object_factory.h"
#include "camera.h"
#include "resource_manager.h"
#include "types.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

ObjectFactory::ObjectFactory(World *world) {
  this->world = world;
}

void setupTrailRenderer(Object &obj) {
  glGenVertexArrays(1, &obj.trailVAO);
  glGenBuffers(1, &obj.trailVBO);

  glBindVertexArray(obj.trailVAO);

  glBindBuffer(GL_ARRAY_BUFFER, obj.trailVBO);
  glBufferData(GL_ARRAY_BUFFER, MAX_TRAIL * sizeof(Vec3), obj.trail,
               GL_DYNAMIC_DRAW);

  // Position (location = 0)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void *)0);

  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

float randomFloat(float min, float max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(min, max);
  return dis(gen);
}

void ObjectFactory::generateRandomPlanet() {
  float mass = randomFloat(this->minPlanetMass, this->maxPlanetMass);
  // Simple heuristic for radius: r ~ cbrt(mass)
  float radius = std::cbrt(mass) * this->radiusScale;

  Vec3 position(randomFloat(-this->positionRange, this->positionRange),
                randomFloat(-this->positionRange, this->positionRange),
                randomFloat(-this->positionRange, this->positionRange));

  Vec3 velocity(randomFloat(-this->velocityRange, this->velocityRange),
                randomFloat(-this->velocityRange, this->velocityRange),
                randomFloat(-this->velocityRange, this->velocityRange));

  // Bright colors
  Vec3 color(randomFloat(0.5f, 1.0f), randomFloat(0.5f, 1.0f),
             randomFloat(0.5f, 1.0f));

  spawnPlanet(position, radius, mass, velocity, color, "solid");
}

void ObjectFactory::generateRandomSystem(int numPlanets) {
  spawnSun(Vec3(0.0f), 2.0f, 10000.0f, Vec3(5.0f, 0.0f, 0.0f),
           Vec3(1.0f, 1.0f, 0.5f));
  for (int i = 0; i < numPlanets; ++i) {
    generateRandomPlanet();
  }
}

float ObjectFactory::parsePreset(const char *filePath) {
  float G = 1.0f; // Default gravitational constant

  std::ifstream presetFile(filePath);
  if (!presetFile.is_open()) {
    std::cerr << "Failed to open preset file: " << filePath << std::endl;
    return G;
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
          iss >> G;
          std::cout << "Set G constant to: " << G << std::endl;
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
        spawnSun(Vec3(posX, posY, posZ), radius, mass, Vec3(velX, velY, velZ),
                 Vec3(colorR, colorG, colorB));
      else {
        if (!ResourceManager::TextureExists(type)) {
          std::cerr << "Texture for type '" << type
                    << "' not found. Using default texture." << std::endl;
          type = "solid"; // Fallback to a default texture
        }
        spawnPlanet(Vec3(posX, posY, posZ), radius, mass,
                    Vec3(velX, velY, velZ), Vec3(colorR, colorG, colorB), type);
      }
    }
  }
  return G;
}

ObjectID ObjectFactory::spawnPlanet(Vec3 position, float radius, float mass,
                                    Vec3 initialSpeed, Vec3 color,
                                    std::string textureID) {
  ObjectID objID = this->world->CreateObject();
  Object &obj = this->world->objects[objID];

  Sprite sprite = Sprite();
  sprite.modelID = "sphere";
  sprite.textureID = textureID;
  sprite.shaderID = "diffuse";
  sprite.color = color;
  obj.spriteID = this->world->AddSprite(sprite);

  Body body = Body();
  body.mass = mass;
  body.velocity = initialSpeed;
  obj.bodyID = this->world->AddBody(body);

  obj.transform.position = position;
  obj.transform.radius = radius;
  for (int i = 0; i < MAX_TRAIL; i++) {
    obj.trail[i] = position;
  }

  setupTrailRenderer(obj);

  return objID;
}

ObjectID ObjectFactory::spawnSun(Vec3 position, float radius, float mass,
                                 Vec3 initialSpeed, Vec3 color) {
  ObjectID objID = this->world->CreateObject();
  Object &obj = this->world->objects[objID];
  this->world->sunID = objID;

  Sprite sprite = Sprite();
  sprite.modelID = "sphere";
  sprite.textureID = "sun";
  sprite.shaderID = "sun";
  sprite.color = color;
  obj.spriteID = this->world->AddSprite(sprite);

  Body body = Body();
  body.mass = mass;
  body.velocity = initialSpeed;
  obj.bodyID = this->world->AddBody(body);

  obj.transform.position = position;
  obj.transform.radius = radius;
  for (int i = 0; i < MAX_TRAIL; i++) {
    obj.trail[i] = position;
  }

  setupTrailRenderer(obj);

  return objID;
}

ParticleID ObjectFactory::spawnParticle(Vec3 position, Vec3 velocity,
                                        float lifetime, float size,
                                        SpriteID spriteID) {
  Particle particle;
  particle.position = position;
  particle.velocity = velocity;
  particle.elapsedTime = 0.0f;
  particle.lifetime = lifetime;
  particle.size = size;
  particle.spriteID = spriteID;
  particle.active = true;

  return this->world->AddParticle(particle);
}

void ObjectFactory::spawnExplosion(Vec3 origin, Vec3 normal, Object &obj,
                                   int count) {

  Sprite explosionSprite;
  explosionSprite.modelID = "cube";
  explosionSprite.textureID = "solid";
  explosionSprite.shaderID = "diffuse";
  explosionSprite.color = this->world->sprites[obj.spriteID]
                              .color; // Use the same color as the object
  SpriteID explosionSpriteID = this->world->AddSprite(explosionSprite);

  float size = obj.transform.radius * 0.1f;
  float lifetime = 5.0f;

  Body &body = this->world->bodies[obj.bodyID];
  Vec3 objVel = body.velocity;
  float speed = std::sqrt(objVel.x * objVel.x + objVel.y * objVel.y +
                          objVel.z * objVel.z);

  for (int i = 0; i < count; i++) {
    Vec3 randomVec(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f),
                   randomFloat(-1.0f, 1.0f));

    // Project random vector onto the plane orthogonal to the impact normal
    Vec3 perp = randomVec - normal * normal.dot(randomVec);

    // Normalize and add a tiny bit of spherical randomness to give the disk
    // some thickness
    Vec3 randDir = (perp.normalized() + randomVec * 0.15f).normalized();

    // Base velocity is a fraction of the original object's velocity to carry
    // momentum
    Vec3 baseVel = objVel * randomFloat(0.0f, 0.3f);

    // The main explosive force spreads radially outward from the impact normal
    float outwardSpeed = speed * randomFloat(0.3f, 0.6f);

    Vec3 velocity = baseVel + randDir * outwardSpeed;

    float variation = randomFloat(0.5f, 1.5f);
    float finalLifetime = lifetime * variation;
    float finalSize = size * variation;

    spawnParticle(origin, velocity, finalLifetime, finalSize,
                  explosionSpriteID);
  }
}
