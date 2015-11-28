#include "math3d.cpp"
#include "renderer.cpp"

bool IsNumber(char c)
{
  return c >= '0' && c <= '9';
}

u32 ParseFloat(char* inText, float* outFloat)
{
  char* text = inText;
  float result = 0;

  if (*text == '-')
  {
    result = -0;
    ++text;
  }

  float multiplier = 10.0f;
  for (;;)
  {
    char c = *text;
    if (IsNumber(c))
    {
      result *= multiplier;
      result += c - '0';
      ++text;
    }
    else if (c == '.')
    {
      multiplier = 1.0f / 10.0f;
      ++text;
    }
    else
      break;
  }

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

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  u8* memory = (u8*)gameMemory;

  u32 fileSize = PlatformLoadFile(
      "../data/Creeper/creeper.obj",
      (void*)memory,
      gameMemorySize);

  char* text = (char*)memory;
  memory += fileSize;

  Vector4* vertices = (Vector4*)memory;
  Vector4* verticesOriginal = vertices;
  memory += fileSize;

  Vector2* uvs = (Vector2*)memory;
  Vector2* uvsOriginal = uvs;
  memory += fileSize;

  Vector4* normales = (Vector4*)memory;
  Vector4* normalesOriginal = normales;
  memory += fileSize;

  ModelFace* faces = (ModelFace*)memory;
  ModelFace* facesOriginal = faces;
  memory += fileSize;

  assert(u32((u8*)memory - (u8*)gameMemory) < gameMemorySize);

  for (u32 i = 0; i < fileSize; ++i)
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
          } break;
          case 'n':
          {
            i += 2;
            Vector4 normale;
            i += ParseVector3(text + i, &normale);
            normale.w = 1.0f;

            *normales = normale;
            ++normales;
          } break;
        }
      } break;
      case 'f':
      {
      } break;
      default:
      {
        // nothing to do here...
      } break;
    }
  }
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
