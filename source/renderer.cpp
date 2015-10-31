#include <cassert>

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

  Color& operator () (u32 x, u32 y)
  {
    assert(x < m_width);
    assert(y < m_height);

    return m_pixels[y * m_width + x];
  }

  int Width() { return m_width; }
  int Height() { return m_height; }

private:
  u32 m_width;
  u32 m_height;
  Color* m_pixels;
};

void Render(Bitmap& bitmap)
{
  const Color clearColor { .8f, .3f, .6f, 1.0f };
  for (int y = 0; y < bitmap.Height(); ++y)
  {
    for (int x = 0; x < bitmap.Width(); ++x)
    {
      bitmap(x, y) = clearColor;
    }
  }
}
