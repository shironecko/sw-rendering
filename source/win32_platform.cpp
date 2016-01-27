#define _HAS_EXCEPTIONS 0
#define _STATIC_CPPLIB

#include "platform_api.h"

#ifdef GAME_PROJECT
#include "game.cpp"
#elif defined(RESOURCE_CONVERTER_PROJECT)
#include "resource_converter.cpp"
#else
#error "You did not specified project type!"
#endif

#include <windows.h>

u64 PlatformGetFileSize(char* path)
{
  HANDLE fileHandle = CreateFile(
    path,
    FILE_READ_ATTRIBUTES,
    FILE_SHARE_READ,
    nullptr,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    0
  );

  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    OutputDebugStringA("Was not able to open a file!\n");
    return 0;
  }

  LARGE_INTEGER size;
  BOOL getFileSizeResult = GetFileSizeEx(
    fileHandle,
    &size
  );

  if (!getFileSizeResult)
  {
    OutputDebugStringA("Was not able to get a file size!\n");
    return 0;
  }

  CloseHandle(fileHandle);
  return u64(size.QuadPart);
}

u32 PlatformLoadFile(char* path, void* memory, u32 memorySize)
{
  HANDLE fileHandle = CreateFile(
    path,
    GENERIC_READ,
    FILE_SHARE_READ,
    nullptr,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL,
    0
  );

  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    OutputDebugStringA("Was not able to open a file!\n");
    return 0;
  }

  DWORD bytesRead;
  BOOL readResult = ReadFile(
    fileHandle,
    memory,
    memorySize,
    &bytesRead,
    nullptr
  );

  if (!readResult)
  {
    OutputDebugStringA("Was not able to read from a file!\n");
    return 0;
  }

  CloseHandle(fileHandle);
  return bytesRead;
}

bool PlatformWriteFile(char* path, void* memory, u32 bytesToWrite)
{
  // TODO: handle directory creation

  HANDLE fileHandle = CreateFile(
    path,
    GENERIC_WRITE,
    0,
    nullptr,
    CREATE_ALWAYS,
    FILE_ATTRIBUTE_NORMAL,
    0
  );

  if (fileHandle == INVALID_HANDLE_VALUE)
  {
    OutputDebugStringA("Was not able to create or owerwrite a file!\n");
    return false;
  }

  DWORD bytesWritten;
  BOOL writeResult = WriteFile(
    fileHandle,
    memory,
    bytesToWrite,
    &bytesWritten,
    nullptr
  );

  if (!writeResult)
  {
    OutputDebugStringA("Was not able to write to a file!\n");
    CloseHandle(fileHandle);
    return false;
  }

  if (bytesToWrite != bytesWritten)
  {
    OutputDebugStringA("Bytes to write was not equal to bytes written!\n");
    CloseHandle(fileHandle);
    return false;
  }

  CloseHandle(fileHandle);
  return true;
}

struct Win32BackBuffer
{
  BITMAPINFO info;
  u32* memory;
};

struct
{
  bool            shouldRun;
  u32             windowWidth;
  u32             windowHeight;

  void*           memory;
  u32             memorySize;
  RenderTarget    renderBuffer;
  Win32BackBuffer backBuffer;

} g_platformData
{
  true,
  640,
  480,
  nullptr,
  0,
  {},
  {}
};

void Win32SetupRenderingBuffers(u32 width, u32 height)
{
  u8* platformMemory = (u8*)g_platformData.memory;

  BITMAPINFO info {};
  info.bmiHeader.biSize = sizeof(info.bmiHeader);
  info.bmiHeader.biWidth = width;
  info.bmiHeader.biHeight = height;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;

  g_platformData.backBuffer.info = info;
  g_platformData.backBuffer.memory = (u32*)platformMemory;
  platformMemory += width * height * sizeof(u32);

  Texture** texture = &g_platformData.renderBuffer.texture;
  *texture = (Texture*)platformMemory;
  (*texture)->width = width;
  (*texture)->height = height;
  platformMemory += width * height * sizeof(Color32) + sizeof(Texture);

  g_platformData.renderBuffer.zBuffer = (float*)platformMemory;
  platformMemory += width * height * sizeof(float);

  assert(u32(platformMemory - (u8*)g_platformData.memory) <= g_platformData.memorySize);
}

void Win32PresentToWindow(
    HDC windowDC,
    u32 windowWidth,
    u32 windowHeight,
    Win32BackBuffer* backBuffer,
    RenderTarget* renderBuffer)
{
  u32 bufferWidth = renderBuffer->texture->width;
  u32 bufferHeight = renderBuffer->texture->height;

  // TODO: think about just allocating back-buffer here, on the stack
  for (u32 y = 0; y < bufferHeight; ++y)
  {
    for (u32 x = 0; x < bufferWidth; ++x)
    {
      Color32 bufferColor = renderBuffer->texture->GetTexel(x, y);

      backBuffer->memory[y * bufferWidth + x] = 
          u32(bufferColor.b) |
          u32(bufferColor.g) << 8 |
          u32(bufferColor.r) << 16;
    }
  }

  StretchDIBits(
    windowDC,
    0,
    0,
    windowWidth,
    windowHeight,
    0,
    0,
    backBuffer->info.bmiHeader.biWidth,
    backBuffer->info.bmiHeader.biHeight,
    backBuffer->memory,
    &backBuffer->info,
    DIB_RGB_COLORS,
    SRCCOPY);
}

global const u32 g_keyMapSize = 0xA5 + 1;

global const u32 g_keyMap[0xA5 + 1] =
{
  [VK_BACK]       = KbKey::BACKSPACE,
  [VK_TAB]        = KbKey::TAB,
  [VK_RETURN]     = KbKey::RETURN,
  [VK_ESCAPE]     = KbKey::ESCAPE,
  [VK_SPACE]      = KbKey::SPACE,
  [VK_END]        = KbKey::END,
  [VK_HOME]       = KbKey::HOME,
  [VK_LEFT]       = KbKey::LEFT,
  [VK_UP]         = KbKey::UP,
  [VK_RIGHT]      = KbKey::RIGHT,
  [VK_DOWN]       = KbKey::DOWN,
  [VK_DELETE]     = KbKey::DELETE,

  [0x30]          = KbKey::N_0,
  [0x31]          = KbKey::N_1,
  [0x32]          = KbKey::N_2,
  [0x33]          = KbKey::N_3,
  [0x34]          = KbKey::N_4,
  [0x35]          = KbKey::N_5,
  [0x36]          = KbKey::N_6,
  [0x37]          = KbKey::N_7,
  [0x38]          = KbKey::N_8,
  [0x39]          = KbKey::N_9,

  [0x41]          = KbKey::A,
  [0x42]          = KbKey::B,
  [0x43]          = KbKey::C,
  [0x44]          = KbKey::D,
  [0x45]          = KbKey::E,
  [0x46]          = KbKey::F,
  [0x47]          = KbKey::G,
  [0x48]          = KbKey::H,
  [0x49]          = KbKey::I,
  [0x4A]          = KbKey::J,
  [0x4B]          = KbKey::K,
  [0x4C]          = KbKey::L,
  [0x4D]          = KbKey::M,
  [0x4E]          = KbKey::N,
  [0x4F]          = KbKey::O,
  [0x50]          = KbKey::P,
  [0x51]          = KbKey::Q,
  [0x52]          = KbKey::R,
  [0x53]          = KbKey::S,
  [0x54]          = KbKey::T,
  [0x55]          = KbKey::U,
  [0x56]          = KbKey::V,
  [0x57]          = KbKey::W,
  [0x58]          = KbKey::X,
  [0x59]          = KbKey::Y,
  [0x5A]          = KbKey::Z,

  [VK_F1]         = KbKey::F1,
  [VK_F2]         = KbKey::F2,
  [VK_F3]         = KbKey::F3,
  [VK_F4]         = KbKey::F4,
  [VK_F5]         = KbKey::F5,
  [VK_F6]         = KbKey::F6,
  [VK_F7]         = KbKey::F7,
  [VK_F8]         = KbKey::F8,
  [VK_F9]         = KbKey::F9,
  [VK_F10]        = KbKey::F10,
  [VK_F11]        = KbKey::F11,
  [VK_F12]        = KbKey::F12,

  [VK_LSHIFT]     = KbKey::SHIFT_L,
  [VK_RSHIFT]     = KbKey::SHIFT_R,
  [VK_LCONTROL]   = KbKey::CONTROL_L,
  [VK_RCONTROL]   = KbKey::CONTROL_R,
  [VK_LMENU]      = KbKey::ALT_L,
  [VK_RMENU]      = KbKey::ALT_R, // 0xA5 Right MENU key
};

LRESULT CALLBACK Win32WindowProc(
  HWND   window,
  UINT   message,
  WPARAM wParam,
  LPARAM lParam)
{
  switch (message)
  {
    case WM_SIZE:
    {
      g_platformData.windowWidth = LOWORD(lParam);
      g_platformData.windowHeight = HIWORD(lParam);
    } break;
    case WM_EXITSIZEMOVE:
    {
      // this is basically free with manual memory management
      Win32SetupRenderingBuffers(g_platformData.windowWidth, g_platformData.windowHeight);
    } break;
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC windowDC = BeginPaint(window, &ps);

      Win32PresentToWindow(
        windowDC,
        g_platformData.windowWidth,
        g_platformData.windowHeight,
        &g_platformData.backBuffer,
        &g_platformData.renderBuffer);

      EndPaint(window, &ps);
    } break;
    case WM_CLOSE:
    case WM_DESTROY:
    {
      g_platformData.shouldRun = false;
    } break;
    default:
    {
      return DefWindowProc(window, message, wParam, lParam);
    } break;
  }

  return 0;
}

int CALLBACK WinMain(
  HINSTANCE instance,
  HINSTANCE /* prevInstance */,
  char*     /* cmdLine */,
  int       /* cmdShow */)
{
  //*****CREATING A WINDOW*****//
  WNDCLASSEX wndClass {};
  wndClass.cbSize = sizeof(wndClass);
  wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wndClass.lpfnWndProc = Win32WindowProc;
  wndClass.hInstance = instance;
  wndClass.lpszClassName = "Software Renderer Window Class Name";

  RegisterClassEx(&wndClass);

  HWND window = CreateWindowEx(
    0,
    wndClass.lpszClassName,
    "Software Renderer",
    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
    CW_USEDEFAULT,
    CW_USEDEFAULT,
    g_platformData.windowWidth,
    g_platformData.windowHeight,
    0,
    0,
    instance,
    nullptr
  );

  //*****ALLOCATING MEMORY*****//
  g_platformData.memorySize = 128 * Mb;
  g_platformData.memory = VirtualAlloc(
    nullptr,
    g_platformData.memorySize,
    MEM_RESERVE | MEM_COMMIT,
    PAGE_READWRITE
  );
  assert(g_platformData.memory);

  u32   gameMemorySize = 512 * Mb;
  void* gameMemory = VirtualAlloc(
    nullptr,
    gameMemorySize,
    MEM_RESERVE | MEM_COMMIT,
    PAGE_READWRITE
  );
  assert(gameMemory);

  //*****MISC SETUP*****//
  Win32SetupRenderingBuffers(g_platformData.windowWidth, g_platformData.windowHeight);

  HDC windowDC = GetDC(window);
  MSG message {};
  bool kbState[KbKey::LAST_KEY] {};

  LARGE_INTEGER lastFrameTime;
  LARGE_INTEGER queryFrequency;
  QueryPerformanceCounter(&lastFrameTime);
  QueryPerformanceFrequency(&queryFrequency);

  GameInitialize(gameMemory, gameMemorySize);

  //*****RENDERING LOOP*****//
  while (g_platformData.shouldRun)
  {
    LARGE_INTEGER currentFrameTime;
    QueryPerformanceCounter(&currentFrameTime);
    u64 ticksElapsed = currentFrameTime.QuadPart - lastFrameTime.QuadPart;
    float deltaTime = float(ticksElapsed) / float(queryFrequency.QuadPart);
    lastFrameTime = currentFrameTime;

    // TODO: sort this out
    char windowTitle[256];
    wsprintf(windowTitle, "Software Renderer \t %ums per frame", u32(deltaTime * 1000.0f));
    SetWindowText(window, windowTitle);

    while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&message);

      if (message.message == WM_KEYDOWN || message.message == WM_KEYUP)
      {
        u32 keyMapIdx = message.wParam;
        if (keyMapIdx < g_keyMapSize)
        {
          u32 kbStateIdx = g_keyMap[keyMapIdx];
          assert(kbStateIdx < KbKey::LAST_KEY);

          kbState[kbStateIdx] = message.message == WM_KEYDOWN;
        }
      }

      DispatchMessage(&message);
    }

    bool gameWantsToContinue = GameUpdate(
        deltaTime,
        gameMemory,
        gameMemorySize,
        &g_platformData.renderBuffer,
        kbState);

    g_platformData.shouldRun &= gameWantsToContinue;

    Win32PresentToWindow(
      windowDC,
      g_platformData.windowWidth,
      g_platformData.windowHeight,
      &g_platformData.backBuffer,
      &g_platformData.renderBuffer);
  }
  
  return 0;
}
