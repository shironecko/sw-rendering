#include <stdio.h>
#include <assert.h>

#include <platform.h>

#define UT_ASSERT assert
#include <utility.c>

#define MATH_ASSERT assert
#include <math3d.c>

#define SWR_ASSERT assert
#include <sw_render.c>

int main(void)
{
	printf("hi!\n");
	return 0;
}
