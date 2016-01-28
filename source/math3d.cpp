#include "platform_api.h"
#include <cmath>
#include "pIII_matrix_inverse.c"

const float PI = float(3.14159265359);

float my_abs(float value)
{
  return value < 0 ? value * -1.0f : value;
}

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

  Vector4 operator / (float scalar) const
  {
    assert(scalar != 0);
    return Vector4 { x / scalar, y / scalar, z / scalar, w / scalar };
  }

  Vector4 operator - (const Vector4& that) const
  {
    return Vector4
    {
      x - that.x,
      y - that.y,
      z - that.z,
      w - that.w
    };
  }

  Vector4 operator - () const
  {
    return Vector4 { -x, -y, -z, -w };
  }

  Vector4 operator + (const Vector4& that) const
  {
    return (*this) - (-that);
  }

  float SqrLength3() const
  {
    assert(w == 0);
    return x * x + y * y + z * z;
  }

  float Length3() const
  {
    return sqrt(SqrLength3());
  }

  Vector4 Normalized3() const
  {
    assert(w == 0);
    return (*this) / Length3();
  }
};

float Dot3(const Vector4& a, const Vector4& b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Dot4(const Vector4& a, const Vector4& b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Vector4 Cross(const Vector4& a, const Vector4& b)
{
  return Vector4
  {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x,
    0
  };
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

  // TODO: sort this out
  /* Matrix4x4 Inverse() const */
  /* { */
  /*   Matrix4x4 result = *this; */
  /*   PIII_Inverse_4x4(&result(0, 0)); */

  /*   return result; */
  /* } */

  static Matrix4x4 Identity()
  {
    return Matrix4x4
    {
      1.0f,    0,    0,    0,
         0, 1.0f,    0,    0,
         0,    0, 1.0f,    0,
         0,    0,    0, 1.0f
    };
  }

  static Matrix4x4 RotationY(float angle)
  {
    // TODO: use intrinsics
    return Matrix4x4
    {
       (float)cos(angle), 0, (float)sin(angle), 0,
                0, 1.0f,      0, 0,
      (float)-sin(angle), 0, (float)cos(angle), 0,
                0, 0,      0, 1.0f
    };
  }

  static Matrix4x4 LookAtCamera(Vector4 eye, Vector4 target, Vector4 up)
  {
    assert(abs(up.Length3() - 1.0f) < 0.0001f);

    Vector4 zaxis = (eye - target).Normalized3();
    Vector4 xaxis = Cross(up, zaxis).Normalized3();
    Vector4 yaxis = Cross(zaxis, xaxis);

    return Matrix4x4
    {
      xaxis.x, xaxis.y, xaxis.z, -Dot3(eye, xaxis),
      yaxis.x, yaxis.y, yaxis.z, -Dot3(eye, yaxis),
      zaxis.x, zaxis.y, zaxis.z, -Dot3(eye, zaxis),
            0,       0,       0,              1.0f
    };
  }

  static Matrix4x4 Projection(float vfov, float aspect, float clipNear, float clipFar)
  {
    // field of view
    float y = 1.0f / tan(vfov * PI / 360.0f);
    float x = y / aspect;

    // map z to [0, 1]
    float zz = -clipFar / (clipFar - clipNear);
    float zw = -clipFar * clipNear / (clipFar - clipNear);

    return Matrix4x4
    {
        x,     0,     0,     0,
        0,     y,     0,     0,
        0,     0,    zz,    zw,
        0,     0, -1.0f,     0
    };
  }

  static Matrix4x4 ScreenSpace(u32 width, u32 height)
  {
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    return Matrix4x4
    {
         hw,    0,    0,  hw,// + 0.5f,
          0,   hh,    0,  hh,// + 0.5f,
          0,    0, 1.0f,          0,
          0,    0,    0,       1.0f
    };
  }
};
