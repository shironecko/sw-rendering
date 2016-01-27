#pragma once

#define global        static
#define local_persist static
#define local         static

typedef signed    char       s8;
typedef signed    short      s16;
typedef signed    int        s32;
typedef signed    long long  s64;

typedef unsigned  char       u8;
typedef unsigned  short      u16;
typedef unsigned  int        u32;
typedef unsigned  long long  u64;

#define TYPE_SIZE_ERROR_MSG "Unsupported type size for current platform, please fix this!"

static_assert(sizeof(s16) == 2, TYPE_SIZE_ERROR_MSG);
static_assert(sizeof(s32) == 4, TYPE_SIZE_ERROR_MSG);
static_assert(sizeof(s64) == 8, TYPE_SIZE_ERROR_MSG);

#define assert(x)

global const u32 Kb = 1024;
global const u32 Mb = 1024 * Kb;
global const u32 Gb = 1024 * Mb;

u64 PlatformGetFileSize(const char* path);
u32 PlatformLoadFile(const char* path, void* memory, u32 memorySize);
bool PlatformWriteFile(const char* path, void* memory, u32 bytesToWrite);

struct KbKey
{
  enum
  {
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
