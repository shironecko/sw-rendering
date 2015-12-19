#include "math3d.cpp"
#include "renderer.cpp"
#include "game_common.cpp"

struct GameData
{
  Mesh* creeperMesh;
  Texture* creeperColorTex;
};

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  u8* memory = (u8*)gameMemory;
  u32 memorySizeLeft = gameMemorySize;

  GameData* gameData = (GameData*)memory;
  memory += sizeof(GameData);
  memorySizeLeft -= sizeof(GameData);

  {
    u32 bytesRead = PlatformLoadFile("../data/cooked/meshes/creeper.mesh", memory, memorySizeLeft);
    gameData->creeperMesh = (Mesh*)memory;
    u8* meshMemory = (u8*)memory;
    memory += bytesRead;
    memorySizeLeft -= bytesRead;

    meshMemory += sizeof(Mesh);
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
    u32 bytesRead = PlatformLoadFile("../data/cooked/textures/creeper_color.tex", memory, memorySizeLeft);
    gameData->creeperColorTex = (Texture*)memory;
    memory += bytesRead;
    memorySizeLeft -= bytesRead;
  }
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
  if (input->keyboard['Q'])
    return false;

  GameData* gameData = (GameData*)gameMemory;
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
