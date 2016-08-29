#include "platform_api.h"

#include "utility.cpp"
#include "math3d.cpp"
#include "renderer.cpp"

// TODO: this is kinda lame, maybe there is smth I can do about it... or who cares
#define PATH_LEN 1024 + 1

bool StringCompare(const char *a, const char *b)
{
    assert(a);
    assert(b);

    while (*a && *b)
    {
        if (*a != *b)
            return false;

        ++a;
        ++b;
    }

    return true;
}

u32 StringLen(const char *str)
{
    assert(str);
    const char *p;
    for (p = str; *p; ++p) { }

    return p - str;
}

u32 StringCopyPred(char *dest, const char *src, u32 max, bool (*pred)(char, void*), void* pred_data)
{
    assert(dest);
    assert(src);

    u32 i;
    for (i = 0; i < max - 1 && src[i]; ++i)
    {
        if (pred && !pred(src[i], pred_data))
            break;

        dest[i] = src[i];
    }

    dest[i] = 0;
    return i;
}

u32 StringCopy(char *dest, const char *src, u32 max)
{
    return StringCopyPred(dest, src, max, 0, 0);
}

u32 StringCat(char *dest, const char *src, u32 destLen)
{
    assert(dest);
    assert(src);
    char *p = dest;
    while (*p) { ++p; }

    return p - dest + StringCopy(p, src, destLen - (p - dest));
}

void StringCombine(char *dest, u32 destLen, const char *a, const char *b)
{
    assert(dest);
    assert(destLen);
    assert(a);
    assert(b);

    *dest = 0;
    StringCat(dest, a, destLen);
    StringCat(dest, b, destLen);
}

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

void MemorySet(void* memory, u8 value, u32 size)
{
    // NOTE: super-slow, but who cares
    u8* mem = (u8*)memory;
    for (u32 i = 0; i < size; ++i)
        *mem = value;

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

bool StringBeginsWith(const char* string, const char* prefix)
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

bool StringPredCharNotInList(char c, void* d)
{
    char *charList = (char*)d;
    for (char *p = charList; *p; ++p)
    {
        if (c == *p)
            return false;

    }

    return true;
}

#pragma pack(push, 1)
struct BitmapFileHeader 
{
  u16 type;
  u32 size;
  u16 reserved1;
  u16 reserved2;
  u32 offBytes;
};

struct BitmapInfoHeader 
{
  u32 size;
  s32 width;
  s32 height;
  u16 planes;
  u16 bitCount;
  u32 compression;
  u32 sizeImage;
  s32 xPelsPerMeter;
  s32 yPelsPerMeter;
  u32 clrUsed;
  u32 clrImportant;
};
#pragma pack(pop)

struct BitmapColor32
{
  union
  {
    u8 components[4];

    struct
    {
      u8 a; // or padding, mostly just padding
      u8 b;
      u8 g;
      u8 r;
    };
  };
};

Texture LoadBmp(
    const char* resourcePath,
    const char* bmpName,
    MemPool *pool,
    bool dryRun)
{
  char bmpPath[PATH_LEN];
  StringCombine(bmpPath, sizeof(bmpPath), resourcePath, bmpName);
  u32 fileSize = (u32)PlatformGetFileSize(bmpPath);
  assert(fileSize);

  u8* initialHiPtr = pool->hiPtr;
  u8* rawBitmap = (u8*)MemPushBack(pool, fileSize);
  PlatformLoadFile(bmpPath, rawBitmap, fileSize);

  BitmapFileHeader* fileHeader = (BitmapFileHeader*)rawBitmap;
  BitmapInfoHeader* infoHeader = (BitmapInfoHeader*)(rawBitmap + sizeof(BitmapFileHeader));

  u16 bitmapFileType = (u16('M') << 8) | 'B';
  assert(fileHeader->type == bitmapFileType);
  assert(infoHeader->bitCount == 32);
  assert(infoHeader->compression == 0 || infoHeader->compression == 3);

  infoHeader->height = infoHeader->height >= 0 ? infoHeader->height : -infoHeader->height;

  Texture result = { 0 };
  StringCopy(result.name, bmpName, sizeof(result.name));
  result.width = infoHeader->width;
  result.height = infoHeader->height;

  BitmapColor32* rawPixels = (BitmapColor32*)(rawBitmap + fileHeader->offBytes);
  u32 texelsSize = sizeof(*result.texels) * result.width * result.height;
  result.texels = (Color32*)MemPush(pool, texelsSize);
  for (u32 i = 0, n = result.width * result.height; i < n; ++i)
  {
    BitmapColor32 rawPixel = rawPixels[i];
    result.texels[i] = { rawPixel.r, rawPixel.g, rawPixel.b, rawPixel.a };
  }

  pool->hiPtr = initialHiPtr;
  return result;
}

/*
 * Returns number of materials loaded
 */
void LoadMtl(const char *resourcePath, const char *mtlName, Material *materials, u32 *inOutMaterialsCount, Texture *textures, u32 *outTexturesCount, MemPool *pool, bool dryRun)
{
    // TODO: figure out something less clunky
    char mtlPath[PATH_LEN];
    StringCombine(mtlPath, sizeof(mtlPath), resourcePath, mtlName);
    u32 fileSize = (u32)PlatformGetFileSize(mtlPath);
    assert(fileSize);

    u8* initialHiPtr = pool->hiPtr;
    char* mtlText = (char*)MemPushBack(pool, fileSize + 1);
    PlatformLoadFile(mtlPath, mtlText, fileSize);
    mtlText[fileSize] = 0;
    u32 matCount = 0;
    u32 texturesCount = 0;

    if (dryRun)
    {
        for (char *c = mtlText; *c; ++c)
        {
            if (StringBeginsWith(c, "newmtl "))
                ++matCount;

        }

        *inOutMaterialsCount = matCount;
    }
    else
        matCount = *inOutMaterialsCount;

    s32 materialIdx = -1;
    for (char* text = mtlText; u32(text - mtlText) < fileSize; text += SkipLine(text))
    {
        if (!dryRun && StringBeginsWith(text, "newmtl "))
        {
            ++materialIdx;
            MemorySet(materials + materialIdx, 0, sizeof(*materials));
            text += StringLen("newmtl ");
            StringCopyPred(materials[materialIdx].name, text, sizeof(materials[materialIdx].name), StringPredCharNotInList, "\n");
        }
        else if (StringBeginsWith(text, "map_Kd "))
        {
            ++texturesCount;
            if (dryRun)
                continue;

            text += StringLen("map_Kd ");
            char bmpName[PATH_LEN] = { 0 };
            StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, " \n");
            textures[texturesCount - 1] = LoadBmp(resourcePath, bmpName, pool, dryRun);
            materials[materialIdx].diffuse = textures + texturesCount - 1;
        }
        else if (StringBeginsWith(text, "bump "))
        {
            ++texturesCount;
            if (dryRun)
                continue;

            text += StringLen("bump ");
            char bmpName[PATH_LEN] = { 0 };
            StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, " \n");
            textures[texturesCount - 1] = LoadBmp(resourcePath, bmpName, pool, dryRun);
            materials[materialIdx].bump = textures + texturesCount - 1;
        }
        else if (StringBeginsWith(text, "map_Ks "))
        {
            ++texturesCount;
            if (dryRun)
                continue;

            text += StringLen("map_Ks ");
            char bmpName[PATH_LEN] = { 0 };
            StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, " \n");
            textures[texturesCount - 1] = LoadBmp(resourcePath, bmpName, pool, dryRun);
            materials[materialIdx].specular = textures + texturesCount - 1;
        }
    }

    assert((materialIdx == (s32)matCount - 1) || dryRun);

    assert(outTexturesCount);
    *outTexturesCount = texturesCount;

    pool->hiPtr = initialHiPtr;
}

void LoadObj(const char* resourcePath, const char* objName, MemPool *pool, Model *inOutModel, u32 *inOutTotalFaceCount, bool dryRun)
{
  char objPath[PATH_LEN];
  StringCombine(objPath, sizeof(objPath), resourcePath, objName);
  u32 fileSize = (u32)PlatformGetFileSize(objPath);
  assert(fileSize);

  u8* initialHiPtr = pool->hiPtr;
  char* objText = (char*)MemPushBack(pool, fileSize + 1);
  PlatformLoadFile(objPath, objText, fileSize);
  objText[fileSize] = 0;

  Vector4 *vertices = 0;
  u32 verticesCount = 0;

  Vector2 *uvs = 0;
  u32 uvsCount = 0;

  Vector4 *normales = 0;
  u32 normalesCount = 0;

  FaceGroup *faceGroups = 0;
  u32 faceGroupsCount = 0;
  MeshFace *faces = 0;
  u32 totalFaceCount = 0;

  Material *materials = 0;
  u32 materialsCount = 0;

  Texture *textures = 0;
  u32 texturesCount = 0;

  if (!dryRun)
  {
      vertices = (Vector4*)MemPush(pool, inOutModel->verticesCount * sizeof(*vertices));
      uvs = (Vector2*)MemPush(pool, inOutModel->uvsCount * sizeof(*uvs));
      normales = (Vector4*)MemPush(pool, inOutModel->normalesCount * sizeof(*normales));
      faceGroups = (FaceGroup*)MemPush(pool, inOutModel->faceGroupsCount * sizeof(*faceGroups));
      faces = (MeshFace*)MemPush(pool, *inOutTotalFaceCount * sizeof(*faces));
      materials = (Material*)MemPush(pool, inOutModel->materialsCount * sizeof(*materials));
      textures = (Texture*)MemPush(pool, inOutModel->texturesCount * sizeof(*textures));

      materialsCount = inOutModel->materialsCount;
  }

  for (char* text = objText; u32(text - objText) < fileSize; text += SkipLine(text))
  {
    if (StringBeginsWith(text, "mtllib "))
    {
        // NOTE: no support for multiple mtl filse yet
        char* localText = text + 7;
        char mtlName[PATH_LEN];
        StringCopyPred(mtlName, localText, sizeof(mtlName), StringPredCharNotInList, "\n");
        u32 tc = 0;
        LoadMtl(resourcePath, mtlName, materials, &materialsCount, textures + texturesCount, &tc, pool, dryRun);
        texturesCount += tc;
    }
    else if (StringBeginsWith(text, "usemtl "))
    {
        // use material group
        ++faceGroupsCount;
        if (dryRun)
            continue;

        char* localText = text + StringLen("usemtl ");
        char materialName[RC_NAME_LEN];
        StringCopyPred(materialName, localText, sizeof(materialName), StringPredCharNotInList, " \n");
        s32 materialIndex = -1;
        for (u32 i = 0; i < materialsCount; ++i)
        {
            if (StringCompare(materialName, materials[i].name))
            {
                materialIndex = i;
                break;
            }
        }

        assert(materialIndex != -1);
        faceGroups[faceGroupsCount - 1].faces = faces + totalFaceCount;
        faceGroups[faceGroupsCount - 1].facesCount = 0;
        faceGroups[faceGroupsCount - 1].material = materials + materialIndex;
    }
    else if (StringBeginsWith(text, "f "))
    {
      // face
      if (!faceGroupsCount)
      {
          ++faceGroupsCount;
          if (!dryRun)
          {
              faceGroups->faces = faces;
              faceGroups->facesCount = 0;
              faceGroups->material = 0;
          }
      }

      MeshFace face;
      char* localText = text + 2;
      for (u32 i = 0; i < 3; ++i)
      {
        localText += ParseUInteger(localText, &face.v[i]) + 1;
        --face.v[i];
        
        localText += ParseUInteger(localText, &face.uv[i]) + 1;
        --face.uv[i];

        localText += ParseUInteger(localText, &face.n[i]) + 1;
        --face.n[i];
      }

      if (!dryRun)
      {
          faces[totalFaceCount] = face;
          ++faceGroups[faceGroupsCount - 1].facesCount;
      }

      ++totalFaceCount;

      if (IsNumber(*localText))
      {
          // triangulate a quad
          face.v[1] = face.v[2];
          face.uv[1] = face.uv[2];
          face.n[1] = face.n[2];
          localText += ParseUInteger(localText, &face.v[2]) + 1;
          --face.v[2];
          localText += ParseUInteger(localText, &face.uv[2]) + 1;
          --face.uv[2];
          localText += ParseUInteger(localText, &face.n[2]) + 1;
          --face.n[2];

          if (!dryRun)
          {
              faces[totalFaceCount] = face;
              ++faceGroups[faceGroupsCount - 1].facesCount;
          }

          ++totalFaceCount;
      }
    }
    else if (StringBeginsWith(text, "v "))
    {
      // vertex
      ++verticesCount;
      if (dryRun)
          continue;

      Vector4 vertice;
      ParseVector3(text + 2, &vertice);
      vertice.w = 1.0f;
      vertices[verticesCount - 1] = vertice;
    }
    else if (StringBeginsWith(text, "vt "))
    {
      // uv
      ++uvsCount;
      if (dryRun)
          continue;

      Vector2 uv;
      char* localText = text + 3;
      for (u32 i = 0; i < 2; ++i)
        localText += ParseFloat(localText, &uv.components[i]) + 1;

      uvs[uvsCount - 1] = uv;
    }
    else if (StringBeginsWith(text, "vn "))
    {
      // normal
      ++normalesCount;
      if (dryRun)
          continue;

      Vector4 normale;
      ParseVector3(text + 3, &normale);
      normale.w = 0.0f;
      normales[normalesCount - 1] = normale;
    }
  }

  if (dryRun)
  {
      inOutModel->verticesCount = verticesCount;
      inOutModel->uvsCount = uvsCount;
      inOutModel->normalesCount = normalesCount;
      inOutModel->faceGroupsCount = faceGroupsCount;
      inOutModel->materialsCount = materialsCount;
      inOutModel->texturesCount = texturesCount;
      *inOutTotalFaceCount = totalFaceCount;
  }
  else
  {
      assert(inOutModel->verticesCount == verticesCount);
      assert(inOutModel->uvsCount == uvsCount);
      assert(inOutModel->normalesCount == normalesCount);
      assert(inOutModel->faceGroupsCount == faceGroupsCount);
      assert(inOutModel->materialsCount == materialsCount);
      assert(inOutModel->texturesCount == texturesCount);
      assert(*inOutTotalFaceCount == totalFaceCount);

      u32 facesInGroups = 0;
      for (u32 i = 0; i < faceGroupsCount; ++i)
          facesInGroups += faceGroups[i].facesCount;

      assert(facesInGroups == totalFaceCount);

      inOutModel->vertices = vertices;
      inOutModel->uvs = uvs;
      inOutModel->normales = normales;
      inOutModel->faceGroups = faceGroups;
      inOutModel->materials = materials;
      inOutModel->textures = textures;
  }

  pool->hiPtr = initialHiPtr;
}

void LoadModel(const char* resourcePath, const char* objName, MemPool *pool, Model *outModel)
{
    assert(resourcePath);
    assert(objName);
    assert(pool);
    assert(outModel);

    // a dry run to collect the number of vertices, size of the textures etc.
    MemPool initialPool = *pool;
    u32 totalFaceCount = 0;
    LoadObj(resourcePath, objName, pool, outModel, &totalFaceCount, true);
    assert(MemoryEqual(&initialPool, pool, sizeof(initialPool)));

    // an actual process of loading a model
    LoadObj(resourcePath, objName, pool, outModel, &totalFaceCount, false);
    assert(initialPool.hiPtr == pool->hiPtr);
}

struct GameData
{
    Model model;
    MemPool pool;
};

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
    GameData* gameData = (GameData*)gameMemory;
    // TODO: add alignment to MemPool
    gameData->pool = NewMemPool((u8*)gameMemory + sizeof(*gameData), gameMemorySize - sizeof(*gameData));

    LoadModel("./assets/", "muro.obj", &gameData->pool, &gameData->model);
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

  float scale = 0.05f;
  Matrix4x4 model = TranslationMatrix(0, -5.0f, 0) * ScaleMatrix(scale, scale, scale);

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

  Matrix4x4 screenMatrix = ScreenSpaceMatrix(renderTarget->texture->width, renderTarget->texture->height); 

  GameData* gameData = (GameData*)gameMemory;

  ClearRenderTarget(renderTarget, { 0, 0, 0, 255 });
  Render(
      renderTarget,
      RenderMode::Shaded | RenderMode::Textured,
      &gameData->model,
      camPos,
      view * model,
      projection,
      screenMatrix,
      Normalized3(Vector4{ 1, 1, 1, 0 }),
      { 255, 255, 255, 255 },
      0.05f,
      &gameData->pool);

  return true;
}
