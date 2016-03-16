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
