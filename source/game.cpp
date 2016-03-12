#include "platform_api.h"

#include "math3d.cpp"
#include "renderer.cpp"

struct GameData
{
  Mesh* creeperMesh;
  Texture* creeperColorTex;
};

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  u8* memory = (u8*)gameMemory;
  u8* memoryEnd = memory + gameMemorySize;

  GameData* gameData = (GameData*)memory;
  memory += sizeof(GameData);

  {
    gameData->creeperMesh = (Mesh*)memory;
    memory += Mesh::offset_to_serializable_data;
    u32 bytesRead = PlatformLoadFile("./data/cooked/meshes/creeper.mesh", memory, memoryEnd - memory);
    assert(bytesRead);
    u8* meshMemory = (u8*)memory;
    memory += bytesRead;

    meshMemory += sizeof(Mesh) - Mesh::offset_to_serializable_data;
    gameData->creeperMesh->vertices = (Vector4*)meshMemory;
    meshMemory += gameData->creeperMesh->verticesCount * sizeof(Vector4);

    gameData->creeperMesh->uvs = (Vector2*)meshMemory;
    meshMemory += gameData->creeperMesh->uvsCount * sizeof(Vector2);

    gameData->creeperMesh->normales = (Vector4*)meshMemory;
    meshMemory += gameData->creeperMesh->normalesCount * sizeof(Vector4);

    gameData->creeperMesh->faces = (MeshFace*)meshMemory;
    meshMemory += gameData->creeperMesh->facesCount * sizeof(MeshFace);
  }

  {
    gameData->creeperColorTex = (Texture*)memory;
    gameData->creeperColorTex->texels = (Color32*)memory + sizeof(Texture);
    memory += Texture::offset_to_serializable_data;
    u32 bytesRead = PlatformLoadFile("./data/cooked/textures/creeper_color.tex", memory, memoryEnd - memory);
    assert(bytesRead);
    memory += bytesRead;
  }
}

local bool GameUpdate(
    float deltaTime,
    void* gameMemory,
    u32 gameMemorySize,
    RenderTarget* renderTarget,
    bool* kbState)
{
  const float camZoomSpeed = 1.0f;
  const float camRotationSpeed = 2.0f;
  local_persist float camDistance = 4.0f;
  local_persist float camRotation = 0;

  if (kbState[KbKey::W])
    camDistance -= camZoomSpeed * deltaTime;
  if (kbState[KbKey::S])
    camDistance += camZoomSpeed * deltaTime;
  if (kbState[KbKey::A])
    camRotation -= camRotationSpeed * deltaTime;
  if (kbState[KbKey::D])
    camRotation += camRotationSpeed * deltaTime;
  if (kbState[KbKey::Q])
    return false;

  GameData* gameData = (GameData*)gameMemory;

  ClearRenderTarget(
      renderTarget,
      { 100, 100, 200, 255 });

  Render(
      renderTarget,
      RenderMode::Textured | RenderMode::Shaded,
      /* RenderMode::Wireframe, */
      gameData->creeperMesh,
      gameData->creeperColorTex,
      camDistance,
      camRotation,
      (Vector4 { 1.0f, -1.0f, 0, 0}).Normalized3());

  return true;
}
