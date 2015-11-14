#include <vector>
#include <cassert>
#include <cstdlib>
#include <algorithm>
#include <limits>

struct Color
{
  union
  {
    float component[4];

    struct
    {
      float r;
      float g;
      float b;
      float a;
    };
  };
};

struct ModelFace
{
  u32 vertices[3];
  u32 uvs[3];
  u32 normals[3];
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

class Bitmap
{
public:
  Bitmap(u32 width, u32 height) :
    m_width(width),
    m_height(height)
  {
    m_pixels = new Color[width * height];
  }

  ~Bitmap()
  {
    delete[] m_pixels;
  }

  Color& operator () (u32 x, u32 y)
  {
    assert(x < m_width);
    assert(y < m_height);

    return m_pixels[y * m_width + x];
  }

  u32 Width() { return m_width; }
  u32 Height() { return m_height; }

private:
  u32 m_width;
  u32 m_height;
  Color* m_pixels;
};

void DrawLine(i32 x1, i32 y1, i32 x2, i32 y2, Color color, Bitmap& bitmap)
{
  using std::abs;
  using std::swap;

  if (abs(x2 - x1) > abs(y2 - y1)) // horizontal line
  {
    if (x1 > x2)
    {
      swap(x1, x2);
      swap(y1, y2);
    }

    float y = float(y1);
    float step = float(y2 - y1) / float(x2 - x1);
    for (i32 x = x1; x <= x2; ++x)
    {
      bitmap(x, u32(y)) = color;
      y += step;
    }
  }
  else // vertical line
  {
    if (y1 > y2)
    {
      swap(y1, y2);
      swap(x1, x2);
    }

    float x = float(x1);
    float step = float(x2 - x1) / float(y2 - y1);
    for (i32 y = y1; y <= y2; ++y)
    {
      bitmap(u32(x), y) = color;
      x += step;
    }
  }
}

void Render(
    Bitmap& bitmap,
    std::vector<float>& zBuffer,
    u32 renderMode,
    std::vector<Vector4>& vertices,
    std::vector<std::array<float, 2>>& uvs,
    std::vector<Vector4>& normales,
    std::vector<ModelFace>& faces,
    float camDistance, float camRotY,
    Vector4 sunlightDirection)
{
  const Color clearColor { 0, 0, 0, 0};
  for (u32 y = 0; y < bitmap.Height(); ++y)
  {
    for (u32 x = 0; x < bitmap.Width(); ++x)
    {
      bitmap(x, y) = clearColor;
    }
  }

  for (u32 i = 0, n = bitmap.Width() * bitmap.Height(); i < n; ++i)
    zBuffer[i] = std::numeric_limits<float>::infinity();

  const Color modelColor { 1.0f, 1.0f, 1.0f, 1.0f };

  Matrix4x4 model = Matrix4x4::Identity();

  Vector4 camPos { 0, 0, camDistance, 1.0f };
  camPos = Matrix4x4::RotationY(camRotY) * camPos;
  Matrix4x4 view = Matrix4x4::LookAtCamera( 
      camPos,
      {    0,    0,    0, 1.0f },
      {    0, 1.0f,    0,    0 });

  Matrix4x4 projection = Matrix4x4::Projection(
      90.0f, 
      float(bitmap.Width()) / float(bitmap.Height()),
      0.1f,
      1000.0f);

  Matrix4x4 screen = Matrix4x4::ScreenSpace( bitmap.Width(), bitmap.Height()); 
  Matrix4x4 transform = projection * view * model;

  for (auto& face: faces)
  {
    Vector4 verts[3];
    verts[0] = transform * vertices[face.vertices[0]];
    verts[1] = transform * vertices[face.vertices[1]];
    verts[2] = transform * vertices[face.vertices[2]];

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

    if (renderMode & (RenderMode::Shaded | RenderMode::Textured))
    {
      using std::min;
      using std::max;
      u32 minX = min(x1, min(x2, x3));
      u32 minY = min(y1, min(y2, y3));
      u32 maxX = max(x1, max(x2, x3)) + 1;
      u32 maxY = max(y1, max(y2, y3)) + 1;

      Vector4 norms[3];
      norms[0] = normales[face.normals[0]];
      norms[1] = normales[face.normals[1]];
      norms[2] = normales[face.normals[2]];
      float lum[3];
      lum[0] = Dot3(norms[0], sunlightDirection);
      lum[1] = Dot3(norms[1], sunlightDirection);
      lum[2] = Dot3(norms[2], sunlightDirection);

      struct Vector2
      {
        union
        {
          float components[2];
          struct
          {
            float x;
            float y;
          };
        };

        float Dot(Vector2 other) const
        {
          return x * other.x + y * other.y;
        }

        Vector2 operator - (Vector2 other) const
        {
          return Vector2 { x - other.x, y - other.y };
        }
      };

      Vector2 a { float(x1), float(y1) };
      Vector2 b { float(x2), float(y2) };
      Vector2 c { float(x3), float(y3) };

      Vector2 v0 = b - a;
      Vector2 v1 = c - a;

      for (u32 x = minX; x < maxX; ++x)
      {
        for (u32 y = minY; y < maxY; ++y)
        {
          // calculate barycentric coords...
          Vector2 p { float(x), float(y) };
          Vector2 v2 = p - a;

          float d00 = v0.Dot(v0);
          float d01 = v0.Dot(v1);
          float d11 = v1.Dot(v1);
          float d20 = v2.Dot(v0);
          float d21 = v2.Dot(v1);

          float denom = d00 * d11 - d01 * d01;

          float v = (d11 * d20 - d01 * d21) / denom;
          float w = (d00 * d21 - d01 * d20) / denom;
          float u = 1.0f - v - w;

          if (!(v >= -0.001 && w >= -0.001 && u >= -0.001))
            continue;

          u32 zIndex = y * bitmap.Width() + x;
          float z = verts[1].z * v + verts[2].z * w + verts[0].z * u;
          if (zBuffer[zIndex] > z)
          {
            float l = lum[1] * v + lum[2] * w + lum[0] * u;
            if (l < 0)
              l = 0;
            else if (l > 1.0f)
              l = 1.0f;

            bitmap(x, y) = Color { l, l, l, 1.0f };
            zBuffer[zIndex] = z;
          }
        }
      }
    }

    if (renderMode & RenderMode::Wireframe)
    {
      DrawLine(x1, y1, x2, y2, modelColor, bitmap);
      DrawLine(x2, y2, x3, y3, modelColor, bitmap);
      DrawLine(x3, y3, x1, y1, modelColor, bitmap);
    }
  }
}
