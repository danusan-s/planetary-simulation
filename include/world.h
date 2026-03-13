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
  SpriteID AddSprite(const Sprite &sprite);
  BodyID AddBody(const Body &body);
  ParticleID AddParticle(const Particle &particle);
};

#endif // !WORLD_H
