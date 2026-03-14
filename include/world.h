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
