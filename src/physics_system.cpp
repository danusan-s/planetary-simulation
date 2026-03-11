#include "physics_system.h"
#include "types.h"
#include <glm/ext/quaternion_geometric.hpp>

PhysicsSystem::PhysicsSystem() : G(1.0f) {
}

PhysicsSystem::~PhysicsSystem() {
}

const float epsilon = 0.0001f;

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

      float dist_sq = delta.dot(delta) + epsilon;
      Vec3 direction = delta.normalized();

      bodyA.velocity += direction * (G * bodyB.mass / dist_sq) * dt;
      bodyB.velocity -= direction * (G * bodyA.mass / dist_sq) * dt;
    }
  }

  for (auto &obj : world->objects) {
    if (obj.bodyID == INVALID_ID || obj.bodyID >= world->bodies.size())
      continue;

    Body &body = world->bodies[obj.bodyID];

    obj.transform.position += body.velocity * dt;

    float speed_sq = body.velocity.dot(body.velocity);
    float speed = std::sqrt(speed_sq);

    // Avoid division by zero
    if (speed < 0.1f)
      speed = 0.1f;

    // Inverse relationship:
    // Higher speed -> lower sample rate (more frequent sampling)
    int dynamicSampleRate = static_cast<int>(30.0f / speed);

    if (dynamicSampleRate < 1)
      dynamicSampleRate = 1;

    if (counter % dynamicSampleRate == 0)
      obj.updateTrail(obj.transform.position);
  }
  ++counter;
}
