#include "math3d.cpp"
#include "renderer.cpp"

struct Input
{
  bool keyboard[256];
};

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
    RenderTarget* renderTarget)
{
  Mesh* mesh = (Mesh*)gameMemory;
  Render(
      renderTarget,
      RenderMode::Wireframe,
      mesh,
      nullptr,
      0,
      0,
      4.0f,
      0,
      { 0, 0, 0, 0});

  return true;
}
