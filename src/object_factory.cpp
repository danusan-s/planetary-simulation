#include "object_factory.h"

ObjectFactory::ObjectFactory(World &world) {
  this->world = &world;
}

Object &ObjectFactory::spawnPlanet(Vec3 position, float radius, Vec3 color,
                                   float mass, Vec3 initialSpeed) {
  Object &obj = this->world->CreateObject();

  Sprite sprite = Sprite();
  sprite.modelID = "sphere";
  sprite.textureID = "planet";
  sprite.shaderID = "diffuse";
  SpriteID spriteID = this->world->AddSprite(sprite);

  Body body = Body();

  body.mass = mass;
  body.velocity = initialSpeed;

  obj.bodyID = this->world->AddBody(body);

  obj.transform.position = position;
  obj.transform.radius = radius;

  return obj;
}
