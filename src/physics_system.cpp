#include "physics_system.h"
#include "resource_manager.h"
#include "types.h"
#include <cmath>
#include <iostream>
#include <vector>

// Temporary debug helper — drains all GL errors and prints them with a tag.
static void checkGL(const char *tag) {
  GLenum err;
  while ((err = glGetError()) != GL_NO_ERROR) {
    std::cerr << "[GL ERROR " << err << "] at " << tag << std::endl;
  }
}

// Number of particles spawned per collision explosion.
static constexpr int EXPLOSION_PARTICLE_COUNT = 50;

PhysicsSystem::PhysicsSystem() : G(1.0f) {
}

PhysicsSystem::~PhysicsSystem() {
}

void PhysicsSystem::step(World *world, float dt, ObjectFactory *factory) {
  dispatchObjects(world, dt);
  checkGL("step:afterDispatchObjects");
  resolveCollisions(world, factory);
  checkGL("step:afterResolveCollisions");
  dispatchParticles(world, dt);
  checkGL("step:afterDispatchParticles");
  ++counter;
}

void PhysicsSystem::dispatchObjects(World *world, float dt) {
  checkGL("dispatchObjects:enter");
  const Shader &shader = ResourceManager::GetShader("objectphy");
  shader.Use();
  checkGL("dispatchObjects:use");

  int objectCount = static_cast<int>(world->objects.size());

  shader.SetFloat("dt", dt);
  shader.SetFloat("G", G);
  shader.SetInteger("objectCount", objectCount);
  shader.SetInteger("frameCounter", counter);
  checkGL("dispatchObjects:uniforms");

  // Binding 0: read buffer (ping)
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, world->SSBOobjA);
  // Binding 1: write buffer (pong)
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, world->SSBOobjB);
  // Binding 2: collision output
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, world->SSBOcollisions);
  // Binding 3: trail storage
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, world->SSBOtrail);
  checkGL("dispatchObjects:bindbuffers");

  // Upload per-object trail heads as a flat uniform array
  glUniform1iv(glGetUniformLocation(shader.ID, "trailHeads"), objectCount,
               world->trailHeads);
  checkGL("dispatchObjects:trailHeads");

  int groups = (objectCount + 63) / 64;
  if (groups < 1) groups = 1;
  glDispatchCompute(groups, 1, 1);
  checkGL("dispatchObjects:dispatch");
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  checkGL("dispatchObjects:barrier");

  // Ping-pong: output becomes next frame's input
  std::swap(world->SSBOobjA, world->SSBOobjB);

  // Unbind all SSBO binding points before mapping — some drivers (Mesa)
  // generate GL_INVALID_OPERATION if the buffer is still attached to an
  // indexed binding point when mapped.
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, 0);

  // Advance trail heads on CPU to stay in sync with the shader
  if (objectCount > 0) {
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBOobjA); // now the written buf
  checkGL("dispatchObjects:bindForMap");
  GPUBody *bodies = reinterpret_cast<GPUBody *>(
      glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                       objectCount * sizeof(GPUBody), GL_MAP_READ_BIT));
  checkGL("dispatchObjects:mapBufferRange");
  if (bodies) {
    for (int i = 0; i < objectCount; i++) {
      if (bodies[i].alive > 0.5f) {
        // Compute the same dynamic rate the shader used so heads stay in sync
        float speed = glm::length(bodies[i].velocity);
        if (speed < 0.1f) speed = 0.1f;
        int rate = static_cast<int>(20.0f / speed);
        if (rate < 1) rate = 1;
        if (counter % rate == 0) {
          world->trailHeads[i] = (world->trailHeads[i] + 1) % MAX_TRAIL;
        }
        // Mirror position back to CPU for light-source / collision-response use
        world->objects[i].transform.position =
            Vec3(bodies[i].position.x, bodies[i].position.y, bodies[i].position.z);
      }
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    checkGL("dispatchObjects:unmap");
  }
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
  checkGL("dispatchObjects:unbind");
  } // objectCount > 0
}

void PhysicsSystem::resolveCollisions(World *world, ObjectFactory *factory) {
  // Read just the collision count first (first uint in the buffer)
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBOcollisions);

  uint32_t count = 0;
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &count);

  if (count == 0) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    return;
  }

  if (count > MAX_COLLISIONS) count = MAX_COLLISIONS;

  // Read the collision pairs (offset by 4 uints for std430 alignment)
  std::vector<CollisionPair> pairs(count);
  glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,
                     sizeof(uint32_t) * 4,
                     count * sizeof(CollisionPair),
                     pairs.data());

  // Reset the count for the next frame
  const uint32_t zero = 0;
  glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint32_t), &zero);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  for (const auto &pair : pairs) {
    uint32_t sIdx = pair.survivorIdx;
    uint32_t dIdx = pair.destroyedIdx;

    if (sIdx >= world->objects.size() || dIdx >= world->objects.size())
      continue;

    Object &survivor  = world->objects[sIdx];
    Object &destroyed = world->objects[dIdx];

    if (!survivor.active || !destroyed.active)
      continue;
    if (survivor.bodyID == INVALID_ID || destroyed.bodyID == INVALID_ID)
      continue;

    Body &bodyS = world->bodies[survivor.bodyID];
    Body &bodyD = world->bodies[destroyed.bodyID];

    // Read current GPU state for both objects
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBOobjA);
    GPUBody gpuS, gpuD;
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,
                       sIdx * sizeof(GPUBody), sizeof(GPUBody), &gpuS);
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER,
                       dIdx * sizeof(GPUBody), sizeof(GPUBody), &gpuD);

    // Merge physics
    float newRadius = std::cbrt(std::pow(gpuS.radius, 3.0f) +
                                std::pow(gpuD.radius, 3.0f));
    float totalMass = gpuS.mass + gpuD.mass;
    glm::vec3 finalVel =
        (gpuS.velocity * gpuS.mass + gpuD.velocity * gpuD.mass) / totalMass;

    // Merge color (CPU-side sprites)
    Vec3 colorS = world->sprites[survivor.spriteID].color;
    Vec3 colorD = world->sprites[destroyed.spriteID].color;
    Vec3 finalColor = (colorS * gpuS.mass + colorD * gpuD.mass) / totalMass;
    if (sIdx == world->sunID || dIdx == world->sunID) {
      SpriteID sunSpriteID = world->objects[world->sunID].spriteID;
      finalColor = world->sprites[sunSpriteID].color;
    }
    world->sprites[survivor.spriteID].color = finalColor;

    // Update CPU body mirror
    bodyS.mass     = totalMass;
    bodyS.radius   = newRadius;
    bodyS.velocity = Vec3(finalVel.x, finalVel.y, finalVel.z);
    survivor.transform.scale = Vec3(newRadius);

    // Write merged GPUBody back
    gpuS.mass     = totalMass;
    gpuS.radius   = newRadius;
    gpuS.velocity = finalVel;
    gpuS.scale    = glm::vec3(newRadius);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    sIdx * sizeof(GPUBody), sizeof(GPUBody), &gpuS);

    // Mark destroyed object dead in SSBO
    gpuD.alive = 0.0f;
    glBufferSubData(GL_SHADER_STORAGE_BUFFER,
                    dIdx * sizeof(GPUBody), sizeof(GPUBody), &gpuD);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    // Spawn explosion on CPU (position from GPU read)
    Vec3 posS(gpuS.position.x, gpuS.position.y, gpuS.position.z);
    Vec3 posD(gpuD.position.x, gpuD.position.y, gpuD.position.z);
    Vec3 normal = (posS - posD).normalized();
    factory->spawnExplosion(posS + normal * newRadius, normal,
                            destroyed, EXPLOSION_PARTICLE_COUNT);

    destroyed.destroyObj();
  }
}

void PhysicsSystem::dispatchParticles(World *world, float dt) {
  int particleCount = static_cast<int>(world->particles.size());
  if (particleCount == 0) return;

  checkGL("dispatchParticles:enter");
  const Shader &shader = ResourceManager::GetShader("particlephy");
  shader.Use();
  checkGL("dispatchParticles:use");

  shader.SetFloat("dt", dt);
  shader.SetFloat("G", G);
  shader.SetInteger("objectCount", static_cast<int>(world->objects.size()));
  shader.SetInteger("particleCount", particleCount);
  checkGL("dispatchParticles:uniforms");

  // Binding 0: body buffer (read-only, use the current ping buffer)
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, world->SSBOobjA);
  // Binding 1: particle buffer (read-write)
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, world->SSBOparticles);
  checkGL("dispatchParticles:bindbuffers");

  int groups = (particleCount + 63) / 64;
  glDispatchCompute(groups, 1, 1);
  checkGL("dispatchParticles:dispatch");
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
  checkGL("dispatchParticles:barrier");

  // Sync GPU particle state back to CPU so renderParticles uses correct
  // positions/velocities and dead particles are skipped.
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, world->SSBOparticles);
  GPUParticle *gpuParts = reinterpret_cast<GPUParticle *>(
      glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                       particleCount * sizeof(GPUParticle),
                       GL_MAP_READ_BIT));
  if (gpuParts) {
    for (int i = 0; i < particleCount; i++) {
      Particle &p = world->particles[i];
      const GPUParticle &g = gpuParts[i];
      p.transform.position = Vec3(g.position.x, g.position.y, g.position.z);
      p.transform.scale    = Vec3(g.scale.x, g.scale.y, g.scale.z);
      p.velocity           = Vec3(g.velocity.x, g.velocity.y, g.velocity.z);
      p.elapsedTime        = g.elapsedTime;
      p.active             = (g.alive > 0.5f);
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
  }
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
