#include <windows.h>

#include "types.h"
#include "renderer.cpp"

global bool       g_shouldRun = true;
global u32        g_windowWidth;
global u32        g_windowHeight;
global BITMAPINFO g_backBufferInfo;
global u32*       g_backBufferMemory;
global Bitmap*    g_renderBuffer;

u8 CompressColorComponent(float component)
{
  return u8(component * 255.0f);
}

BITMAPINFO ResizeRenderingBuffers(u32 width, u32 height)
{
  delete[] g_backBufferMemory;
  g_backBufferMemory = new u32[width * height];

  delete g_renderBuffer;
  g_renderBuffer = new Bitmap(width, height);

  BITMAPINFO info {};
  info.bmiHeader.biSize = sizeof(info.bmiHeader);
  info.bmiHeader.biWidth = width;
  info.bmiHeader.biHeight = height;
  info.bmiHeader.biPlanes = 1;
  info.bmiHeader.biBitCount = 32;
  info.bmiHeader.biCompression = BI_RGB;

  return info;
}

LRESULT CALLBACK WindowProc(
  HWND   window,
  UINT   message,
  WPARAM wParam,
  LPARAM lParam)
{
  switch (message)
  {
    case WM_SIZE:
    {
      g_windowWidth = LOWORD(lParam);
      g_windowHeight = HIWORD(lParam);
    } break;
    case WM_EXITSIZEMOVE:
    {
      if (g_windowWidth != g_renderBuffer->Width() || g_windowHeight != g_renderBuffer->Height())
        g_backBufferInfo = ResizeRenderingBuffers(g_windowWidth, g_windowHeight);
        
    } break;
    case WM_PAINT:
    {
      PAINTSTRUCT ps;
      HDC windowDC = BeginPaint(window, &ps);

      StretchDIBits(
        windowDC,
        0,
        0,
        g_windowWidth,
        g_windowHeight,
        0,
        0,
        g_backBufferInfo.bmiHeader.biWidth,
        g_backBufferInfo.bmiHeader.biHeight,
        g_backBufferMemory,
        &g_backBufferInfo,
        DIB_RGB_COLORS,
        SRCCOPY);
      
      EndPaint(window, &ps);
    } break;
    case WM_CLOSE:
    case WM_DESTROY:
    {
      g_shouldRun = false;
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
  LPSTR     /* cmdLine */,
  int       /* cmdShow */)
{
  g_windowWidth = 640;
  g_windowHeight = 480;

  WNDCLASSEX wndClass {};
  wndClass.cbSize = sizeof(wndClass);
  wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
  wndClass.lpfnWndProc = WindowProc;
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
    g_windowWidth,
    g_windowHeight,
    0,
    0,
    instance,
    nullptr
  );

  g_backBufferInfo = ResizeRenderingBuffers(g_windowWidth, g_windowHeight);

  HDC windowDC = GetDC(window);
  MSG message {};
  while (g_shouldRun)
  {
    while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }

    Render(*g_renderBuffer);
    for (int y = 0; y < g_renderBuffer->Height(); ++y)
    {
      for (int x = 0; x < g_renderBuffer->Width(); ++x)
      {
        Color& bufferColor = (*g_renderBuffer)(x, y);

        g_backBufferMemory[y * g_renderBuffer->Width() + x] = 
            CompressColorComponent(bufferColor.b) |
            (CompressColorComponent(bufferColor.g) << 8) |
            (CompressColorComponent(bufferColor.r) << 16);
      }
    }

    StretchDIBits(
      windowDC,
      0,
      0,
      g_windowWidth,
      g_windowHeight,
      0,
      0,
      g_backBufferInfo.bmiHeader.biWidth,
      g_backBufferInfo.bmiHeader.biHeight,
      g_backBufferMemory,
      &g_backBufferInfo,
      DIB_RGB_COLORS,
      SRCCOPY);
  }
  
  return 0;
}
