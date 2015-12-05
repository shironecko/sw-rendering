// TODO: sort this out
/* #include <array> */
/* #include <vector> */
/* #include <algorithm> */
/* #include <limits> */

struct Color
{
  union
  {
    float components[4];

    struct
    {
      float r;
      float g;
      float b;
      float a;
    };
  };
};

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
  Vector4* vertices;
  u32 verticesCount;
  Vector2* uvs;
  u32 uvsCount;
  Vector4* normales;
  u32 normalesCount;
  MeshFace* faces;
  u32 facesCount;
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

struct Bitmap
{
  Color GetPixel(u32 x, u32 y)
  {
    assert(x < width);
    assert(y < height);

    return memory[y * width + x];
  }

  void SetPixel(u32 x, u32 y, Color value)
  {
    assert(x < width);
    assert(y < height);

    memory[y * width + x] = value;
  }

  u32 width;
  u32 height;
  Color* memory;
};

// TODO: sort this out
void my_swap(i32& a, i32& b)
{
  i32 tmp = a;
  a = b;
  b = tmp;
}

u32 my_abs(i32 x)
{
  return x > 0 ? x : -x;
}

void DrawLine(i32 x1, i32 y1, i32 x2, i32 y2, Color color, Bitmap* bitmap)
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
    for (i32 x = x1; x <= x2; ++x)
    {
      bitmap->SetPixel(x, u32(y), color);
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
    for (i32 y = y1; y <= y2; ++y)
    {
      bitmap->SetPixel(u32(x), y, color);
      x += step;
    }
  }
}

struct RenderTarget
{
  Bitmap bitmap;
  float* zBuffer;
};

// TODO: sort this out
void Render(
    RenderTarget* target,
    u32 renderMode,
    Mesh* mesh,
    Color32* colorTexture,
    u32 colorTextureWidth,
    u32 colorTextureHeight,
    float camDistance, float camRotY,
    Vector4 sunlightDirection)
{
  Bitmap* bitmap = &target->bitmap;
  float* zBuffer = target->zBuffer;

  const Color clearColor { 0, 0, 0, 0};
  for (u32 y = 0; y < bitmap->height; ++y)
  {
    for (u32 x = 0; x < bitmap->width; ++x)
    {
      bitmap->SetPixel(x, y, clearColor);
    }
  }

  float infinity = 100.0f;
  for (u32 i = 0, n = bitmap->width * bitmap->height; i < n; ++i)
    zBuffer[i] = infinity;

  Matrix4x4 model = Matrix4x4::Identity();

  Vector4 camPos { 0, 0, camDistance, 1.0f };
  camPos = Matrix4x4::RotationY(camRotY) * camPos;
  Matrix4x4 view = Matrix4x4::LookAtCamera( 
      camPos,
      {    0,    0,    0, 1.0f },
      {    0, 1.0f,    0,    0 });

  Matrix4x4 projection = Matrix4x4::Projection(
      90.0f, 
      float(bitmap->width) / float(bitmap->height),
      0.1f,
      1000.0f);

  Matrix4x4 screen = Matrix4x4::ScreenSpace( bitmap->width, bitmap->height); 
  Matrix4x4 transform = projection * view * model;

  for (u32 i = 0; i < mesh->facesCount; ++i)
  {
    MeshFace face = mesh->faces[i];
    Vector4 verts[3];
    verts[0] = transform * mesh->vertices[face.vertices[0]];
    verts[1] = transform * mesh->vertices[face.vertices[1]];
    verts[2] = transform * mesh->vertices[face.vertices[2]];

    bool isInsideFrustrum = true;
    for (Vector4& vert: verts)
    {
      if (vert.x > vert.w || vert.x < -vert.w ||
          vert.y > vert.w || vert.y < -vert.w ||
          vert.z > vert.w || vert.z < -vert.w)
      {
        isInsideFrustrum = false;
        break;
      }

      vert = vert / vert.w;
      vert = screen * vert;
    }

    if (!isInsideFrustrum)
      continue;

    u32 x1 = u32(verts[0].x);
    u32 y1 = u32(verts[0].y);
    u32 x2 = u32(verts[1].x);
    u32 y2 = u32(verts[1].y);
    u32 x3 = u32(verts[2].x);
    u32 y3 = u32(verts[2].y);

    /* if (renderMode & (RenderMode::Shaded | RenderMode::Textured)) */
    /* { */
    /*   using std::min; */
    /*   using std::max; */
    /*   u32 minX = min(x1, min(x2, x3)); */
    /*   u32 minY = min(y1, min(y2, y3)); */
    /*   u32 maxX = max(x1, max(x2, x3)) + 1; */
    /*   u32 maxY = max(y1, max(y2, y3)) + 1; */

    /*   Vector4 norms[3]; */
    /*   norms[0] = normales[face.normals[0]]; */
    /*   norms[1] = normales[face.normals[1]]; */
    /*   norms[2] = normales[face.normals[2]]; */
    /*   float lum[3]; */
    /*   lum[0] = Dot3(norms[0], sunlightDirection); */
    /*   lum[1] = Dot3(norms[1], sunlightDirection); */
    /*   lum[2] = Dot3(norms[2], sunlightDirection); */

    /*   struct Vector2 */
    /*   { */
    /*     union */
    /*     { */
    /*       float components[2]; */
    /*       struct */
    /*       { */
    /*         float x; */
    /*         float y; */
    /*       }; */
    /*     }; */

    /*     float Dot(Vector2 other) const */
    /*     { */
    /*       return x * other.x + y * other.y; */
    /*     } */

    /*     Vector2 operator - (Vector2 other) const */
    /*     { */
    /*       return Vector2 { x - other.x, y - other.y }; */
    /*     } */
    /*   }; */

    /*   std::array<float, 2> faceUvs[3]; */
    /*   faceUvs[0] = uvs[face.uvs[0]]; */
    /*   faceUvs[1] = uvs[face.uvs[1]]; */
    /*   faceUvs[2] = uvs[face.uvs[2]]; */

    /*   Vector2 a { float(x1), float(y1) }; */
    /*   Vector2 b { float(x2), float(y2) }; */
    /*   Vector2 c { float(x3), float(y3) }; */

    /*   Vector2 v0 = b - a; */
    /*   Vector2 v1 = c - a; */

    /*   for (u32 x = minX; x < maxX; ++x) */
    /*   { */
    /*     for (u32 y = minY; y < maxY; ++y) */
    /*     { */
    /*       // calculate barycentric coords... */
    /*       Vector2 p { float(x), float(y) }; */
    /*       Vector2 v2 = p - a; */

    /*       float d00 = v0.Dot(v0); */
    /*       float d01 = v0.Dot(v1); */
    /*       float d11 = v1.Dot(v1); */
    /*       float d20 = v2.Dot(v0); */
    /*       float d21 = v2.Dot(v1); */

    /*       float denom = d00 * d11 - d01 * d01; */

    /*       float v = (d11 * d20 - d01 * d21) / denom; */
    /*       float w = (d00 * d21 - d01 * d20) / denom; */
    /*       float u = 1.0f - v - w; */

    /*       if (!(v >= -0.001 && w >= -0.001 && u >= -0.001)) */
    /*         continue; */

    /*       u32 zIndex = y * bitmap->width + x; */
    /*       float z = verts[1].z * v + verts[2].z * w + verts[0].z * u; */
    /*       if (zBuffer[zIndex] > z) */
    /*       { */
    /*         float l = lum[1] * v + lum[2] * w + lum[0] * u; */
    /*         if (l < 0) */
    /*           l = 0; */
    /*         else if (l > 1.0f) */
    /*           l = 1.0f; */

    /*         float tu = faceUvs[1][0] * v + faceUvs[2][0] * w + faceUvs[0][0] * u; */
    /*         float tv = faceUvs[1][1] * v + faceUvs[2][1] * w + faceUvs[0][1] * u; */
    /*         tu *= colorTextureWidth; */
    /*         tv *= colorTextureHeight; */
    /*         Color32 texel = colorTexture[u32(tv) * colorTextureWidth + u32(tu)]; */
    /*         bitmap->SetPixel(x, y, Color */ 
    /*         { */ 
    /*           float(texel.r) / 255.0f * l, */
    /*           float(texel.g) / 255.0f * l, */
    /*           float(texel.b) / 255.0f * l, */
    /*           1.0f */
    /*         }); */

    /*         zBuffer[zIndex] = z; */
    /*       } */
    /*     } */
    /*   } */
    /* } */

    if (renderMode & RenderMode::Wireframe)
    {
      const Color modelColor { 1.0f, 1.0f, 1.0f, 1.0f };

      DrawLine(x1, y1, x2, y2, modelColor, bitmap);
      DrawLine(x2, y2, x3, y3, modelColor, bitmap);
      DrawLine(x3, y3, x1, y1, modelColor, bitmap);
    }
  }
}
