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

void Render(Bitmap& bitmap)
{
  const Color clearColor { .8f, .3f, .6f, 1.0f };
  for (u32 y = 0; y < bitmap.Height(); ++y)
  {
    for (u32 x = 0; x < bitmap.Width(); ++x)
    {
      bitmap(x, y) = clearColor;
    }
  }

  // horizontal lines
  DrawLine(200, 200, 300, 250, { 1.0f, 1.0f, 1.0f, 1.0f }, bitmap);
  DrawLine(200, 200, 300, 200, { 1.0f, 1.0f,    0, 1.0f }, bitmap);
  DrawLine(200, 200, 100, 250, { 1.0f,    0, 1.0f, 1.0f }, bitmap);
  DrawLine(200, 200, 100, 200, {    0, 1.0f,    0, 1.0f }, bitmap);
  DrawLine(200, 200, 300, 150, { 1.0f,    0,    0, 1.0f }, bitmap);
  DrawLine(200, 200, 100, 150, { 0.5f, 0.5f, 0.5f, 1.0f }, bitmap);

  // vertical lines
  DrawLine(200, 200, 200, 300, {    0,    0,    0, 1.0f }, bitmap);
  DrawLine(200, 200, 200, 100, { 0.3f, 0.8f, 0.5f, 1.0f }, bitmap);
  DrawLine(200, 200, 250, 300, { 0.8f, 0.8f, 0.5f, 1.0f }, bitmap);
  DrawLine(200, 200, 150, 300, { 0.8f, 0.8f, 0.5f, 1.0f }, bitmap);
  DrawLine(200, 200, 150, 100, { 0.8f, 0.8f, 0.8f, 1.0f }, bitmap);
  DrawLine(200, 200, 250, 100, { 0.1f, 0.2f, 0.7f, 1.0f }, bitmap);
}
