#include "platform_api.h"

#include "math3d.cpp"
#include "renderer.cpp"

struct GameData
{
  Mesh* creeperMesh;
  Texture* creeperColorTex;
  void* freeMemory;
  u32 freeMemorySize;
};

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  u8* memory = (u8*)gameMemory;
  u8* memoryEnd = memory + gameMemorySize;

  GameData* gameData = (GameData*)memory;
  memory += sizeof(GameData);
  memory = align(memory);

  {
    gameData->creeperMesh = (Mesh*)memory;
    memory += Mesh::offset_to_serializable_data;
    u32 bytesRead = PlatformLoadFile("./data/cooked/meshes/creeper.mesh", memory, memoryEnd - memory);
    assert(bytesRead);
    u8* meshMemory = (u8*)memory;
    memory += bytesRead;
    memory = align(memory);

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
    memory = align(memory);
  }

  gameData->freeMemory = memory;
  gameData->freeMemorySize = memoryEnd - memory;
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
  local_persist float camDistance = 7.0f;
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

  Matrix4x4 model = IdentityMatrix();

  Vector4 camPos { 0, 0, camDistance, 1.0f };
  camPos = RotationMatrixY(camRotation) * camPos;
  Matrix4x4 view = LookAtCameraMatrix( 
      camPos,
      {    0,    0,    0, 1.0f },
      {    0, 1.0f,    0,    0 });

  Matrix4x4 projection = ProjectionMatrix(
      90.0f, 
      float(renderTarget->texture->width) / float(renderTarget->texture->height),
      0.1f,
      1000.0f);

  Matrix4x4 screenMatrix = ScreenSpaceMatrix( renderTarget->texture->width, renderTarget->texture->height); 
  Matrix4x4 MVP = projection * view * model;

  GameData* gameData = (GameData*)gameMemory;

  ClearRenderTarget(
      renderTarget,
      { 0, 0, 0, 255 });

  Render(
      renderTarget,
      RenderMode::Textured | RenderMode::Shaded,
      gameData->creeperMesh,
      gameData->creeperColorTex,
      MVP,
      screenMatrix,
      Normalized3(Vector4{ 0.5f, -1, 0.25f, 0 }),
      { 255, 255, 255, 255 },
      gameData->freeMemory,
      gameData->freeMemorySize);

  return true;
}
