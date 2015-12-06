#include "math3d.cpp"
#include "renderer.cpp"
#include "game_common.cpp"

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  u8* memory = (u8*)gameMemory;
  u32 bytesRead = PlatformLoadFile("../data/cooked/meshes/creeper.mesh", gameMemory, gameMemorySize);
  Mesh* mesh = (Mesh*)memory;
  u8* meshMemory = (u8*)memory;
  memory += bytesRead;

  meshMemory += sizeof(Mesh);
  mesh->vertices = (Vector4*)meshMemory;
  meshMemory += mesh->verticesCount * sizeof(Vector4);

  mesh->uvs = (Vector2*)meshMemory;
  meshMemory += mesh->uvsCount * sizeof(Vector2);

  mesh->normales = (Vector4*)meshMemory;
  meshMemory += mesh->normalesCount * sizeof(Vector4);

  mesh->faces = (MeshFace*)meshMemory;
  meshMemory += mesh->facesCount * sizeof(MeshFace);
}

local bool GameUpdate(
    float deltaTime,
    void* gameMemory,
    u32 gameMemorySize,
    RenderTarget* renderTarget,
    Input* input)
{
  const float camZoomSpeed = 1.0f;
  const float camRotationSpeed = 2.0f;
  local_persist float camDistance = 4.0f;
  local_persist float camRotation = 0;

  if (input->keyboard['W'])
    camDistance -= camZoomSpeed * deltaTime;
  if (input->keyboard['S'])
    camDistance += camZoomSpeed * deltaTime;
  if (input->keyboard['A'])
    camRotation -= camRotationSpeed * deltaTime;
  if (input->keyboard['D'])
    camRotation += camRotationSpeed * deltaTime;

  Mesh* mesh = (Mesh*)gameMemory;
  Render(
      renderTarget,
      RenderMode::Shaded,
      /* RenderMode::Wireframe, */
      mesh,
      nullptr,
      0,
      0,
      camDistance,
      camRotation,
      (Vector4 { 1.0f, -1.0f, 0, 0}).Normalized3());

  return true;
}
