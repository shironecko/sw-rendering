#include "math3d.cpp"
#include "renderer.cpp"

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

void ParseMeshObj(
    char* text,
    u32 textLength,
    Vector4* verticesMemory, 
    Vector2* uvsMemory,
    Vector4* normalesMemory,
    MeshFace* facesMemory,
    Mesh* outMesh)
{
  Vector4* vertices = verticesMemory;
  Vector2* uvs = uvsMemory;
  Vector4* normales = normalesMemory;
  MeshFace* faces = facesMemory;

  for (u32 i = 0; i < textLength;)
  {
    switch (text[i])
    {
      case 'v':
      {
        switch (text[i + 1])
        {
          case ' ':
          {
            i += 2;
            Vector4 vertice;
            i += ParseVector3(text + i, &vertice);
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
              i += ParseFloat(text + i, &uv.components[j]);
              ++i;
            }

            *uvs = uv;
            ++uvs;
          } break;
          case 'n':
          {
            i += 3;
            Vector4 normale;
            i += ParseVector3(text + i, &normale);
            normale.w = 1.0f;

            *normales = normale;
            ++normales;
          } break;
          default:
          {
            i += SkipLine(text + i);
          } break;
        }
      } break;
      case 'f':
      {
        MeshFace face;
        i += 2;
        for (u32 j = 0; j < 3; ++j)
        {
          i += ParseUInteger(text + i, &face.vertices[j]);
          --face.vertices[j];
          ++i;
          i += ParseUInteger(text + i, &face.uvs[j]);
          --face.uvs[j];
          ++i;
          i += ParseUInteger(text + i, &face.normals[j]);
          --face.normals[j];
          ++i;
        }

        *faces = face;
        ++faces;
      } break;
      default:
      {
        // lets skip a line here...
        i += SkipLine(text + i);
      } break;
    }
  }

  outMesh->vertices       = verticesMemory;
  outMesh->verticesCount  = vertices - verticesMemory;
  outMesh->uvs            = uvsMemory;
  outMesh->uvsCount       = uvs - uvsMemory;
  outMesh->normales       = normalesMemory;
  outMesh->normalesCount  = normales - normalesMemory;
  outMesh->faces          = facesMemory;
  outMesh->facesCount     = faces - facesMemory;
}

void MemoryCopy(u8* destination, u8* source, u32 bytesToCopy)
{
  for (u32 i = 0; i < bytesToCopy; ++i)
  {
    destination[i] = source[i];
  }
}

Mesh* PackMesh(Mesh mesh, void* inMemory, u32* outMeshBytes)
{
  u8* memory = (u8*)inMemory;
  Mesh* result = (Mesh*)memory;
  memory += sizeof(Mesh);

  result->vertices = (Vector4*)memory;
  result->verticesCount = mesh.verticesCount;
  u32 verticesBytes = mesh.verticesCount * sizeof(Vector4);
  MemoryCopy((u8*)result->vertices, (u8*)mesh.vertices, verticesBytes);
  memory += verticesBytes;

  result->uvs = (Vector2*)memory;
  result->uvsCount = mesh.uvsCount;
  u32 uvsBytes = mesh.uvsCount * sizeof(Vector2);
  MemoryCopy((u8*)result->uvs, (u8*)mesh.uvs, uvsBytes);
  memory += uvsBytes;

  result->normales = (Vector4*)memory;
  result->normalesCount = mesh.normalesCount;
  u32 normalesBytes = mesh.normalesCount * sizeof(Vector4);
  MemoryCopy((u8*)result->normales, (u8*)mesh.normales, normalesBytes);
  memory += normalesBytes;

  result->faces = (MeshFace*)memory;
  result->facesCount = mesh.facesCount;
  u32 facesBytes = mesh.facesCount * sizeof(MeshFace);
  MemoryCopy((u8*)result->faces, (u8*)mesh.faces, facesBytes);
  memory += facesBytes;

  *outMeshBytes = memory - (u8*)inMemory;
  return result;
}

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  u8* memory = (u8*)gameMemory;

  u32 fileSize = PlatformLoadFile( "../data/source/meshes/creeper.obj", (void*)memory, gameMemorySize);

  char* text = (char*)memory;
  memory += fileSize;

  Vector4* vertices = (Vector4*)memory;
  memory += fileSize;

  Vector2* uvs = (Vector2*)memory;
  memory += fileSize;

  Vector4* normales = (Vector4*)memory;
  memory += fileSize;

  MeshFace* faces = (MeshFace*)memory;
  memory += fileSize;

  Mesh unpackedMesh;
  ParseMeshObj( text, fileSize, vertices, uvs, normales, faces, &unpackedMesh);

  u32 meshBytes;
  Mesh* packedMesh = PackMesh(unpackedMesh, memory, &meshBytes);
  PlatformWriteFile("../data/cooked/meshes/creeper.mesh", packedMesh, meshBytes);

  assert(u32((u8*)memory - (u8*)gameMemory) < gameMemorySize);
}

local bool GameUpdate(
    float /* deltaTime */,
    void* /* gameMemory */,
    u32 /* gameMemorySize */,
    RenderTarget* /* renderTarget */)
{
  // NOTE: converter does all stuff on GameInitialize(...)
  return false;
}
