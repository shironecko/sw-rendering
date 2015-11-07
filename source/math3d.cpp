#include <cmath>
#include <cassert>

extern "C"
{
  #include "pIII_matrix_inverse.c"
}

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
  union
  {
    float components[16];
    Vector4 rows[4];
  };

  float& operator()(u32 row, u32 col)
  {
    assert(row < 4 && col < 4);
    return components[row * 4 + col];
  }

  const float& operator()(u32 row, u32 col) const
  {
    assert(row < 4 && col < 4);
    return components[row * 4 + col];
  }

  Vector4 Row(u32 row) const
  {
    return rows[row];
  }

  Vector4 Col(u32 column) const
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
      Dot4(vec, Row(0)),
      Dot4(vec, Row(1)),
      Dot4(vec, Row(2)),
      Dot4(vec, Row(3))
    };
  }

  Matrix4x4 operator * (const Matrix4x4& mat) const
  {
    Matrix4x4 result;
    for (u32 y = 0; y < 4; ++y)
    {
      Vector4 row = Row(y);
      for (u32 x = 0; x < 4; ++x)
      {
        Vector4 col = mat.Col(x);
        result(y, x) = Dot4(row, col);
      }
    }

    return result;
  }

  void operator *= (const Matrix4x4 that)
  {
    *this = *this * that;
  }

  Matrix4x4 Inverse() const
  {
    Matrix4x4 result = *this;
    PIII_Inverse_4x4(&result(0, 0));

    return result;
  }
};

