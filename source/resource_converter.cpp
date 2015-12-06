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

void MemoryCopy(u8* destination, u8* source, u32 bytesToCopy)
{
  for (u32 i = 0; i < bytesToCopy; ++i)
  {
    destination[i] = source[i];
  }
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

  for (u32 i = 0; i < fileSize; )
  {
    switch (objText[i])
    {
      case 'v':
      {
        switch (objText[i + 1])
        {
          case ' ':
          {
            i += 2;
            Vector4 vertice;
            i += ParseVector3(objText + i, &vertice);
            vertice.w = 1.0f;

            *vertices = vertice;
            ++vertices;
          } break;
          case 't':
          {
            i += 3;
            Vector2 uv;

            for (u32 j = 0; j < 2; ++j)
            {
              i += ParseFloat(objText + i, &uv.components[j]);
              ++i;
            }

            *uvs = uv;
            ++uvs;
          } break;
          case 'n':
          {
            i += 3;
            Vector4 normale;
            i += ParseVector3(objText + i, &normale);
            normale.w = 1.0f;

            *normales = normale;
            ++normales;
          } break;
          default:
          {
            i += SkipLine(objText + i);
          } break;
        }
      } break;
      case 'f':
      {
        MeshFace face;
        i += 2;
        for (u32 j = 0; j < 3; ++j)
        {
          i += ParseUInteger(objText + i, &face.vertices[j]);
          --face.vertices[j];
          ++i;
          i += ParseUInteger(objText + i, &face.uvs[j]);
          --face.uvs[j];
          ++i;
          i += ParseUInteger(objText + i, &face.normals[j]);
          --face.normals[j];
          ++i;
        }

        *faces = face;
        ++faces;
      } break;
      default:
      {
        // lets skip a line here...
        i += SkipLine(objText + i);
      } break;
    }
  }

  Mesh* mesh = (Mesh*)memory;
  u8* memoryOriginal = memory;
  memory += sizeof(Mesh);

  mesh->vertices = (Vector4*)memory;
  mesh->verticesCount = vertices - verticesOriginal;
  u32 verticesBytes = mesh->verticesCount * sizeof(Vector4);
  MemoryCopy((u8*)mesh->vertices, (u8*)verticesOriginal, verticesBytes);
  memory += verticesBytes;

  mesh->uvs = (Vector2*)memory;
  mesh->uvsCount = uvs - uvsOriginal;
  u32 uvsBytes = mesh->uvsCount * sizeof(Vector2);
  MemoryCopy((u8*)mesh->uvs, (u8*)uvsOriginal, uvsBytes);
  memory += uvsBytes;

  mesh->normales = (Vector4*)memory;
  mesh->normalesCount = normales - normalesOriginal;
  u32 normalesBytes = mesh->normalesCount * sizeof(Vector4);
  MemoryCopy((u8*)mesh->normales, (u8*)normalesOriginal, normalesBytes);
  memory += normalesBytes;

  mesh->faces = (MeshFace*)memory;
  mesh->facesCount = faces - facesOriginal;
  u32 facesBytes = mesh->facesCount * sizeof(MeshFace);
  MemoryCopy((u8*)mesh->faces, (u8*)facesOriginal, facesBytes);
  memory += facesBytes;

  return memory - memoryOriginal;
}

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  u8* memory = (u8*)gameMemory;
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
