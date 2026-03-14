#include "world.h"
#include "types.h"
#include <cstring>

World::World() {
  camera = Camera(glm::vec3(0.0f, 0.0f, 50.0f));
  sunID = INVALID_ID;
  sprites.reserve(MAX_OBJECTS);
  bodies.reserve(MAX_OBJECTS);
  objects.reserve(MAX_OBJECTS);
  particles.reserve(MAX_PARTICLES);

  std::memset(trailHeads, 0, sizeof(trailHeads));

  // --- SSBOobjA / SSBOobjB: ping-pong GPUBody buffers ---
  glGenBuffers(1, &SSBOobjA);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOobjA);
  glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_OBJECTS * sizeof(GPUBody), nullptr,
               GL_DYNAMIC_DRAW);

  glGenBuffers(1, &SSBOobjB);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOobjB);
  glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_OBJECTS * sizeof(GPUBody), nullptr,
               GL_DYNAMIC_DRAW);

  // --- SSBOtrail: flat vec4 array [MAX_OBJECTS * MAX_TRAIL] ---
  // Each vec4 stores (x, y, z, 0.0). Using vec4 keeps std430 alignment clean.
  glGenBuffers(1, &SSBOtrail);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOtrail);
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               MAX_OBJECTS * MAX_TRAIL * sizeof(glm::vec4), nullptr,
               GL_DYNAMIC_DRAW);

  // --- SSBOcollisions: uint count + CollisionPair[MAX_COLLISIONS] ---
  glGenBuffers(1, &SSBOcollisions);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOcollisions);
  // Layout: [uint count][pad pad pad][CollisionPair * MAX_COLLISIONS]
  // We put count + 3 padding uints at the front for std430 alignment.
  glBufferData(GL_SHADER_STORAGE_BUFFER,
               sizeof(uint32_t) * 4 + MAX_COLLISIONS * sizeof(CollisionPair),
               nullptr, GL_DYNAMIC_DRAW);
  // Zero out the count
  const uint32_t zero = 0;
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &zero);

  // --- SSBOparticles: GPUParticle[MAX_PARTICLES] ---
  glGenBuffers(1, &SSBOparticles);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBOparticles);
  glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_PARTICLES * sizeof(GPUParticle),
               nullptr, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

World::~World() {
  for (size_t i = 0; i < objects.size(); i++) {
    if (objects[i].active)
      DestroyObject(static_cast<ObjectID>(i));
  }
  glDeleteBuffers(1, &SSBOobjA);
  glDeleteBuffers(1, &SSBOobjB);
  glDeleteBuffers(1, &SSBOtrail);
  glDeleteBuffers(1, &SSBOcollisions);
  glDeleteBuffers(1, &SSBOparticles);
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
