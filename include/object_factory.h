#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "world.h"

class ObjectFactory {
public:
  // Random generation parameters (configurable from menu)
  float minPlanetMass = 1.0f;
  float maxPlanetMass = 1000.0f;
  float velocityRange = 5.0f;
  float positionRange = 200.0f;
  float radiusScale = 0.2f;
  float colorMin = 0.5f;
  float colorMax = 1.0f;

  ObjectFactory(World *world);

  void generateRandomPlanet();
  void generateRandomSystem(int numPlanets);
  float parsePreset(const char *filePath);
  ObjectID spawnPlanet(Vec3 position, float radius, float mass,
                       Vec3 initialSpeed, Vec3 color, std::string textureID);
  ObjectID spawnSun(Vec3 position, float radius, float mass, Vec3 initialSpeed,
                    Vec3 color);
  void spawnExplosion(Vec3 origin, Vec3 normal, Object &obj, int count);

private:
  World *world;
  ParticleID spawnParticle(Vec3 position, Vec3 velocity, float lifetime,
                           float size, Vec3 color);
};

#endif // !OBJECT_FACTORY_H
