#include <cmath>
#include <cassert>

struct Vector4
{
  union
  {
    float components[4];
    
    struct
    {
      float x;
      float y;
      float z;
      float w;
    };
  };

  float& operator[](u32 i)
  {
    assert(i < 4);
    return components[i];
  }

  float operator[](u32 i) const
  {
    assert(i < 4);
    return components[i];
  }
};

float Dot3(const Vector4& a, const Vector4& b)
{
  assert(a.w == 0 && b.w == 0);
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Dot4(const Vector4& a, const Vector4& b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

struct Matrix4x4
{
  Vector4 rows[4];

  Vector4& operator [] (u32 row)
  {
    assert(row < 4);
    return rows[row];
  }

  const Vector4& operator [] (u32 row) const
  {
    assert(row < 4);
    return rows[row];
  }

  Vector4 Column(u32 column) const
  {
    return Vector4
    {
      rows[0][column],
      rows[1][column],
      rows[2][column],
      rows[3][column]
    };
  }

  Vector4 operator * (const Vector4& vec) const
  {
    return Vector4
    {
      Dot4(vec, rows[0]),
      Dot4(vec, rows[1]),
      Dot4(vec, rows[2]),
      Dot4(vec, rows[3])
    };
  }

  Matrix4x4 operator * (const Matrix4x4& mat) const
  {
    Matrix4x4 result;
    for (u32 y = 0; y < 4; ++y)
    {
      const Vector4& row = rows[y];
      for (u32 x = 0; x < 4; ++x)
      {
        Vector4 col = mat.Column(x);
        result[y][x] = Dot4(row, col);
      }
    }

    return result;
  }
};
