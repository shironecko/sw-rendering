#include <cassert>

class Stack
{
public:
  void Initialize(void* memory, u32 size)
  {
    m_memory = (u8*)memory;
    m_size = size;
    m_top = 0;
  }

  u8* Push(u32 bytes)
  {
    assert(m_top + bytes < m_size);
    u8* result = m_memory + m_top;
    m_top += bytes;

    return result;
  }

  u32 GetPosition()
  {
    return m_top;
  }

  void SetPosition(u32 position)
  {
    assert(position < m_size);
    m_top = position;
  }

private:
  u8* m_memory;
  u32 m_size;
  u32 m_top;
}
