#pragma once

#define global static
#define local_persist static
#define local static

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

#define TYPE_SIZE_ERROR_MSG "Unsupported type size for current platform, please fix this!"
static_assert(sizeof(s16) == 2, TYPE_SIZE_ERROR_MSG);
static_assert(sizeof(s32) == 4, TYPE_SIZE_ERROR_MSG);
static_assert(sizeof(s64) == 8, TYPE_SIZE_ERROR_MSG);
#undef TYPE_SIZE_ERROR_MSG 

/*
 * TODO: is this a good way to figure out bitness? or do I need to check
 * against platform-dependant defines?
 */
#if defined(_WIN32)
#define PAPI_X32
#elif defined(_WIN64)
#define PAPI_X64
#else
#error "Unsupported platform!"
#endif

#if defined(PAPI_X32)
typedef u32 uptr;
typedef u32 usize;
#elif defined(PAPI_X64)
typedef u64 uptr;
typedef u64 usize;
#endif

template <typename T>
T align(T x, u32 alignment = 16) {
	return (T)(((uptr)x + alignment - 1) & (~(alignment - 1)));
}

#if defined(_MSC_VER)
#define PLATFORM_INTRIN_HEADER "intrin.h"
#elif defined(__GNUC__) || defined(__clang__)
#define PLATFORM_INTRIN_HEADER "x86intrin.h"
#else
#error "Unsupported compiler!"
#endif

global const u32 Kb = 1024;
global const u32 Mb = 1024 * Kb;
global const u32 Gb = 1024 * Mb;

void PlatformAssert(u32 condition);
u64 PlatformGetFileSize(const char *path);
u32 PlatformLoadFile(const char *path, void *memory, u32 memorySize);
bool PlatformWriteFile(const char *path, void *memory, u32 bytesToWrite);

#define assert(x) PlatformAssert((u32)(x))

struct KbKey {
	enum {
		Unknown,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		N_1,
		N_2,
		N_3,
		N_4,
		N_5,
		N_6,
		N_7,
		N_8,
		N_9,
		N_0,

		ShiftL,
		ShiftR,
		ControlL,
		ControlR,
		AltL,
		AltR,

		Left,
		Right,
		Up,
		Down,

		PageDown,
		PageUp,

		Return,
		Escape,
		Delete,
		Backspace,
		Tab,
		Space,
		End,
		Home,

		Colon,
		Semicolon,
		Apostrophe,
		Grave,
		Comma,
		Period,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		LastKey
	};
};
