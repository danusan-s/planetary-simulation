#include "physics_system.h"
#include "types.h"
#include <glm/ext/quaternion_geometric.hpp>

// Small value added to distances to prevent division by zero in gravity.
static constexpr float GRAVITY_EPSILON = 0.00001f;

// Seconds after a particle spawns before gravity starts affecting it.
// Prevents newly spawned explosion particles from immediately collapsing back.
static constexpr float PARTICLE_GRAVITY_DELAY = 0.1f;

// Minimum speed (units/s) used when computing the dynamic trail sample rate.
// Prevents division by a near-zero speed producing an absurdly high rate.
static constexpr float MIN_SPEED_FOR_TRAIL = 0.1f;

// Numerator of the dynamic trail sample rate formula: rate = TRAIL_RATE_DIVISOR
// / speed. Higher value -> trail sampled less frequently at a given speed.
static constexpr float TRAIL_RATE_DIVISOR = 20.0f;

// Number of particles spawned per collision explosion.
static constexpr int EXPLOSION_PARTICLE_COUNT = 20;

PhysicsSystem::PhysicsSystem() : G(1.0f) {
}

PhysicsSystem::~PhysicsSystem() {
}

void PhysicsSystem::step(World *world, float dt, ObjectFactory *factory) {
  applyGravityAndCollisions(world, dt, factory);
  stepParticles(world, dt);
  integratePositions(world, dt);
  ++counter;
}

void PhysicsSystem::applyGravityAndCollisions(World *world, float dt,
                                              ObjectFactory *factory) {
  for (size_t i = 0; i < world->objects.size(); i++) {
    Object &objA = world->objects[i];
    if (!objA.active || objA.bodyID == INVALID_ID ||
        objA.bodyID >= world->bodies.size())
      continue;

    Body &bodyA = world->bodies[objA.bodyID];

    for (size_t j = i + 1; j < world->objects.size(); j++) {
      Object &objB = world->objects[j];
      if (!objB.active || objB.bodyID == INVALID_ID ||
          objB.bodyID >= world->bodies.size())
        continue;

      Body &bodyB = world->bodies[objB.bodyID];

      Vec3 delta = objB.transform.position - objA.transform.position;
      float dist_sq = delta.dot(delta) + GRAVITY_EPSILON;
      float dist = std::sqrt(dist_sq);

      if (dist < (bodyA.radius + bodyB.radius)) {
        // Volume is conserved so r1^3 + r2^3 = R^3
        float newRadius =
            std::cbrt(std::pow(bodyA.radius, 3) + std::pow(bodyB.radius, 3));
        Vec3 normal = delta.normalized();
        float totalMass = bodyA.mass + bodyB.mass;
        Vec3 finalVelocity =
            (bodyA.velocity * bodyA.mass + bodyB.velocity * bodyB.mass) /
            totalMass;
        Vec3 finalColor = (world->sprites[objA.spriteID].color * bodyA.mass +
                           world->sprites[objB.spriteID].color * bodyB.mass) /
                          totalMass;

        bool isSunCollision =
            (world->sunID != INVALID_ID) &&
            (static_cast<ObjectID>(i) == world->sunID ||
             static_cast<ObjectID>(j) == world->sunID);
        if (isSunCollision) {
          const Object &sun = world->objects[world->sunID];
          finalColor = world->sprites[sun.spriteID].color;
        }

        if (static_cast<ObjectID>(i) == world->sunID ||
            bodyA.mass >= bodyB.mass) {
          world->sprites[objA.spriteID].color = finalColor;
          bodyA.velocity = finalVelocity;
          bodyA.mass = totalMass;
          bodyA.radius = newRadius;
          objA.transform.scale = Vec3(newRadius);
          factory->spawnExplosion(objA.transform.position + normal * newRadius,
                                  normal, objB, EXPLOSION_PARTICLE_COUNT);
          world->DestroyObject(static_cast<ObjectID>(j));
        } else {
          normal =
              normal * (-1); // Flip normal to point from B to A for explosion
          world->sprites[objB.spriteID].color = finalColor;
          bodyB.velocity = finalVelocity;
          bodyB.mass = totalMass;
          bodyB.radius = newRadius;
          objB.transform.scale = Vec3(newRadius);
          factory->spawnExplosion(objB.transform.position + normal * newRadius,
                                  normal, objA, EXPLOSION_PARTICLE_COUNT);
          world->DestroyObject(static_cast<ObjectID>(i));
          break; // objA is destroyed, stop checking it against other objects
        }
        continue;
      }

      Vec3 direction = delta.normalized();
      bodyA.velocity += direction * (G * bodyB.mass / dist_sq) * dt;
      bodyB.velocity -= direction * (G * bodyA.mass / dist_sq) * dt;
    }
  }
}

void PhysicsSystem::stepParticles(World *world, float dt) {
  for (size_t pi = 0; pi < world->particles.size(); pi++) {
    Particle &particle = world->particles[pi];
    if (!particle.active)
      continue;

    bool absorbed = false;
    if (particle.elapsedTime > PARTICLE_GRAVITY_DELAY) {
      for (const auto &obj : world->objects) {
        if (!obj.active || obj.bodyID == INVALID_ID ||
            obj.bodyID >= world->bodies.size())
          continue;

        const Body &body = world->bodies[obj.bodyID];
        Vec3 delta = obj.transform.position - particle.transform.position;
        float dist_sq = delta.dot(delta) + GRAVITY_EPSILON;
        float dist = std::sqrt(dist_sq);

        if (dist < body.radius) {
          world->DeactivateParticle(static_cast<ParticleID>(pi));
          absorbed = true;
          break;
        }

        Vec3 direction = delta.normalized();
        particle.velocity += direction * (G * body.mass / dist_sq) * dt;
      }

      if (absorbed)
        continue;
    }

    particle.transform.position += particle.velocity * dt;
    particle.elapsedTime += dt;
    if (particle.lifetime - particle.elapsedTime <= 0.0f) {
      world->DeactivateParticle(static_cast<ParticleID>(pi));
    }
  }
}

void PhysicsSystem::integratePositions(World *world, float dt) {
  for (auto &obj : world->objects) {
    if (!obj.active || obj.bodyID == INVALID_ID ||
        obj.bodyID >= world->bodies.size())
      continue;

    Body &body = world->bodies[obj.bodyID];
    obj.transform.position += body.velocity * dt;

    float speed = std::sqrt(body.velocity.dot(body.velocity));

    // Avoid division by zero
    if (speed < MIN_SPEED_FOR_TRAIL)
      speed = MIN_SPEED_FOR_TRAIL;

    // Inverse relationship: higher speed -> lower sample rate (more frequent
    // sampling)
    int dynamicSampleRate = static_cast<int>(TRAIL_RATE_DIVISOR / speed);
    if (dynamicSampleRate < 1)
      dynamicSampleRate = 1;

    if (counter % dynamicSampleRate == 0)
      obj.updateTrail(obj.transform.position);
  }
}
