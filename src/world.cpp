#include "world.h"
#include "types.h"

World::World() {
}

Object &World::CreateObject() {
  objects.emplace_back();
  return objects.back();
}

SpriteID World::AddSprite(const Sprite &sprite) {
  sprites.push_back(sprite);
  return static_cast<SpriteID>(sprites.size() - 1);
}

BodyID World::AddBody(const Body &body) {
  bodies.push_back(body);
  return static_cast<BodyID>(bodies.size() - 1);
}

void World::Init() {
  this->camera = new Camera();
}
