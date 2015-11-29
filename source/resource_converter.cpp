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

  for (u32 i = 0; i < fileSize;)
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
        ModelFace face;
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
