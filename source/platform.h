#pragma once

#include <SDL.h>

#define GLOBAL static
#define LOCAL_PERSISTENT static
#define LOCAL static

typedef Sint8 s8;
typedef Sint16 s16;
typedef Sint32 s32;
typedef Sint64 s64;

typedef Uint8 u8;
typedef Uint16 u16;
typedef Uint32 u32;
typedef Uint64 u64;

typedef u32 b32;
typedef size_t usize;
typedef uintptr_t uptr;

#define true 1
#define false 0

#define Kb (1024)
#define Mb (1024 * Kb)
#define Gb (1024ULL * Mb)
#define Tb (1024ULL * Gb)

#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__) || defined(__NACL__)
#define HAVE_OPENGLES
#endif

#if defined(HAVE_OPENGLES)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

struct gl_functions {
#define gl_function(ret, func, params) ret(APIENTRY *func) params;
#include "gl_functions.h"
#undef gl_function
};
