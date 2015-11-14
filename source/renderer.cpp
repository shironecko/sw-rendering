#include <vector>
#include <cassert>
#include <cstdlib>
#include <algorithm>

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

void Render(Bitmap& bitmap, std::vector<Vector4>& vertices, std::vector<ModelFace>& faces, float camDistance, float camRotY)
{
  const Color clearColor { 0, 0, 0, 0};
  for (u32 y = 0; y < bitmap.Height(); ++y)
  {
    for (u32 x = 0; x < bitmap.Width(); ++x)
    {
      bitmap(x, y) = clearColor;
    }
  }

  const Color modelColor { 1.0f, 1.0f, 1.0f, 1.0f };
  const float s = 1.0f;
  const Matrix4x4 scale
  {
        s,    0,    0,    0,
        0,    s,    0,    0,
        0,    0,    s,    0,
        0,    0,    0, 1.0f
  };
  Matrix4x4 model = scale;

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
    /* Vector4 test = vertices[face.vertices[0]]; */
    /* test = model * test; */
    /* test = view * test; */
    /* test = projection * test; */
    /* test = test / test.w; */
    /* test = screen * test; */

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

    DrawLine(x1, y1, x2, y2, modelColor, bitmap);
    DrawLine(x2, y2, x3, y3, modelColor, bitmap);
    DrawLine(x3, y3, x1, y1, modelColor, bitmap);
  }
}
