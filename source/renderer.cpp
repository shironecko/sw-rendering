#include "platform_api.h"

struct Color32
{
  union
  {
    u8 components[4];

    struct
    {
      u8 r;
      u8 g;
      u8 b;
      u8 padding;
    };
  };
};

struct MeshFace
{
  u32 vertices[3];
  u32 uvs[3];
  u32 normals[3];
};

struct Mesh
{
  enum
  {
    offset_to_serializable_data = sizeof(void*) * 4,
  };

  Vector4* vertices;
  Vector2* uvs;
  Vector4* normales;
  MeshFace* faces;

  u32 verticesCount;
  u32 uvsCount;
  u32 normalesCount;
  u32 facesCount;
};

struct Texture
{
  enum
  {
    offset_to_serializable_data = sizeof(void*),
  };

  Color32 *texels;
  u32 width;
  u32 height;

  Color32 GetTexel(u32 x, u32 y)
  {
    assert(x < width);
    assert(y < height);

    return *(texels + y * width + x);
  }

  void SetTexel(u32 x, u32 y, Color32 color)
  {
    assert(x < width);
    assert(y < height);

    *(texels + y * width + x) = color;
  }
};

namespace RenderMode
{
  enum
  {
    BackfaceCulling = 1 << 0,
    Shaded          = 1 << 1,
    Textured        = 1 << 2,
    Wireframe       = 1 << 3,
  };
}

// TODO: sort this out
void my_swap(s32& a, s32& b)
{
  s32 tmp = a;
  a = b;
  b = tmp;
}

u32 my_abs(s32 x)
{
  return x > 0 ? x : -x;
}

template<typename T>
T my_min(T a, T b)
{
  return a < b ? a : b;
}

template<typename T>
T my_max(T a, T b)
{
  return a > b ? a : b;
}

void DrawLine(s32 x1, s32 y1, s32 x2, s32 y2, Color32 color, Texture* texture)
{
  if (my_abs(x2 - x1) > my_abs(y2 - y1)) // horizontal line
  {
    if (x1 > x2)
    {
      my_swap(x1, x2);
      my_swap(y1, y2);
    }

    float y = float(y1);
    float step = float(y2 - y1) / float(x2 - x1);
    for (s32 x = x1; x <= x2; ++x)
    {
      texture->SetTexel(x, u32(y), color);
      y += step;
    }
  }
  else // vertical line
  {
    if (y1 > y2)
    {
      my_swap(y1, y2);
      my_swap(x1, x2);
    }

    float x = float(x1);
    float step = float(x2 - x1) / float(y2 - y1);
    for (s32 y = y1; y <= y2; ++y)
    {
      texture->SetTexel(u32(x), y, color);
      x += step;
    }
  }
}

struct RenderTarget
{
  Texture* texture;
  float* zBuffer;
};

void ClearRenderTarget(
    RenderTarget* target,
    Color32 clearColor)
{
  Texture* targetTexture = target->texture;
  float* zBuffer = target->zBuffer;

  for (u32 y = 0; y < targetTexture->height; ++y)
  {
    for (u32 x = 0; x < targetTexture->width; ++x)
    {
      targetTexture->SetTexel(x, y, clearColor);
    }
  }

  float infinity = 100.0f;
  for (u32 i = 0, n = targetTexture->width * targetTexture->height; i < n; ++i)
    zBuffer[i] = infinity;
}

// TODO: sort this out
void Render(
    RenderTarget* target,
    u32 renderMode,
    Mesh* mesh,
    Texture* colorTexture,
    Matrix4x4 MVP,
    Matrix4x4 screenMatrix,
    Vector4 sunlightDirection,
    Color32 sunlightColor,
    void* tmpMemory,
    u32 tmpMemorySize)
{
  Texture* targetTexture = target->texture;
  float* zBuffer = target->zBuffer;

  u8* memory = (u8*)align(tmpMemory);
  u8* memoryEnd = (u8*)tmpMemory + tmpMemorySize;

  Vector4* vertices = (Vector4*)memory;
  memory += sizeof(Vector4) * mesh->verticesCount;
  assert(memory <= memoryEnd);

  for (u32 i = 0, e = mesh->verticesCount; i < e; ++i)
  {
    vertices[i] = MVP * mesh->vertices[i];
  }

  MeshFace* faces = (MeshFace*)memory;
  u32 facesToDraw = 0;
  memory += sizeof(MeshFace) * mesh->facesCount;
  for (u32 i = 0, e = mesh->facesCount; i < e; ++i)
  {
    MeshFace face = mesh->faces[i];

    bool isInsideFrustrum = true;
    for (u32 j = 0; j < 3; ++j)
    {
      Vector4 vertex = vertices[face.vertices[j]];

      if (vertex.x > vertex.w || vertex.x < -vertex.w ||
          vertex.y > vertex.w || vertex.y < -vertex.w ||
          vertex.z > vertex.w || vertex.z < -vertex.w)
      {
        isInsideFrustrum = false;
        break;
      }
    }

    if (isInsideFrustrum)
    {
      faces[facesToDraw] = face;
      ++facesToDraw;
    }
  }

  // TODO: avoid computing irrelevant data (?)
  for (u32 i = 0; i < mesh->verticesCount; ++i)
  {
    Vector4 vertex = vertices[i];
    vertex = vertex / vertex.w;
    vertex = screenMatrix * vertex;
    vertices[i] = vertex;
  }

  if (renderMode & RenderMode::Wireframe)
  {
    for (u32 i = 0; i < facesToDraw; ++i)
    {
      const Color32 modelColor{ 255, 255, 255, 255 };

      Vector4 v1 = vertices[faces[i].vertices[0]];
      Vector4 v2 = vertices[faces[i].vertices[1]];
      Vector4 v3 = vertices[faces[i].vertices[2]];

      s32 x1 = (s32)v1.x;
      s32 y1 = (s32)v1.y;
      s32 x2 = (s32)v2.x;
      s32 y2 = (s32)v2.y;
      s32 x3 = (s32)v3.x;
      s32 y3 = (s32)v3.y;

      DrawLine(x1, y1, x2, y2, modelColor, targetTexture);
      DrawLine(x2, y2, x3, y3, modelColor, targetTexture);
      DrawLine(x3, y3, x1, y1, modelColor, targetTexture);
    }
  }

  /* for (u32 i = 0; i < mesh->facesCount; ++i) */
  /* { */
  /*   MeshFace face = mesh->faces[i]; */
  /*   Vector4 verts[3]; */
  /*   verts[0] = MVP * mesh->vertices[face.vertices[0]]; */
  /*   verts[1] = MVP * mesh->vertices[face.vertices[1]]; */
  /*   verts[2] = MVP * mesh->vertices[face.vertices[2]]; */

  /*   bool isInsideFrustrum = true; */
  /*   for (Vector4& vert: verts) */
  /*   { */
  /*     if (vert.x > vert.w || vert.x < -vert.w || */
  /*         vert.y > vert.w || vert.y < -vert.w || */
  /*         vert.z > vert.w || vert.z < -vert.w) */
  /*     { */
  /*       isInsideFrustrum = false; */
  /*       break; */
  /*     } */

  /*     vert = vert / vert.w; */
  /*     vert = screenMatrix * vert; */
  /*   } */

  /*   if (!isInsideFrustrum) */
  /*     continue; */

  /*   u32 x1 = u32(verts[0].x); */
  /*   u32 y1 = u32(verts[0].y); */
  /*   u32 x2 = u32(verts[1].x); */
  /*   u32 y2 = u32(verts[1].y); */
  /*   u32 x3 = u32(verts[2].x); */
  /*   u32 y3 = u32(verts[2].y); */

  /*   if (renderMode & (RenderMode::Shaded | RenderMode::Textured)) */
  /*   { */
  /*     u32 minX = my_min(x1, my_min(x2, x3)); */
  /*     u32 minY = my_min(y1, my_min(y2, y3)); */
  /*     u32 maxX = my_max(x1, my_max(x2, x3)) + 1; */
  /*     u32 maxY = my_max(y1, my_max(y2, y3)) + 1; */

  /*     Vector4 norms[3]; */
  /*     float lum[3]; */
  /*     if (renderMode & RenderMode::Shaded) */
  /*     { */
  /*       norms[0] = mesh->normales[face.normals[0]]; */
  /*       norms[1] = mesh->normales[face.normals[1]]; */
  /*       norms[2] = mesh->normales[face.normals[2]]; */
  /*       lum[0] = Dot3(norms[0], sunlightDirection); */
  /*       lum[1] = Dot3(norms[1], sunlightDirection); */
  /*       lum[2] = Dot3(norms[2], sunlightDirection); */
  /*     } */

  /*     Vector2 faceUvs[3]; */
  /*     faceUvs[0] = mesh->uvs[face.uvs[0]]; */
  /*     faceUvs[1] = mesh->uvs[face.uvs[1]]; */
  /*     faceUvs[2] = mesh->uvs[face.uvs[2]]; */

  /*     Vector2 a { float(x1), float(y1) }; */
  /*     Vector2 b { float(x2), float(y2) }; */
  /*     Vector2 c { float(x3), float(y3) }; */

  /*     Vector2 v0 = b - a; */
  /*     Vector2 v1 = c - a; */

  /*     for (u32 x = minX; x < maxX; ++x) */
  /*     { */
  /*       for (u32 y = minY; y < maxY; ++y) */
  /*       { */
  /*         // calculate barycentric coords... */
  /*         Vector2 p { float(x), float(y) }; */
  /*         Vector2 v2 = p - a; */

  /*         float d00 = v0.Dot(v0); */
  /*         float d01 = v0.Dot(v1); */
  /*         float d11 = v1.Dot(v1); */
  /*         float d20 = v2.Dot(v0); */
  /*         float d21 = v2.Dot(v1); */

  /*         float denom = d00 * d11 - d01 * d01; */

  /*         float v = (d11 * d20 - d01 * d21) / denom; */
  /*         float w = (d00 * d21 - d01 * d20) / denom; */
  /*         float u = 1.0f - v - w; */

  /*         if (!(v >= -0.001 && w >= -0.001 && u >= -0.001)) */
  /*           continue; */

  /*         u32 zIndex = y * targetTexture->width + x; */
  /*         float z = verts[1].z * v + verts[2].z * w + verts[0].z * u; */
  /*         if (zBuffer[zIndex] > z) */
  /*         { */
  /*           float l = 1.0f; */

  /*           if (renderMode & RenderMode::Shaded) */
  /*           { */
  /*             l = lum[1] * v + lum[2] * w + lum[0] * u; */
  /*             if (l < 0) */
  /*               l = 0; */
  /*             else if (l > 1.0f) */
  /*               l = 1.0f; */
  /*           } */

  /*           Color32 texel = { 255, 255, 255, 255 }; */

  /*           if (renderMode & RenderMode::Textured) */
  /*           { */
  /*             float tu = faceUvs[1].x * v + faceUvs[2].x * w + faceUvs[0].x * u; */
  /*             float tv = faceUvs[1].y * v + faceUvs[2].y * w + faceUvs[0].y * u; */
  /*             tu *= colorTexture->width; */
  /*             tv *= colorTexture->height; */
  /*             texel = colorTexture->GetTexel(u32(tu), u32(tv)); */
  /*           } */

  /*           targetTexture->SetTexel(x, y, Color32 */ 
  /*           { */ 
  /*             u8(texel.r * l), */
  /*             u8(texel.g * l), */
  /*             u8(texel.b * l), */
  /*             255 */
  /*           }); */

  /*           zBuffer[zIndex] = z; */
  /*         } */
  /*       } */
  /*     } */
  /*   } */

  /*   if (renderMode & RenderMode::Wireframe) */
  /*   { */
  /*     const Color32 modelColor{ 255, 255, 255, 255 }; */

  /*     DrawLine(x1, y1, x2, y2, modelColor, targetTexture); */
  /*     DrawLine(x2, y2, x3, y3, modelColor, targetTexture); */
  /*     DrawLine(x3, y3, x1, y1, modelColor, targetTexture); */
  /*   } */
  /* } */
}
