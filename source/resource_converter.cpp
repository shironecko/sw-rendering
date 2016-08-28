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

u32 StringCat(char *dest, const char *src, u32 dest_len)
{
    assert(dest);
    assert(src);
    char *p = dest;
    while (*p) { ++p; }

    return p - dest + StringCopy(p, src, dest_len - (p - dest));
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

u32 ParseBitmap(
    const char* resourcePath,
    const char* bmpName,
    Texture *outTexture,
    MemPool *pool)
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

  StringCopy(outTexture->name, bmpName, sizeof(outTexture->name));
  outTexture->width = infoHeader->width;
  outTexture->height = infoHeader->height;

  BitmapColor32* rawPixels = (BitmapColor32*)(rawBitmap + fileHeader->offBytes);
  u32 texelsSize = sizeof(*outTexture->texels) * outTexture->width * outTexture->height;
  outTexture->texels = (Color32*)MemPush(pool, texelsSize);
  for (u32 i = 0, n = outTexture->width * outTexture->height; i < n; ++i)
  {
    BitmapColor32 rawPixel = rawPixels[i];
    outTexture->texels[i] = { rawPixel.r, rawPixel.g, rawPixel.b, 0 };
  }

  pool->hiPtr = initialHiPtr;
  return texelsSize;
}

/*
 * Returns number of materials loaded
 */
u32 ParseMtl(const char *resourcePath, const char *mtlName, Material *materials, MemPool *pool)
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
            ParseBitmap(resourcePath, bmpName, tex, pool);

            materials[materialIdx].diffuse = tex;
        }
        else if (StringBeginsWith(text, "bump "))
        {
            text += StringLen("bump ");
            char bmpName[PATH_LEN];
            *bmpName = 0;
            StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, " \n");
            
            Texture *tex = (Texture*)MemPush(pool, sizeof(*tex));
            ParseBitmap(resourcePath, bmpName, tex, pool);

            materials[materialIdx].bump = tex;
        }
        else if (StringBeginsWith(text, "map_Ks "))
        {
            text += StringLen("map_Ks ");
            char bmpName[PATH_LEN];
            *bmpName = 0;
            StringCopyPred(bmpName, text, sizeof(bmpName), StringPredCharNotInList, " \n");
            
            Texture *tex = (Texture*)MemPush(pool, sizeof(*tex));
            ParseBitmap(resourcePath, bmpName, tex, pool);

            materials[materialIdx].specular = tex;
        }
    }

    pool->hiPtr = initialHiPtr;
    return matCount;
}

struct LoadedObj
{
    Mesh mesh;
    Material *materials;
    u32 materialsCount;
    Texture *textures;
    u32 texturesCount;
};

LoadedObj ParseMeshObj(
    const char* resourcePath,
    const char* objName,
    MemPool *pool)
{
  // TODO: same thing, clunky
  char objPath[PATH_LEN];
  *objPath = 0;
  StringCat(objPath, resourcePath, sizeof(objPath));
  StringCat(objPath, objName, sizeof(objPath));
  u32 fileSize = (u32)PlatformGetFileSize(objPath);
  assert(fileSize);

  u8* initialHiPtr = pool->hiPtr;
  char* objText = (char*)MemPushBack(pool, fileSize);
  PlatformLoadFile(objPath, objText, fileSize);

  Vector4 *vertices, *verticesOriginal;
  vertices = verticesOriginal = (Vector4*)MemPushBack(pool, fileSize);
  Vector2 *uvs, *uvsOriginal; 
  uvs = uvsOriginal = (Vector2*)MemPushBack(pool, fileSize);
  Vector4 *normales, *normalesOriginal; 
  normales = normalesOriginal = (Vector4*)MemPushBack(pool, fileSize);
  Material *materials, *materialsOriginal;
  materials = materialsOriginal = (Material*)MemPushBack(pool, fileSize);

  const u32 maxFacesGroups = 20;
  FacesGroup *facesGroups, *facesGroupsOriginal;
  facesGroups = facesGroupsOriginal = (FacesGroup*)MemPushBack(pool, maxFacesGroups * sizeof(*facesGroups));
  facesGroups->faces = (MeshFace*)MemPushBack(pool, fileSize);
  facesGroups->facesCount = 0;
  facesGroups->material = 0;

  for (char* text = objText; u32(text - objText) < fileSize; text += SkipLine(text))
  {
    if (StringBeginsWith(text, "mtllib "))
    {
        assert(!(materials - materialsOriginal));
        char* localText = text + 7;
        char mtlName[PATH_LEN];
        StringCopyPred(mtlName, localText, sizeof(mtlName), StringPredCharNotInList, "\n");
        materials += ParseMtl(resourcePath, mtlName, materials, pool);
    }
    else if (StringBeginsWith(text, "usemtl "))
    {
        char* localText = text + StringLen("usemtl ");
        char materialName[RC_NAME_LEN];
        StringCopyPred(materialName, localText, sizeof(materialName), StringPredCharNotInList, " \n");
        s32 materialIndex = -1;
        for (u32 i = 0; i < (u32)(materials - materialsOriginal); ++i)
        {
            if (StringCompare(materialName, materialsOriginal[i].name))
            {
                materialIndex = i;
                break;
            }
        }

        assert(materialIndex != -1);
        if (facesGroups->facesCount)
        {
            ++facesGroups;
            facesGroups->faces = (MeshFace*)MemPushBack(pool, fileSize);
            facesGroups->facesCount = 0;
        }

        facesGroups->material = materialsOriginal + materialIndex;
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

      facesGroups->faces[facesGroups->facesCount++] = face;

      if (IsNumber(*localText))
      {
          // triangulate a quad
          face.vertices[1] = face.vertices[2];
          face.uvs[1] = face.uvs[2];
          face.normals[1] = face.normals[2];
          localText += ParseUInteger(localText, &face.vertices[2]) + 1;
          --face.vertices[2];
          localText += ParseUInteger(localText, &face.uvs[2]) + 1;
          --face.uvs[2];
          localText += ParseUInteger(localText, &face.normals[2]) + 1;
          --face.normals[2];

          facesGroups->faces[facesGroups->facesCount++] = face;
      }

      assert(facesGroups->facesCount * sizeof(*facesGroups->faces) <= fileSize);
    }
    else if (StringBeginsWith(text, "v "))
    {
      // vertex
      Vector4 vertice;
      ParseVector3(text + 2, &vertice);
      vertice.w = 1.0f;

      *vertices = vertice;
      ++vertices;
      assert((vertices - verticesOriginal) * sizeof(*vertices) <= fileSize);
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
      assert((uvs - uvsOriginal) * sizeof(*uvs) <= fileSize);
    }
    else if (StringBeginsWith(text, "vn "))
    {
      // normal
      Vector4 normale;
      ParseVector3(text + 3, &normale);
      normale.w = 0.0f;

      *normales = normale;
      ++normales;
      assert((normales - normalesOriginal) * sizeof(*normales) <= fileSize);
    }
  }

  LoadedObj result = { 0 };
  Mesh *mesh = &result.mesh;
  StringCopy(mesh->name, objName, sizeof(mesh->name));

  mesh->verticesCount = vertices - verticesOriginal;
  u32 verticesBytes = mesh->verticesCount * sizeof(*mesh->vertices);
  mesh->vertices = (Vector4*)MemPush(pool, verticesBytes);
  MemoryCopy(mesh->vertices, verticesOriginal, verticesBytes);

  mesh->uvsCount = uvs - uvsOriginal;
  u32 uvsBytes = mesh->uvsCount * sizeof(*mesh->uvs);
  mesh->uvs = (Vector2*)MemPush(pool, uvsBytes);
  MemoryCopy(mesh->uvs, uvsOriginal, uvsBytes);

  mesh->normalesCount = normales - normalesOriginal;
  u32 normalesBytes = mesh->normalesCount * sizeof(*mesh->normales);
  mesh->normales = (Vector4*)MemPush(pool, normalesBytes);
  MemoryCopy(mesh->normales, normalesOriginal, normalesBytes);

  mesh->facesGroupsCount = facesGroups - facesGroupsOriginal;
  u32 facesGroupsBytes = mesh->facesGroupsCount * sizeof(*mesh->facesGroups);
  mesh->facesGroups = (FacesGroup*)MemPush(pool, facesGroupsBytes);
  MemoryCopy(mesh->facesGroups, facesGroupsOriginal, facesGroupsBytes);

  result.materialsCount = materials - materialsOriginal;
  u32 materialsBytes = result.materialsCount * sizeof(*result.materials);
  result.materials = (Material*)MemPush(pool, materialsBytes);
  MemoryCopy(result.materials, materialsOriginal, materialsBytes);

  for (u32 i = 0; i < result.materialsCount; ++i)
  {
      result.texturesCount += result.materials[i].diffuse ? 1 : 0;
      result.texturesCount += result.materials[i].bump ? 1 : 0;
      result.texturesCount += result.materials[i].specular ? 1 : 0;
  }

  u32 texturesBytes = result.texturesCount * sizeof(*result.textures);
  result.textures = (Texture*)MemPush(pool, texturesBytes);
  Texture *t = result.textures;
  for (u32 i = 0; i < result.materialsCount; ++i)
  {
      if (result.materials[i].diffuse) { MemoryCopy(t++, result.materials[i].diffuse, sizeof(*t)); }
      if (result.materials[i].bump) { MemoryCopy(t++, result.materials[i].bump, sizeof(*t)); }
      if (result.materials[i].specular) { MemoryCopy(t++, result.materials[i].specular, sizeof(*t)); }
  }

  pool->hiPtr = initialHiPtr;
  return result;
}

/* bool BmpToTex(const char* bmpPath, const char* texPath, void* memory, u32 memorySize) */
/* { */
/*     Texture* texture = (Texture*)memory; */
/*     u32 textureSize = ParseBitmap( */
/*         bmpPath, */
/*         (u8*)memory, */
/*         memorySize); */

/*     bool writeResult = PlatformWriteFile( */
/*         texPath, */
/*         (u8*)texture + Texture::offset_to_serializable_data, */
/*         textureSize - Texture::offset_to_serializable_data); */

/*     return writeResult; */
/* } */

local void GameInitialize(void* gameMemory, u32 gameMemorySize)
{
  MemPool pool = NewMemPool(gameMemory, gameMemorySize);

  {
    LoadedObj obj = ParseMeshObj(
        "./data/source/",
        "muro.obj",
        &pool);

    char dummy = 0;

    /* bool writeResult = PlatformWriteFile( */
    /*     "./data/cooked/muro.mesh", */
    /*     (u8*)mesh + Mesh::offset_to_serializable_data, */
    /*     meshSize - Mesh::offset_to_serializable_data); */

    /* assert(writeResult); */
  }

  {
    /* bool result = BmpToTex("./data/source/muro_body_dm.bmp", "./data/cooked/muro_body_dm.tex", memory, gameMemorySize); */
    /* assert(result); */
    /* result = BmpToTex("./data/source/muro_body_nm.bmp", "./data/cooked/muro_body_nm.tex", memory, gameMemorySize); */
    /* assert(result); */
    /* result = BmpToTex("./data/source/muro_body_sm.bmp", "./data/cooked/muro_body_sm.tex", memory, gameMemorySize); */
    /* assert(result); */
    /* result = BmpToTex("./data/source/muro_head_dm.bmp", "./data/cooked/muro_head_dm.tex", memory, gameMemorySize); */
    /* assert(result); */
    /* result = BmpToTex("./data/source/muro_head_nm.bmp", "./data/cooked/muro_head_nm.tex", memory, gameMemorySize); */
    /* assert(result); */
    /* result = BmpToTex("./data/source/muro_head_sm.bmp", "./data/cooked/muro_head_sm.tex", memory, gameMemorySize); */
    /* assert(result); */
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
