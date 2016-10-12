#define GLOBAL static
#define LOCAL_PERSISTENT static
#define LOCAL static

#ifndef INTMAX_MAX
#include <stdint.h>
#endif // #ifndef INTMAX_MAX

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef u32 b32;
typedef size_t usize;
typedef uintptr_t uptr;

#define true 1
#define false 0

#define Kb (1024)
#define Mb (1024 * Kb)
#define Gb (1024ULL * Mb)
#define Tb (1024ULL * Gb)
