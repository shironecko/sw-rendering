#include <cmath>

struct Vector4
{
  union
  {
    float components[4];
    
    struct
    {
      float x;
      float y;
      float z;
      float w;
    };
  };
};
