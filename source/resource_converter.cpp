#include "platform_api.h"

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

struct MemPool
{
    u8 *start, *end;
    u8 *lowPtr, *hiPtr;
};

MemPool NewMemPool(void *memory, u64 size)
{
    MemPool result;
    result.start = result.lowPtr = (u8*)memory;
    result.end = result.hiPtr = (u8*)memory + size;

    return result;
}

void* MemPush(MemPool *pool, u64 size)
{
    u8 *new_low_ptr = pool->lowPtr + size;
    assert(new_low_ptr <= pool->hiPtr);
    void *result = pool->lowPtr;
    pool->lowPtr = new_low_ptr;

    return result;
}

void* MemPushBack(MemPool *pool, u64 size)
{
    u8 *new_hi_ptr = pool->hiPtr - size;
    assert(new_hi_ptr >= pool->lowPtr);
    pool->hiPtr = new_hi_ptr;

    return new_hi_ptr;
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
      u8 padding;
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
  *bmpPath = 0;
  StringCat(bmpPath, resourcePath, sizeof(bmpPath));
  StringCat(bmpPath, bmpName, sizeof(bmpPath));
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
    result.texels[i] = { rawPixel.r, rawPixel.g, rawPixel.b, 0 };
  }

  pool->hiPtr = initialHiPtr;
  return result;
}

/*
 * Returns number of materials loaded
 */
u32 LoadMtl(const char *resourcePath, const char *mtlName, Material *materials, MemPool *pool, bool dryRun)
{
    // TODO: figure out something less clunky
    char mtlPath[PATH_LEN];
    *mtlPath = 0;
    StringCat(mtlPath, resourcePath, sizeof(mtlPath));
    StringCat(mtlPath, mtlName, sizeof(mtlPath));
    u32 fileSize = (u32)PlatformGetFileSize(mtlPath);
    assert(fileSize);

    u8* initialHiPtr = pool->hiPtr;
    char* mtlText = (char*)MemPushBack(pool, fileSize);
    PlatformLoadFile(mtlPath, mtlText, fileSize);
    u32 matCount = 0;

    // highly inefficient, but who cares...
    for (char *c = mtlText; *c; ++c)
    {
        if (StringBeginsWith(c, "newmtl "))
            ++matCount;

    }

    s32 materialIdx = -1;
    for (char* text = mtlText; u32(text - mtlText) < fileSize; text += SkipLine(text))
    {
        if (StringBeginsWith(text, "newmtl "))
        {
            ++materialIdx;
            MemorySet(materials + materialIdx, 0, sizeof(*materials));
            text += StringLen("newmtl ");
            StringCopyPred(materials[materialIdx].name, text, sizeof(materials[materialIdx].name), StringPredCharNotInList, "\n");
            
        }
        else if (StringBeginsWith(text, "map_Kd "))
        {
            text += StringLen("map_Kd ");
            char bmpName[PATH_LEN];
            *bmpName = 0;
            StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, " \n");
            
            Texture *tex = (Texture*)MemPush(pool, sizeof(*tex));
            *tex = LoadBmp(resourcePath, bmpName, pool, dryRun);

            materials[materialIdx].diffuse = tex;
        }
        else if (StringBeginsWith(text, "bump "))
        {
            text += StringLen("bump ");
            char bmpName[PATH_LEN];
            *bmpName = 0;
            StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, " \n");
            
            Texture *tex = (Texture*)MemPush(pool, sizeof(*tex));
            *tex = LoadBmp(resourcePath, bmpName, pool, dryRun);

            materials[materialIdx].bump = tex;
        }
        else if (StringBeginsWith(text, "map_Ks "))
        {
            text += StringLen("map_Ks ");
            char bmpName[PATH_LEN];
            *bmpName = 0;
            StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, " \n");
            
            Texture *tex = (Texture*)MemPush(pool, sizeof(*tex));
            *tex = LoadBmp(resourcePath, bmpName, pool, dryRun);

            materials[materialIdx].specular = tex;
        }
    }

    pool->hiPtr = initialHiPtr;
    return matCount;
}

void LoadObj(const char* resourcePath, const char* objName, MemPool *pool, Model *outModel, bool dryRun)
{
  char objPath[PATH_LEN];
  StringCombine(objPath, sizeof(objPath), resourcePath, objName);
  u32 fileSize = (u32)PlatformGetFileSize(objPath);
  assert(fileSize);

  u8* initialHiPtr = pool->hiPtr;
  char* objText = (char*)MemPushBack(pool, fileSize);
  PlatformLoadFile(objPath, objText, fileSize);

  Vector4 *vertices = 0;
  u32 verticesCount = 0;

  Vector2 *uvs = 0;
  u32 uvsCount = 0;

  Vector4 *normales = 0;
  u32 normalesCount = 0;

  Material *materials = 0;
  u32 materialsCount = 0;

  FacesGroup *faceGroups = 0;
  u32 faceGroupsCount = 0;
  u32 totalFaceCount = 0;

  for (char* text = objText; u32(text - objText) < fileSize; text += SkipLine(text))
  {
    if (StringBeginsWith(text, "mtllib "))
    {
        /* assert(!(materials - materialsOriginal)); */
        /* char* localText = text + 7; */
        /* char mtlName[PATH_LEN]; */
        /* StringCopyPred(mtlName, localText, sizeof(mtlName), StringPredCharNotInList, "\n"); */
        /* materials += ParseMtl(resourcePath, mtlName, materials, pool); */
    }
    else if (StringBeginsWith(text, "usemtl "))
    {
        // use material group
        if (dryRun)
        {
            ++faceGroupsCount;
            continue;
        }

        /* char* localText = text + StringLen("usemtl "); */
        /* char materialName[RC_NAME_LEN]; */
        /* StringCopyPred(materialName, localText, sizeof(materialName), StringPredCharNotInList, " \n"); */
        /* s32 materialIndex = -1; */
        /* for (u32 i = 0; i < (u32)(materials - materialsOriginal); ++i) */
        /* { */
        /*     if (StringCompare(materialName, materialsOriginal[i].name)) */
        /*     { */
        /*         materialIndex = i; */
        /*         break; */
        /*     } */
        /* } */

        /* assert(materialIndex != -1); */
        /* if (facesGroups->facesCount) */
        /* { */
        /*     ++facesGroups; */
        /*     facesGroups->faces = (MeshFace*)MemPushBack(pool, fileSize); */
        /*     facesGroups->facesCount = 0; */
        /* } */

        /* facesGroups->material = materialsOriginal + materialIndex; */
    }
    else if (StringBeginsWith(text, "f "))
    {
      // face
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

      ++totalFaceCount;
      if (!dryRun)
          faceGroups->faces[faceGroups->facesCount++] = face;

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

          ++totalFaceCount;
          if (!dryRun)
              faceGroups->faces[faceGroups->facesCount++] = face;

      }
    }
    else if (StringBeginsWith(text, "v "))
    {
      // vertex
      if (dryRun)
      {
          ++verticesCount;
          continue;
      }

      Vector4 vertice;
      ParseVector3(text + 2, &vertice);
      vertice.w = 1.0f;

      *vertices = vertice;
      ++vertices;
    }
    else if (StringBeginsWith(text, "vt "))
    {
      // uv
      if (dryRun)
      {
          ++uvsCount;
          continue;
      }

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
      if (dryRun)
      {
          ++normalesCount;
          continue;
      }

      Vector4 normale;
      ParseVector3(text + 3, &normale);
      normale.w = 0.0f;

      *normales = normale;
      ++normales;
    }
  }

  outModel->verticesCount = verticesCount;
  outModel->uvsCount = uvsCount;
  outModel->normalesCount = normalesCount;
  outModel->faceGroupsCount = faceGroupsCount;
  outModel->materialsCount = materialsCount;
  /* outModel->texturesCount = texturesCount; */
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
    LoadObj(resourcePath, objName, pool, outModel, true);
    assert(MemoryEqual(&initialPool, pool, sizeof(initialPool)));

    // an actual process of loading a model
    LoadObj(resourcePath, objName, pool, outModel, false);
    assert(initialPool.hiPtr == pool->hiPtr);
}

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  MemPool pool = NewMemPool(gameMemory, gameMemorySize);

  {
    Model model = { 0 };
    LoadModel("./data/source/", "muro.obj", &pool, &model);

    char dummy = 0;

  }
}

local bool GameUpdate(
    float /* deltaTime */,
    void* /* gameMemory */,
    u32 /* gameMemorySize */,
    RenderTarget* /* renderTarget */,
    bool* /* kbState */)
{
  // NOTE: converter does all stuff on GameInitialize(...)
  return false;
}
