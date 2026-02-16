#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "world.h"

class PhysicsSystem {
public:
  PhysicsSystem();
  ~PhysicsSystem();

  void step(World &world, float dt);
};

#endif // !PHYSICS_SYSTEM_H
