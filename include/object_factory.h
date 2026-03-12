#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "world.h"

class ObjectFactory {
private:
  World *world;

public:
  ObjectFactory(World *world);
  void generateRandomPlanets(int n);
  float parsePreset(const char *filePath);
  ObjectID spawnPlanet(Vec3 position, float radius, float mass,
                       Vec3 initialSpeed, Vec3 color, std::string textureID);
  ObjectID spawnSun(Vec3 position, float radius, float mass, Vec3 initialSpeed,
                    Vec3 color);
};

#endif // !OBJECT_FACTORY_H
