#include "math3d.cpp"
#include "renderer.cpp"
#include "game_common.cpp"

u32 SkipLine(char* inText)
{
  char* text = inText;
  while (*text != 0x0A && *text != 0)
  {
    ++text;
  }

  return text - inText + 1;
}

bool IsNumber(char c)
{
  return c >= '0' && c <= '9';
}

u32 ParseUInteger(char* inText, u32* outUInt)
{
  char* text = inText;
  u32 result = 0;

  while (IsNumber(*text))
  {
    result *= 10;
    result += *text - '0';
    ++text;
  }

  *outUInt = result;
  return text - inText;
}

u32 ParseFloat(char* inText, float* outFloat)
{
  char* text = inText;
  bool positive = true;
  u32 fraction = 0;

  if (*text == '-')
  {
    positive = false;
    ++text;
  }

  bool pointEncountered = false;
  u32 divisor = 1;
  for (;;)
  {
    char c = *text;
    if (IsNumber(c))
    {
      fraction *= 10;
      fraction += c - '0';

      ++text;

      if (pointEncountered)
        divisor *= 10;
    }
    else if (c == '.')
    {
      pointEncountered = true;
      ++text;
    }
    else
      break;
  }

  float result = float(fraction) / float(divisor); 
  if (!positive)
    result *= -1.0f;

  *outFloat = result;
  return text - inText;
}

void MemoryCopy(void* destination, void* source, u32 bytesToCopy)
{
  u8* destinationBytes = (u8*)destination;
  u8* sourceBytes = (u8*)source;

  for (u32 i = 0; i < bytesToCopy; ++i)
  {
    destinationBytes[i] = sourceBytes[i];
  }
}

bool MemoryEqual(void* memoryA, void* memoryB, u32 bytesToCompare)
{
  u8* memoryABytes = (u8*)memoryA;
  u8* memoryBBytes = (u8*)memoryB;

  for (u32 i = 0; i < bytesToCompare; ++i)
  {
    if (memoryABytes[i] != memoryBBytes[i])
      return false;
  }

  return true;
}

u32 ParseVector3(char* inText, Vector4* outVector)
{
  char* text = inText;
  Vector4 result;

  for (u32 i = 0; i < 3; ++i)
  {
    text += ParseFloat(text, &result.components[i]);
    ++text;
  }

  *outVector = result;
  return text - inText;
}

bool StringBeginsWith(char* string, char* prefix)
{
  while (*string && *prefix)
  {
    if (*string != *prefix)
      return false;

    ++string;
    ++prefix;
  }

  return true;
}

/* ParseMeshObj
 * Opens, reads and converts into a Mesh an OBJ file
 * The resulting mesh will be placed at the beginning
 * of the memory that was passed to this function
 */
u32 ParseMeshObj(
    char* pathToObj,
    u8* memory,
    u32 memorySize)
{
  u32 fileSize = (u32)PlatformGetFileSize(pathToObj);
  u8* temporaryStorage = memory + memorySize;
  temporaryStorage -= fileSize;
  char* objText = (char*)temporaryStorage;
  PlatformLoadFile(pathToObj, (void*)objText, fileSize);

  temporaryStorage -= fileSize;
  Vector4* vertices = (Vector4*)temporaryStorage;
  Vector4* verticesOriginal = vertices;

  temporaryStorage -= fileSize;
  Vector2* uvs = (Vector2*)temporaryStorage;
  Vector2* uvsOriginal = uvs;

  temporaryStorage -= fileSize;
  Vector4* normales = (Vector4*)temporaryStorage;
  Vector4* normalesOriginal = normales;

  temporaryStorage -= fileSize;
  MeshFace* faces = (MeshFace*)temporaryStorage;
  MeshFace* facesOriginal = faces;

  for (char* text = objText; u32(text - objText) < fileSize; text += SkipLine(text))
  {
    if (StringBeginsWith(text, "v "))
    {
      // vertex
      Vector4 vertice;
      ParseVector3(text + 2, &vertice);
      vertice.w = 1.0f;

      *vertices = vertice;
      ++vertices;
    }
    else if (StringBeginsWith(text, "vt "))
    {
      // uv
      Vector2 uv;
      char* localText = text + 3;
      for (u32 i = 0; i < 2; ++i)
      {
        localText += ParseFloat(localText, &uv.components[i]) + 1;
      }

      *uvs = uv;
      ++uvs;
    }
    else if (StringBeginsWith(text, "vn "))
    {
      // normal
      Vector4 normale;
      ParseVector3(text + 3, &normale);
      normale.w = 0.0f;

      *normales = normale;
      ++normales;
    }
    else if (StringBeginsWith(text, "f "))
    {
      // face
      MeshFace face;
      char* localText = text + 2;
      for (u32 i = 0; i < 3; ++i)
      {
        localText += ParseUInteger(localText, &face.vertices[i]) + 1;
        --face.vertices[i];
        
        localText += ParseUInteger(localText, &face.uvs[i]) + 1;
        --face.uvs[i];

        localText += ParseUInteger(localText, &face.normals[i]) + 1;
        --face.normals[i];
      }

      *faces = face;
      ++faces;
    }
  }

  Mesh* mesh = (Mesh*)memory;
  u8* memoryOriginal = memory;
  memory += sizeof(Mesh);

  mesh->vertices = (Vector4*)memory;
  mesh->verticesCount = vertices - verticesOriginal;
  u32 verticesBytes = mesh->verticesCount * sizeof(Vector4);
  MemoryCopy(mesh->vertices, verticesOriginal, verticesBytes);
  memory += verticesBytes;

  mesh->uvs = (Vector2*)memory;
  mesh->uvsCount = uvs - uvsOriginal;
  u32 uvsBytes = mesh->uvsCount * sizeof(Vector2);
  MemoryCopy(mesh->uvs, uvsOriginal, uvsBytes);
  memory += uvsBytes;

  mesh->normales = (Vector4*)memory;
  mesh->normalesCount = normales - normalesOriginal;
  u32 normalesBytes = mesh->normalesCount * sizeof(Vector4);
  MemoryCopy(mesh->normales, normalesOriginal, normalesBytes);
  memory += normalesBytes;

  mesh->faces = (MeshFace*)memory;
  mesh->facesCount = faces - facesOriginal;
  u32 facesBytes = mesh->facesCount * sizeof(MeshFace);
  MemoryCopy(mesh->faces, facesOriginal, facesBytes);
  memory += facesBytes;

  return memory - memoryOriginal;
}

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  u8* memory = (u8*)gameMemory;

  {
    Mesh* mesh = (Mesh*)memory;
    u32 meshSize = ParseMeshObj(
        "../data/source/meshes/creeper.obj",
        memory,
        gameMemorySize);

    PlatformWriteFile(
        "../data/cooked/meshes/creeper.mesh",
        mesh,
        meshSize);
  }
}

local bool GameUpdate(
    float /* deltaTime */,
    void* /* gameMemory */,
    u32 /* gameMemorySize */,
    RenderTarget* /* renderTarget */,
    Input* /* input */)
{
  // NOTE: converter does all stuff on GameInitialize(...)
  return false;
}
