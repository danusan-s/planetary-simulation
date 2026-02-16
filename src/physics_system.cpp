#include "physics_system.h"
#include "types.h"
#include <glm/ext/quaternion_geometric.hpp>

PhysicsSystem::PhysicsSystem() {
}

PhysicsSystem::~PhysicsSystem() {
}

void PhysicsSystem::step(World &world, float dt) {
  for (auto &obj : world.objects) {
    if (obj.bodyID == INVALID_ID || obj.bodyID >= world.bodies.size())
      continue;

    obj.previousTransform = obj.transform;
    Body &body = world.bodies[obj.bodyID];

    obj.transform.position += body.velocity * dt;
  }
}
