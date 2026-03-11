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

  void spawnButton(float x, float y, float width, float height,
                   const std::string &label, std::function<void()> onClick);
};

#endif // !OBJECT_FACTORY_H
