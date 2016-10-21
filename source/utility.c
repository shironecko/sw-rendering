#pragma once

//******************** Config ********************//

#ifndef UT_ASSERT
#define UT_ASSERT SDL_assert
#endif // #ifndef UT_ASSERT

#ifndef UT_INLINE
#define UT_INLINE inline
#endif // #ifndef UT_INLINE

#define UT_FN LOCAL
#define UT_INLINE_FN UT_INLINE UT_FN

//******************** Data ********************//

typedef struct {
	u8 *start, *end;
	u8 *low, *hi;
} mem_pool;

//******************** Functions ********************//

UT_INLINE_FN mem_pool new_mem_pool(void *memory, u64 size) {
	mem_pool result;
	result.start = result.low = (u8 *)memory;
	result.end = result.hi = (u8 *)memory + size;

	return result;
}

UT_INLINE_FN void *mem_push(mem_pool *pool, u64 size) {
	u8 *new_low = pool->low + size;
	UT_ASSERT(new_low <= pool->hi);
	void *result = pool->low;
	pool->low = new_low;

	return result;
}

UT_INLINE_FN void *mem_push_back(mem_pool *pool, u64 size) {
	u8 *new_hi = pool->hi - size;
	UT_ASSERT(new_hi >= pool->low);
	pool->hi = new_hi;

	return new_hi;
}

#define DEFINE_MAX(fn_name, type)                                                                  \
	UT_INLINE_FN type fn_name(type a, type b) {                                                    \
		return a > b ? a : b;                                                                      \
	}

#define DEFINE_MIN(fn_name, type)                                                                  \
	UT_INLINE_FN type fn_name(type a, type b) {                                                    \
		return a < b ? a : b;                                                                      \
	}

#define DEFINE_ABS(fn_name, type)                                                                  \
	UT_INLINE_FN type fn_name(type x) {                                                            \
		return x > 0 ? x : -x;                                                                     \
	}

DEFINE_MIN(minf, float);
DEFINE_MAX(maxf, float);
DEFINE_ABS(absf, float);

DEFINE_MIN(minu, u32);
DEFINE_MAX(maxu, u32);
