#include "physics_system.h"
#include "types.h"
#include <glm/ext/quaternion_geometric.hpp>

PhysicsSystem::PhysicsSystem() {
}

PhysicsSystem::~PhysicsSystem() {
}

const float G = 0.001f;
const int sampleCount = 2;

void PhysicsSystem::step(World *world, float dt) {

  for (size_t i = 0; i < world->objects.size(); i++) {
    Object &objA = world->objects[i];
    if (objA.bodyID == INVALID_ID || objA.bodyID >= world->bodies.size())
      continue;

    Body &bodyA = world->bodies[objA.bodyID];

    Vec3 delVel = Vec3(0.0f);

    for (size_t j = i + 1; j < world->objects.size(); j++) {
      Object &objB = world->objects[j];
      if (objB.bodyID == INVALID_ID || objB.bodyID >= world->bodies.size())
        continue;

      Body &bodyB = world->bodies[objB.bodyID];

      Vec3 delta = objB.transform.position - objA.transform.position;

      float dist = delta.dot(delta);
      Vec3 direction = delta.normalized();

      bodyA.velocity += direction * (G * bodyB.mass / dist);
      bodyB.velocity -= direction * (G * bodyA.mass / dist);
    }
  }

  for (auto &obj : world->objects) {
    if (obj.bodyID == INVALID_ID || obj.bodyID >= world->bodies.size())
      continue;

    Body &body = world->bodies[obj.bodyID];

    obj.transform.position += body.velocity * dt;
    if (counter % sampleCount == 0)
      obj.updateTrail(obj.transform.position);
  }
  ++counter;
}
