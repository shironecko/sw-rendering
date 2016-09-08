#pragma once

#include "platform_api.h"

template <typename T>
T abs(T x) {
	return x >= 0 ? x : -x;
}

template <typename T>
T min(T a, T b) {
	return a > b ? b : a;
}

template <typename T>
T max(T a, T b) {
	return a > b ? a : b;
}

template <typename T>
void swap(T *a, T *b) {
	T tmp = *a;
	*a = *b;
	*b = tmp;
}

struct MemPool {
	u8 *start, *end;
	u8 *lowPtr, *hiPtr;
};

MemPool NewMemPool(void *memory, u64 size) {
	MemPool result;
	result.start = result.lowPtr = (u8 *)memory;
	result.end = result.hiPtr = (u8 *)memory + size;

	return result;
}

void *MemPush(MemPool *pool, u64 size) {
	u8 *new_low_ptr = pool->lowPtr + size;
	assert(new_low_ptr <= pool->hiPtr);
	void *result = pool->lowPtr;
	pool->lowPtr = new_low_ptr;

	return result;
}

void *MemPushBack(MemPool *pool, u64 size) {
	u8 *new_hi_ptr = pool->hiPtr - size;
	assert(new_hi_ptr >= pool->lowPtr);
	pool->hiPtr = new_hi_ptr;

	return new_hi_ptr;
}

void MemoryCopy(void *destination, void *source, u32 bytesToCopy) {
	u8 *destinationBytes = (u8 *)destination;
	u8 *sourceBytes = (u8 *)source;

	for (u32 i = 0; i < bytesToCopy; ++i) { destinationBytes[i] = sourceBytes[i]; }
}

void MemorySet(void *memory, u8 value, u32 size) {
	// NOTE: super-slow, but who cares
	u8 *mem = (u8 *)memory;
	for (u32 i = 0; i < size; ++i) *mem = value;
}

bool MemoryEqual(void *memoryA, void *memoryB, u32 bytesToCompare) {
	u8 *memoryABytes = (u8 *)memoryA;
	u8 *memoryBBytes = (u8 *)memoryB;

	for (u32 i = 0; i < bytesToCompare; ++i) {
		if (memoryABytes[i] != memoryBBytes[i]) return false;
	}

	return true;
}
