#include "object_factory.h"

ObjectFactory::ObjectFactory(World *world) {
  this->world = world;
}

Object &ObjectFactory::spawnPlanet(Vec3 position, float radius, float mass,
                                   Vec3 initialSpeed, Vec3 color) {
  Object &obj = this->world->CreateObject();

  Sprite sprite = Sprite();
  sprite.modelID = "sphere";
  sprite.textureID = "solid";
  sprite.shaderID = "diffuse";
  sprite.color = color;
  obj.spriteID = this->world->AddSprite(sprite);

  Body body = Body();
  body.mass = mass;
  body.velocity = initialSpeed;
  obj.bodyID = this->world->AddBody(body);

  obj.transform.position = position;
  obj.transform.radius = radius;

  return obj;
}
