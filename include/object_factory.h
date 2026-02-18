#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "world.h"

class ObjectFactory {
private:
  World *world;

public:
  ObjectFactory(World *world);
  ObjectID spawnPlanet(Vec3 position, float radius, float mass,
                      Vec3 initialSpeed, Vec3 color);
  ObjectID spawnSun(Vec3 position, float radius, float mass, Vec3 initialSpeed,
                   Vec3 color);
};

#endif // !OBJECT_FACTORY_H
