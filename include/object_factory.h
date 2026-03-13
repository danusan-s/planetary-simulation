#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "world.h"

class ObjectFactory {
private:
  World *world;

  float minPlanetMass = 1.0f;
  float maxPlanetMass = 100.0f;
  float velocityRange = 5.0f;
  float positionRange = 100.0f;
  float radiusScale = 0.2f;

public:
  ObjectFactory(World *world);
  void generateRandomPlanet();
  void generateRandomSystem(int numPlanets);
  float parsePreset(const char *filePath);
  ObjectID spawnPlanet(Vec3 position, float radius, float mass,
                       Vec3 initialSpeed, Vec3 color, std::string textureID);
  ObjectID spawnSun(Vec3 position, float radius, float mass, Vec3 initialSpeed,
                    Vec3 color);
};

#endif // !OBJECT_FACTORY_H
