#include "world.h"
#include "types.h"

World::World() {
  camera = Camera();
  sunID = INVALID_ID;
  sprites.reserve(512);
  bodies.reserve(512);
  objects.reserve(512);
  particles.reserve(1024);
}

World::~World() {
  for (auto &obj : objects) {
    obj.destroyObj();
  }
}

ObjectID World::CreateObject() {
  Object obj;
  obj.active = true;

  // If close to capacity, we can reuse inactive objects to avoid unnecessary
  // allocations
  if (objects.size() >= objects.capacity() * 0.9) {
    for (size_t i = 0; i < objects.size(); i++) {
      if (!objects[i].active) {
        objects[i].destroyObj(); // free any lingering GPU buffers before reuse
        objects[i] = obj;
        return static_cast<ObjectID>(i);
      }
    }
  }

  objects.push_back(obj);
  return static_cast<ObjectID>(objects.size() - 1);
}

SpriteID World::AddSprite(const Sprite &sprite) {
  sprites.push_back(sprite);
  return static_cast<SpriteID>(sprites.size() - 1);
}

BodyID World::AddBody(const Body &body) {
  bodies.push_back(body);
  return static_cast<BodyID>(bodies.size() - 1);
}

ParticleID World::AddParticle(const Particle &particle) {
  // Particles often have short lifetimes, so we can reuse inactive particles to
  // avoid unnecessary allocations
  for (size_t i = 0; i < particles.size(); i++) {
    if (!particles[i].active) {
      particles[i] = particle;
      return static_cast<ParticleID>(i);
    }
  }

  particles.push_back(particle);
  return static_cast<ParticleID>(particles.size() - 1);
}
