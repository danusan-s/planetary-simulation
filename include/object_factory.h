#ifndef OBJECT_FACTORY_H
#define OBJECT_FACTORY_H

#include "world.h"

class ObjectFactory {
private:
  World *world;

public:
  ObjectFactory(World &world);
  Object &spawnPlanet(Vec3 position, float radius, Vec3 color, float mass,
                      Vec3 initialSpeed);
};

#endif // !OBJECT_FACTORY_H
