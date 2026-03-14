#ifndef PHYSICS_SYSTEM_H
#define PHYSICS_SYSTEM_H

#include "object_factory.h"
#include "shader.h"
#include "types.h"
#include "world.h"
#include <vector>

class PhysicsSystem {
public:
  PhysicsSystem();
  ~PhysicsSystem();

  int counter = 0;
  float G = 1.0f;

  void step(World *world, float dt, ObjectFactory *factory);

private:
  // Dispatch the object gravity+integration compute shader, then read back
  // any collision pairs and resolve them on the CPU.
  void dispatchObjects(World *world, float dt);
  void resolveCollisions(World *world, ObjectFactory *factory);

  // Dispatch the particle gravity+integration compute shader.
  void dispatchParticles(World *world, float dt);
};

#endif // !PHYSICS_SYSTEM_H
