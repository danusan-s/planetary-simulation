#include "world.h"
#include "types.h"

World::World() {
  camera = Camera(glm::vec3(0.0f, 0.0f, 50.0f));
  sunID = INVALID_ID;
  sprites.reserve(512);
  bodies.reserve(512);
  objects.reserve(512);
  particles.reserve(5192);
}

World::~World() {
  for (size_t i = 0; i < objects.size(); i++) {
    if (objects[i].active)
      DestroyObject(static_cast<ObjectID>(i));
  }
}

ObjectID World::CreateObject() {
  Object obj;
  obj.active = true;

  if (!freeObjects.empty()) {
    ObjectID id = freeObjects.back();
    freeObjects.pop_back();
    objects[id] = obj;
    return id;
  }

  objects.push_back(obj);
  return static_cast<ObjectID>(objects.size() - 1);
}

void World::DestroyObject(ObjectID id) {
  if (id >= objects.size())
    return;

  Object &obj = objects[id];
  if (!obj.active)
    return;

  // Return associated Sprite and Body slots to their free lists
  if (obj.spriteID != INVALID_ID && obj.spriteID < sprites.size()) {
    sprites[obj.spriteID].active = false;
    freeSprites.push_back(obj.spriteID);
  }
  if (obj.bodyID != INVALID_ID && obj.bodyID < bodies.size()) {
    bodies[obj.bodyID].active = false;
    freeBodies.push_back(obj.bodyID);
  }

  obj.destroyObj(); // frees trail GPU buffers, sets active = false
  obj.spriteID = INVALID_ID;
  obj.bodyID = INVALID_ID;
  freeObjects.push_back(id);
}

SpriteID World::AddSprite(const Sprite &sprite) {
  Sprite s = sprite;
  s.active = true;

  if (!freeSprites.empty()) {
    SpriteID id = freeSprites.back();
    freeSprites.pop_back();
    sprites[id] = s;
    return id;
  }

  sprites.push_back(s);
  return static_cast<SpriteID>(sprites.size() - 1);
}

BodyID World::AddBody(const Body &body) {
  Body b = body;
  b.active = true;

  if (!freeBodies.empty()) {
    BodyID id = freeBodies.back();
    freeBodies.pop_back();
    bodies[id] = b;
    return id;
  }

  bodies.push_back(b);
  return static_cast<BodyID>(bodies.size() - 1);
}

ParticleID World::AddParticle(const Particle &particle) {
  if (!freeParticles.empty()) {
    ParticleID id = freeParticles.back();
    freeParticles.pop_back();
    particles[id] = particle;
    return id;
  }

  particles.push_back(particle);
  return static_cast<ParticleID>(particles.size() - 1);
}

void World::DeactivateParticle(ParticleID id) {
  if (id >= particles.size())
    return;
  if (!particles[id].active)
    return;

  particles[id].active = false;
  freeParticles.push_back(id);
}
