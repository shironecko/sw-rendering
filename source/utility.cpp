#pragma once

#include "platform_api.h"

template<typename T>
T abs(T x)
{
  return x >= 0 ? x : -x;
}

template<typename T>
T min(T a, T b)
{
  return a > b ? b : a;
}

template<typename T>
T max(T a, T b)
{
  return a > b ? a : b;
}

template<typename T>
void swap(T* a, T* b)
{
  T tmp = *a;
  *a = *b;
  *b = tmp;
}

struct MemPool
{
    u8 *start, *end;
    u8 *lowPtr, *hiPtr;
};

MemPool NewMemPool(void *memory, u64 size)
{
    MemPool result;
    result.start = result.lowPtr = (u8*)memory;
    result.end = result.hiPtr = (u8*)memory + size;

    return result;
}

void* MemPush(MemPool *pool, u64 size)
{
    u8 *new_low_ptr = pool->lowPtr + size;
    assert(new_low_ptr <= pool->hiPtr);
    void *result = pool->lowPtr;
    pool->lowPtr = new_low_ptr;

    return result;
}

void* MemPushBack(MemPool *pool, u64 size)
{
    u8 *new_hi_ptr = pool->hiPtr - size;
    assert(new_hi_ptr >= pool->lowPtr);
    pool->hiPtr = new_hi_ptr;

    return new_hi_ptr;
}

