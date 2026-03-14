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

// Writes a GPUBody into SSBOobjA at the slot corresponding to objID,
// and initialises the trail SSBO slice with the spawn position.
static void uploadGPUBody(World *world, ObjectID objID, const GPUBody &gpu) {
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBOobjA);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, objID * sizeof(GPUBody),
                  sizeof(GPUBody), &gpu);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // Fill the entire trail slice with the spawn position so trails don't
  // show spurious lines from the origin on the first frame.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBOtrail);
  glm::vec4 initPos(gpu.position, 0.0f);
  for (int t = 0; t < MAX_TRAIL; t++) {
    GLintptr offset = (objID * MAX_TRAIL + t) * sizeof(glm::vec4);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, sizeof(glm::vec4),
                    &initPos);
  }
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // Reset the CPU trail head
  world->trailHeads[objID] = 0;
}

// Writes a GPUParticle into SSBOparticles at the slot corresponding to
// particleID.
static void uploadGPUParticle(World *world, ParticleID pid,
                              const GPUParticle &gpu) {
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBOparticles);
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, pid * sizeof(GPUParticle),
                  sizeof(GPUParticle), &gpu);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

float randomFloat(float min, float max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(min, max);
  return dis(gen);
}

void ObjectFactory::generateRandomPlanet() {
  float mass = randomFloat(this->minPlanetMass, this->maxPlanetMass);
  float radius = std::cbrt(mass) * this->radiusScale;

  Vec3 position(randomFloat(-this->positionRange, this->positionRange),
                randomFloat(-this->positionRange, this->positionRange),
                randomFloat(-this->positionRange, this->positionRange));

  Vec3 velocity(randomFloat(-this->velocityRange, this->velocityRange),
                randomFloat(-this->velocityRange, this->velocityRange),
                randomFloat(-this->velocityRange, this->velocityRange));

  // Bright colors
  Vec3 color(randomFloat(this->colorMin, this->colorMax),
             randomFloat(this->colorMin, this->colorMax),
             randomFloat(this->colorMin, this->colorMax));

  spawnPlanet(position, radius, mass, velocity, color, "solid");
}

void ObjectFactory::generateRandomSystem(int numPlanets) {
  // spawnSun(Vec3(0.0f), 2.0f, 5000.0f, Vec3(0.0f, 0.0f, 0.0f),
  //          Vec3(1.0f, 1.0f, 1.0f));
  for (int i = 0; i < numPlanets; ++i) {
    generateRandomPlanet();
  }
}

float ObjectFactory::parsePreset(const char *filePath) {
  float G = 1.0f;

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
          break;
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        if (prefix == "G") {
          iss >> G;
          std::cout << "Set G constant to: " << G << std::endl;
        }
      }
    } else {
      std::string type = line;
      if (type.empty())
        continue;

      std::cout << "Parsing object definition: " << type << std::endl;
      float posX = 0, posY = 0, posZ = 0, radius = 1, mass = 1, velX = 0,
            velY = 0, velZ = 0, colorR = 1, colorG = 1, colorB = 1;

      while (std::getline(presetFile, line)) {
        if (line.empty())
          break;
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        if (prefix == "pos")
          iss >> posX >> posY >> posZ;
        else if (prefix == "radius")
          iss >> radius;
        else if (prefix == "mass")
          iss >> mass;
        else if (prefix == "vel")
          iss >> velX >> velY >> velZ;
        else if (prefix == "color")
          iss >> colorR >> colorG >> colorB;
      }

      if (type == "sun")
        spawnSun(Vec3(posX, posY, posZ), radius, mass, Vec3(velX, velY, velZ),
                 Vec3(colorR, colorG, colorB));
      else {
        if (!ResourceManager::TextureExists(type)) {
          std::cerr << "Texture '" << type << "' not found, using default.\n";
          type = "solid";
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

  Sprite sprite;
  sprite.modelID = "sphere";
  sprite.textureID = textureID;
  sprite.shaderID = "diffuse";
  sprite.color = color;
  obj.spriteID = this->world->AddSprite(sprite);

  Body body;
  body.mass = mass;
  body.radius = radius;
  body.velocity = initialSpeed;
  obj.bodyID = this->world->AddBody(body);

  obj.transform.position = position;
  obj.transform.scale = Vec3(radius);

  GPUBody gpu;
  gpu.position = glm::vec3(position.x, position.y, position.z);
  gpu.mass = mass;
  gpu.velocity = glm::vec3(initialSpeed.x, initialSpeed.y, initialSpeed.z);
  gpu.radius = radius;
  gpu.scale = glm::vec3(radius);
  gpu.alive = 1.0f;
  uploadGPUBody(this->world, objID, gpu);

  return objID;
}

ObjectID ObjectFactory::spawnSun(Vec3 position, float radius, float mass,
                                 Vec3 initialSpeed, Vec3 color) {
  ObjectID objID = this->world->CreateObject();
  Object &obj = this->world->objects[objID];
  this->world->sunID = objID;

  Sprite sprite;
  sprite.modelID = "sphere";
  sprite.textureID = "sun";
  sprite.shaderID = "sun";
  sprite.color = color;
  obj.spriteID = this->world->AddSprite(sprite);

  Body body;
  body.mass = mass;
  body.radius = radius;
  body.velocity = initialSpeed;
  obj.bodyID = this->world->AddBody(body);

  obj.transform.position = position;
  obj.transform.scale = Vec3(radius);

  GPUBody gpu;
  gpu.position = glm::vec3(position.x, position.y, position.z);
  gpu.mass = mass;
  gpu.velocity = glm::vec3(initialSpeed.x, initialSpeed.y, initialSpeed.z);
  gpu.radius = radius;
  gpu.scale = glm::vec3(radius);
  gpu.alive = 1.0f;
  uploadGPUBody(this->world, objID, gpu);

  return objID;
}

ParticleID ObjectFactory::spawnParticle(Vec3 position, Vec3 velocity,
                                        float lifetime, float size,
                                        Vec3 color) {
  Particle particle;
  particle.transform.position = position;
  particle.transform.scale = Vec3(size);
  particle.velocity = velocity;
  particle.elapsedTime = 0.0f;
  particle.lifetime = lifetime;
  particle.color = color;
  particle.active = true;

  ParticleID pid = this->world->AddParticle(particle);

  GPUParticle gpu;
  gpu.position = glm::vec3(position.x, position.y, position.z);
  gpu.lifetime = lifetime;
  gpu.velocity = glm::vec3(velocity.x, velocity.y, velocity.z);
  gpu.elapsedTime = 0.0f;
  gpu.scale = glm::vec3(size);
  gpu.alive = 1.0f;
  uploadGPUParticle(this->world, pid, gpu);

  return pid;
}

void ObjectFactory::spawnExplosion(Vec3 origin, Vec3 normal, Object &obj,
                                   int count) {

  Vec3 color = this->world->sprites[obj.spriteID].color;

  Body &body = this->world->bodies[obj.bodyID];
  static constexpr float MAX_PARTICLE_SIZE = 0.5f;
  float size = std::min(body.radius * 0.2f, MAX_PARTICLE_SIZE);
  float lifetime = 10.0f;
  Vec3 objVel = body.velocity;
  float speed = std::sqrt(objVel.x * objVel.x + objVel.y * objVel.y +
                          objVel.z * objVel.z);

  for (int i = 0; i < count; i++) {
    Vec3 randomVec(randomFloat(-1.0f, 1.0f), randomFloat(-1.0f, 1.0f),
                   randomFloat(-1.0f, 1.0f));

    Vec3 perp = randomVec - normal * normal.dot(randomVec);
    Vec3 randDir = (perp.normalized() + randomVec * 0.15f).normalized();
    Vec3 baseVel = objVel * randomFloat(0.0f, 0.3f);

    float outwardSpeed = speed * randomFloat(0.3f, 0.6f);
    Vec3 velocity = baseVel + randDir * outwardSpeed;

    float variation = randomFloat(0.5f, 1.5f);
    float finalLifetime = lifetime * variation;
    float finalSize = size * variation;

    spawnParticle(origin, velocity, finalLifetime, finalSize, color);
  }
}
