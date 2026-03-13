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

private:
  // Applies pairwise gravitational forces and resolves inelastic merge
  // collisions between all active objects.
  void applyGravityAndCollisions(World *world, float dt, ObjectFactory *factory);

  // Applies gravity from objects onto particles, advances particle positions,
  // and expires particles that have exceeded their lifetime.
  void stepParticles(World *world, float dt);

  // Integrates object positions forward by dt and samples trail points.
  void integratePositions(World *world, float dt);
};

#endif // !PHYSICS_SYSTEM_H
