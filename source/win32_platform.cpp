#define _HAS_EXCEPTIONS 0
#define _STATIC_CPPLIB

// TODO: sort this out
//#include <assert.h>
//#include <stdio.h>

#define assert(x) __nop()

#include "types.cpp"

u32 PlatformLoadFile(char* path, void* memory, u32 memorySize);
bool PlatformWriteFile(char* path, void* memory, u32 bytesToWrite);

#ifdef GAME_PROJECT
#include "game.cpp"
#elif defined(RESOURCE_CONVERTER_PROJECT)
#include "resource_converter.cpp"
#else
#error "You did not specified project type!"
#endif

#include <windows.h>

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
  bool            shouldRun           = true;
  u32             windowWidth         = 640;
  u32             windowHeight        = 480;

  void*           memory              = nullptr;
  u32             memorySize          = 0;
  RenderTarget    renderBuffer        {};
  Win32BackBuffer backBuffer          {};

} g_platformData;

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

  Bitmap* bitmap = &g_platformData.renderBuffer.bitmap;
  bitmap->width = width;
  bitmap->height = height;
  bitmap->memory = (Color*)platformMemory;
  platformMemory += width * height * sizeof(Color);

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
  u32 bufferWidth = renderBuffer->bitmap.width;
  u32 bufferHeight = renderBuffer->bitmap.height;

  // TODO: think about just allocating back-buffer here, on the stack
  for (u32 y = 0; y < bufferHeight; ++y)
  {
    for (u32 x = 0; x < bufferWidth; ++x)
    {
      Color bufferColor = renderBuffer->bitmap.GetPixel(x, y);

      backBuffer->memory[y * bufferWidth + x] = 
           u8(bufferColor.b * 255.0f) |
          (u8(bufferColor.g * 255.0f) << 8) |
          (u8(bufferColor.r * 255.0f) << 16);
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
  Input input {};

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
    /* char windowTitle[256]; */
    /* _snprintf_s(windowTitle, 256, 255, "Software Renderer \t %.2fms per frame", deltaTime * 1000.0f); */
    /* SetWindowText(window, windowTitle); */

    while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&message);

      if (message.message == WM_KEYDOWN)
        input.keyboard[message.wParam] = true;
      else if (message.message == WM_KEYUP)
        input.keyboard[message.wParam] = false;

      DispatchMessage(&message);
    }

    GameUpdate(
        deltaTime,
        gameMemory,
        gameMemorySize,
        &g_platformData.renderBuffer,
        &input);

    Win32PresentToWindow(
      windowDC,
      g_platformData.windowWidth,
      g_platformData.windowHeight,
      &g_platformData.backBuffer,
      &g_platformData.renderBuffer);
  }
  
  return 0;
}
