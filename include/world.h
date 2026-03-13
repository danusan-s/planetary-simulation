#ifndef OBJECT_MANAGER_H
#define OBJECT_MANAGER_H

#include "camera.h"
#include "types.h"
#include <vector>

class World {
public:
  World();
  Camera camera;
  std::vector<Object> objects;
  std::vector<Sprite> sprites;
  std::vector<Body> bodies;
  std::vector<Particle> particles;
  SunID sunID; // Pointer to the sun object for easy access

  ObjectID CreateObject();
  SpriteID AddSprite(const Sprite &sprite);
  BodyID AddBody(const Body &body);
  ParticleID AddParticle(const Particle &particle);
};

#endif
