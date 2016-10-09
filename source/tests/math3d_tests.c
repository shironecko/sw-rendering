#include <stdio.h>
#include <assert.h>

#define GLOBAL static
#define LOCAL_PERSISTENT static
#define LOCAL static

typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed long long s64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef u32 b32;
typedef size_t usize;
typedef uintptr_t uptr;

#define true 1
#define false 0

#define MATH_ASSERT assert
#include <math3d.c>

int main(void)
{
	printf("hi!\n");
	return 0;
}
