#pragma once

#include "platform_api.h"
#include "utility.h"
#include "math.h"
#include PLATFORM_INTRIN_HEADER

//******************** General stuff ********************//

const float PI = float(3.14159265359);

/* float sqrt(float x) */
/* { */
/*   __m128 reg_x = _mm_load_ss(&x); */
/*   reg_x = _mm_sqrt_ss(reg_x); */
/*   float result; */
/*   _mm_store_ss(&result, reg_x); */

/*   return result; */
/* } */

//******************** Vector2 ********************//

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

  float& operator()(u32 i)
  {
    assert(i < 2);
    return components[i];
  }
};

float Dot2(Vector2 a, Vector2 b)
{
  return a.x * b.x + a.y * b.y;
}

Vector2 operator - (Vector2 a, Vector2 b)
{
  return Vector2 { a.x - b.x, a.y - b.y };
}

//******************** Vector4 ********************//

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

  float& operator()(u32 i)
  {
    assert(i < 4);
    return components[i];
  }
};

Vector4 operator / (Vector4 vec, float scalar)
{
  assert(scalar != 0);
  return Vector4 { vec.x / scalar, vec.y / scalar, vec.z / scalar, vec.w / scalar };
}

Vector4 operator - (Vector4 a, Vector4 b)
{
  return Vector4
  {
    a.x - b.x,
    a.y - b.y,
    a.z - b.z,
    a.w - b.w
  };
}

Vector4 operator - (Vector4 vec)
{
  return Vector4 { -vec.x, -vec.y, -vec.z, -vec.w };
}

Vector4 operator + (Vector4 a, Vector4 b)
{
  return a - (-b);
}

float SqrLength3(Vector4 vec)
{
  assert(vec.w == 0);
  return vec.x * vec.x + vec.y * vec.y + vec.z * vec.z;
}

float Length3(Vector4 vec)
{
  return (float)sqrt(SqrLength3(vec));
}

Vector4 Normalized3(Vector4 vec)
{
  assert(vec.w == 0);
  return vec / Length3(vec);
}

float Dot3(Vector4 a, Vector4 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Dot4(Vector4 a, Vector4 b)
{
  return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

Vector4 Cross(Vector4 a, Vector4 b)
{
  return Vector4
  {
    a.y * b.z - a.z * b.y,
    a.z * b.x - a.x * b.z,
    a.x * b.y - a.y * b.x,
    0
  };
}

//******************** Matrix4x4 ********************//

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
};

Vector4 Row(Matrix4x4 mat, u32 row)
{
  return mat.rows[row];
}

Vector4 Column(Matrix4x4 mat, u32 column)
{
  return Vector4
  {
    Row(mat, 0).components[column],
    Row(mat, 1).components[column],
    Row(mat, 2).components[column],
    Row(mat, 3).components[column]
  };
}

Vector4 operator * (Matrix4x4 mat, Vector4 vec)
{
  return Vector4
  {
    Dot4(vec, Row(mat, 0)),
    Dot4(vec, Row(mat, 1)),
    Dot4(vec, Row(mat, 2)),
    Dot4(vec, Row(mat, 3))
  };
}

Matrix4x4 operator * (Matrix4x4 a, Matrix4x4 b)
{
  Matrix4x4 result;
  for (u32 y = 0; y < 4; ++y)
  {
    Vector4 row = Row(a, y);
    for (u32 x = 0; x < 4; ++x)
    {
      Vector4 col = Column(b, x);
      result(y, x) = Dot4(row, col);
    }
  }

  return result;
}

/* Matrix4x4 InverseMatrix() */
/* { */
/*   Matrix4x4 result = *this; */
/*   PIII_Inverse_4x4(&result(0, 0)); */

/*   return result; */
/* } */

static Matrix4x4 IdentityMatrix()
{
  return Matrix4x4
  {
    1.0f,    0,    0,    0,
       0, 1.0f,    0,    0,
       0,    0, 1.0f,    0,
       0,    0,    0, 1.0f
  };
}

static Matrix4x4 RotationMatrixY(float angle)
{
  // TODO: use intrinsics
  return Matrix4x4
  {
     (float)cos(angle),    0, (float)sin(angle),    0,
                     0, 1.0f,                 0,    0,
    (float)-sin(angle),    0, (float)cos(angle),    0,
                     0,    0,                 0, 1.0f
  };
}

static Matrix4x4 LookAtCameraMatrix(Vector4 eye, Vector4 target, Vector4 up)
{
  assert(abs(Length3(up) - 1.0f) < 0.0001f);

  Vector4 zaxis = Normalized3(eye - target);
  Vector4 xaxis = Normalized3(Cross(up, zaxis));
  Vector4 yaxis = Cross(zaxis, xaxis);

  return Matrix4x4
  {
    xaxis.x, xaxis.y, xaxis.z, -Dot3(eye, xaxis),
    yaxis.x, yaxis.y, yaxis.z, -Dot3(eye, yaxis),
    zaxis.x, zaxis.y, zaxis.z, -Dot3(eye, zaxis),
          0,       0,       0,              1.0f
  };
}

static Matrix4x4 ProjectionMatrix(float vfov, float aspect, float clipNear, float clipFar)
{
  // field of view
  float y = 1.0f / (float)tan(vfov * PI / 360.0f);
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

static Matrix4x4 ScreenSpaceMatrix(u32 width, u32 height)
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
