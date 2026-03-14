#ifndef WORLD_H
#define WORLD_H

#include "camera.h"
#include "types.h"
#include <vector>

class World {
public:
  World();
  ~World();
  Camera camera;
  std::vector<Object> objects;
  std::vector<Sprite> sprites;
  std::vector<Body> bodies;
  std::vector<Particle> particles;
  ObjectID sunID; // Index of the sun object; INVALID_ID if no sun exists

  // Ping-pong body buffers: compute reads A, writes B; swapped each frame.
  GLuint SSBOobjA;       // binding 0: GPUBody[MAX_OBJECTS]
  GLuint SSBOobjB;       // binding 1: GPUBody[MAX_OBJECTS]
  // Flat trail storage: vec4[MAX_OBJECTS * MAX_TRAIL]
  // Slot = objectIdx * MAX_TRAIL + trailSlot  (vec4: xyz=pos, w=unused)
  GLuint SSBOtrail;      // binding 3: vec4[MAX_OBJECTS * MAX_TRAIL]
  // Collision output: uint count + CollisionPair[MAX_COLLISIONS]
  GLuint SSBOcollisions; // binding 2
  // Particle buffer
  GLuint SSBOparticles;  // GPUParticle[MAX_PARTICLES]

  // Per-object trail write head (ring buffer index), kept on CPU so the
  // compute shader can receive it as a uniform without a readback.
  int trailHeads[MAX_OBJECTS];

  uint32_t nextObjectID;

  ObjectID CreateObject();
  void DestroyObject(ObjectID id);
  SpriteID AddSprite(const Sprite &sprite);
  BodyID AddBody(const Body &body);
  ParticleID AddParticle(const Particle &particle);
  void DeactivateParticle(ParticleID id);

private:
  // Free-lists for O(1) slot reuse.  Destroyed/deactivated indices are pushed
  // here and popped on the next Create/Add call, avoiding linear scans.
  std::vector<ObjectID> freeObjects;
  std::vector<SpriteID> freeSprites;
  std::vector<BodyID> freeBodies;
  std::vector<ParticleID> freeParticles;
};

#endif // !WORLD_H
