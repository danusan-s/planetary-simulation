#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "object_factory.h"
#include "world.h"

class PhysicsSystem {
public:
  PhysicsSystem();
  ~PhysicsSystem();

  int counter = 0;
  float G = 1.0f;

  void step(World *world, float dt, ObjectFactory *factory);
};

#endif // !PHYSICS_SYSTEM_H
