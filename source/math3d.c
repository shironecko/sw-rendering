#pragma once

#include <math.h>

//******************** Config ********************//

#ifndef MATH_ASSERT
#define MATH_ASSERT SDL_assert
#endif // #ifndef MATH_ASSERT

#ifndef MATH_INLINE
#define MATH_INLINE inline
#endif // #ifndef MATH_INLINE

#define MATH_FN LOCAL
#define MATH_INLINE_FN MATH_INLINE MATH_FN

//******************** Data ********************//

#define PI 3.14159265359f
#define EPSILON 0.0000001f

typedef struct vec2 {
	union {
		float e[2];

		struct {
			float x, y;
		};

		struct {
			float u, v;
		};
	};
} vec2;

typedef struct vec3 {
	union {
		float e[3];

		struct {
			float x, y, z;
		};

		struct {
			float r, g, b;
		};

		struct {
			union {
				vec2 xy, uv;
			};
			float ignored_0;
		};
	};
} vec3;

typedef struct vec4 {
	union {
		float e[4];

		struct {
			float x, y, z, w;
		};

		struct {
			float r, g, b, a;
		};

		struct {
			union {
				vec2 xy, uv;
			};
			vec2 ignored_0;
		};

		struct {
			union {
				vec3 xyz, rgb;
			};
			float ignored_1;
		};
	};
} vec4;

typedef struct mat4 {
	union {
		float e[16];
		vec4 rows[4];
	};
} mat4;

#define V2_ZERO ((vec2){0, 0})
#define V2_ONE ((vec2){1.0f, 1.0f})
#define V2_UP ((vec2){0, 1.0f})
#define V2_RIGHT ((vec2){1.0f, 0})

#define V3_ZERO ((vec3){0, 0, 0})
#define V3_ONE ((vec3){1.0f, 1.0f, 1.0f})
#define V3_UP ((vec3){0, 1.0f, 0})
#define V3_RIGHT ((vec3){1.0f, 0, 0})
#define V3_FORWARD ((vec3){0, 0, -1.0f})

#define V4_ZERO ((vec4){0, 0, 0, 0})
#define V4_ONE ((vec4){1.0f, 1.0f, 1.0f, 1.0f})

//******************** Functions ********************//

MATH_INLINE_FN float clamp(float val, float min, float max) {
	if (val > max)
		return max;
	if (val < min)
		return min;

	return val;
}

MATH_INLINE_FN float clamp01(float val) {
	return clamp(val, 0, 1.0f);
}

/* MATH_INLINE_FN float pow(float val, u32 power) { */
/* 	if (!power) */
/* 		return 0; */

/* 	float result = 1.0f; */
/* 	for (u32 i = 0; i < power; ++i) */
/* 		result *= val; */

/* 	return result; */
/* } */

// vec2
MATH_INLINE_FN vec2 add_v2(vec2 a, vec2 b) {
	return (vec2){a.x + b.x, a.y + b.y};
}
MATH_INLINE_FN vec2 sub_v2(vec2 a, vec2 b) {
	return (vec2){a.x - b.x, a.y - b.y};
}
MATH_INLINE_FN vec2 neg_v2(vec2 v) {
	return (vec2){-v.x, -v.y};
}
MATH_INLINE_FN vec2 mul_v2f(vec2 v, float s) {
	return (vec2){v.x * s, v.y * s};
}
MATH_INLINE_FN vec2 div_v2f(vec2 v, float s) {
	MATH_ASSERT(s != 0);
	return (vec2){v.x / s, v.y / s};
}
MATH_INLINE_FN float sqrlen_v2(vec2 v) {
	return v.x * v.x + v.y * v.y;
}
MATH_INLINE_FN float len_v2(vec2 v) {
	return (float)sqrt(sqrlen_v2(v));
}
MATH_INLINE_FN vec2 norm_v2(vec2 v) {
	return div_v2f(v, len_v2(v));
}
MATH_INLINE_FN float dot_v2(vec2 a, vec2 b) {
	return a.x * b.x + a.y * b.y;
}

// vec3
MATH_INLINE_FN vec3 add_v3(vec3 a, vec3 b) {
	return (vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}
MATH_INLINE_FN vec3 sub_v3(vec3 a, vec3 b) {
	return (vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}
MATH_INLINE_FN vec3 neg_v3(vec3 v) {
	return (vec3){-v.x, -v.y, -v.z};
}
MATH_INLINE_FN vec3 mul_v3f(vec3 v, float s) {
	return (vec3){v.x * s, v.y * s, v.z * s};
}
MATH_INLINE_FN vec3 div_v3f(vec3 v, float s) {
	MATH_ASSERT(s != 0);
	return (vec3){v.x / s, v.y / s, v.z / s};
}
MATH_INLINE_FN float sqrlen_v3(vec3 v) {
	return v.x * v.x + v.y * v.y + v.z * v.z;
}
MATH_INLINE_FN float len_v3(vec3 v) {
	return (float)sqrt(sqrlen_v3(v));
}
MATH_INLINE_FN vec3 norm_v3(vec3 v) {
	return div_v3f(v, len_v3(v));
}
MATH_INLINE_FN float dot_v3(vec3 a, vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
MATH_INLINE_FN vec3 cross(vec3 a, vec3 b) {
	return (vec3){a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// vec4
MATH_INLINE_FN vec4 add_v4(vec4 a, vec4 b) {
	return (vec4){a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w};
}
MATH_INLINE_FN vec4 sub_v4(vec4 a, vec4 b) {
	return (vec4){a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w};
}
MATH_INLINE_FN vec4 neg_v4(vec4 v) {
	return (vec4){-v.x, -v.y, -v.z, -v.w};
}
MATH_INLINE_FN vec4 mul_v4f(vec4 v, float s) {
	return (vec4){v.x * s, v.y * s, v.z * s, v.w * s};
}
MATH_INLINE_FN vec4 div_v4f(vec4 v, float s) {
	MATH_ASSERT(s != 0);
	return (vec4){v.x / s, v.y / s, v.z / s, v.w / s};
}
MATH_INLINE_FN float dot_v4(vec4 a, vec4 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

MATH_INLINE_FN vec4 v3_to_v4(vec3 v, float w)
{
	return (vec4){v.x, v.y, v.z, w};
}

// mat4
MATH_INLINE_FN vec4 row_m4(mat4 m, u32 row) {
    MATH_ASSERT(row < 4);
	return m.rows[row];
}

MATH_INLINE_FN vec4 col_m4(mat4 m, u32 col) {
    MATH_ASSERT(col < 4);
    vec4 m0 = row_m4(m, 0);
    vec4 m1 = row_m4(m, 1);
    vec4 m2 = row_m4(m, 2);
    vec4 m3 = row_m4(m, 3);
	return (vec4){
        m0.e[col], 
        m1.e[col], 
        m2.e[col],
	    m3.e[col]
    };
}

MATH_FN vec4 mul_m4v4(mat4 m, vec4 v) {
	return (vec4){
        dot_v4(v, row_m4(m, 0)), 
        dot_v4(v, row_m4(m, 1)), 
        dot_v4(v, row_m4(m, 2)),
	    dot_v4(v, row_m4(m, 3))
    };
}

MATH_FN mat4 mul_m4(mat4 a, mat4 b) {
	mat4 result;
	for (u32 y = 0; y < 4; ++y) {
		vec4 row = row_m4(a, y);
		for (u32 x = 0; x < 4; ++x) {
			vec4 col = col_m4(b, x);
			result.e[y * 4 + x] = dot_v4(row, col);
		}
	}

	return result;
}

MATH_INLINE_FN mat4 unit_m4() {
	return (mat4){1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f};
}

MATH_INLINE_FN mat4 scale_m4(float sx, float sy, float sz) {
	return (mat4){sx, 0, 0, 0, 0, sy, 0, 0, 0, 0, sz, 0, 0, 0, 0, 1};
};

MATH_INLINE_FN mat4 trans_m4(float x, float y, float z) {
	return (mat4){1, 0, 0, x, 0, 1, 0, y, 0, 0, 1, z, 0, 0, 0, 1};
};

MATH_INLINE_FN mat4 rotx_m4(float angle) {
	return (mat4){1.0f,
	              0,
	              0,
	              0,
	              0,
	              (float)cos(angle),
	              (float)-sin(angle),
	              0,
	              0,
	              (float)sin(angle),
	              (float)cos(angle),
	              0,
	              0,
	              0,
	              0,
	              1.0f};
}

MATH_INLINE_FN mat4 roty_m4(float angle) {
	return (mat4){(float)cos(angle),  0, (float)sin(angle), 0, 0, 1.0f, 0, 0,
	              (float)-sin(angle), 0, (float)cos(angle), 0, 0, 0,    0, 1.0f};
}

MATH_FN mat4 lookat_cam(vec3 eye, vec3 target, vec3 up) {
	// note: do I really need this assert?
	/* MATH_ASSERT(abs(len_v3(up) - 1.0f) < 0.0001f); */

	vec3 zaxis = norm_v3(sub_v3(eye, target));
	vec3 xaxis = norm_v3(cross(up, zaxis));
	vec3 yaxis = cross(zaxis, xaxis);

	return (mat4){xaxis.x, xaxis.y, xaxis.z, -dot_v3(eye, xaxis),
	              yaxis.x, yaxis.y, yaxis.z, -dot_v3(eye, yaxis),
	              zaxis.x, zaxis.y, zaxis.z, -dot_v3(eye, zaxis),
	              0,       0,       0,       1.0f};
}

// lifted from http://www.3dgep.com/understanding-the-view-matrix/
MATH_FN mat4 fps_cam(vec3 eye, float yaw, float pitch) {
	float cos_pitch = (float)cos(pitch);
	float sin_pitch = (float)sin(pitch);
	float cos_yaw = (float)cos(yaw);
	float sin_yaw = (float)sin(yaw);

	vec3 xaxis = {cos_yaw, 0, -sin_yaw};
	vec3 yaxis = {sin_yaw * sin_pitch, cos_pitch, cos_yaw * sin_pitch};
	vec3 zaxis = {sin_yaw * cos_pitch, -sin_pitch, cos_yaw * cos_pitch};

	return (mat4){xaxis.x,
	              yaxis.x,
	              zaxis.x,
	              0,
	              xaxis.y,
	              yaxis.y,
	              zaxis.y,
	              0,
	              xaxis.z,
	              yaxis.z,
	              zaxis.z,
	              0,
	              -dot_v3(xaxis, eye),
	              -dot_v3(yaxis, eye),
	              -dot_v3(zaxis, eye),
	              1};
}

MATH_FN mat4 projection(float vfov, float aspect, float clip_near, float clip_far) {
	// field of view
	float y = 1.0f / (float)tan(vfov * PI / 360.0f);
	float x = y / aspect;

	// map z to [0, 1]
	float zz = -clip_far / (clip_far - clip_near);
	float zw = -clip_far * clip_near / (clip_far - clip_near);

	return (mat4){x, 0, 0, 0, 0, y, 0, 0, 0, 0, zz, zw, 0, 0, -1.0f, 0};
}

MATH_FN mat4 screen_space(u32 width, u32 height) {
	float hw = width * 0.5f;
	float hh = height * 0.5f;

	return (mat4){hw, 0, 0, hw, 0, hh, 0, hh, 0, 0, 1.0f, 0, 0, 0, 0, 1.0f};
}
