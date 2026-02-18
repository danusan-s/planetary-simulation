#include "world.h"
#include "types.h"

World::World() {
  camera = Camera();
  sunID = INVALID_ID;
}

ObjectID World::CreateObject() {
  objects.emplace_back();
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
