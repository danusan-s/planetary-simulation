#include "object_factory.h"
#include "camera.h"
#include "types.h"

ObjectFactory::ObjectFactory(World *world) {
  this->world = world;
}

void setupTrailRenderer(Object &obj) {
  glGenVertexArrays(1, &obj.trailVAO);
  glGenBuffers(1, &obj.trailVBO);

  glBindVertexArray(obj.trailVAO);

  glBindBuffer(GL_ARRAY_BUFFER, obj.trailVBO);
  glBufferData(GL_ARRAY_BUFFER, MAX_TRAIL * sizeof(Vec3), nullptr,
               GL_DYNAMIC_DRAW);

  // Position (location = 0)
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void *)0);

  glEnableVertexAttribArray(0);

  glBindVertexArray(0);
}

ObjectID ObjectFactory::spawnPlanet(Vec3 position, float radius, float mass,
                                    Vec3 initialSpeed, Vec3 color) {
  ObjectID objID = this->world->CreateObject();
  Object &obj = this->world->objects[objID];

  Sprite sprite = Sprite();
  sprite.modelID = "sphere";
  sprite.textureID = "earth";
  sprite.shaderID = "diffuse";
  sprite.color = color;
  obj.spriteID = this->world->AddSprite(sprite);

  Body body = Body();
  body.mass = mass;
  body.velocity = initialSpeed;
  obj.bodyID = this->world->AddBody(body);

  obj.transform.position = position;
  obj.transform.radius = radius;
  for (int i = 0; i < MAX_TRAIL; i++) {
    obj.trail[i] = position;
  }

  setupTrailRenderer(obj);

  return objID;
}

ObjectID ObjectFactory::spawnSun(Vec3 position, float radius, float mass,
                                 Vec3 initialSpeed, Vec3 color) {
  ObjectID objID = this->world->CreateObject();
  Object &obj = this->world->objects[objID];
  this->world->sunID = objID;

  Sprite sprite = Sprite();
  sprite.modelID = "sphere";
  sprite.textureID = "solid";
  sprite.shaderID = "sun";
  sprite.color = color;
  obj.spriteID = this->world->AddSprite(sprite);

  Body body = Body();
  body.mass = mass;
  body.velocity = initialSpeed;
  obj.bodyID = this->world->AddBody(body);

  obj.transform.position = position;
  obj.transform.radius = radius;
  for (int i = 0; i < MAX_TRAIL; i++) {
    obj.trail[i] = position;
  }

  setupTrailRenderer(obj);

  return objID;
}

void ObjectFactory::spawnButton(float x, float y, float width, float height,
                                const std::string &label,
                                std::function<void()> onClick) {
  Button button;
  button.x = x;
  button.y = y;
  button.width = width;
  button.height = height;
  button.label = label;
  button.onClick = onClick;

  this->world->widgets.push_back(button);
}
