#include <windows.h>

#include "types.h"
#include "renderer.cpp"

u8 CompressColorComponent(float component)
{
  return u8(component * 255.0f);
}

LRESULT CALLBACK WindowProc(
  HWND   window,
  UINT   message,
  WPARAM wParam,
  LPARAM lParam)
{
  switch (message)
  {
    case WM_CLOSE:
    {
      PostQuitMessage(0);
      return 0;
    } break;
  }

  return DefWindowProc(window, message, wParam, lParam);
}

int CALLBACK WinMain(
  HINSTANCE instance,
  HINSTANCE /* prevInstance */,
  LPSTR     /* cmdLine */,
  int       /* cmdShow */)
{
  const int windowWidth = 640;
  const int windowHeight = 480;

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
    windowWidth,
    windowHeight,
    0,
    0,
    instance,
    nullptr
  );

  BITMAPINFO backBufferInfo {};
  backBufferInfo.bmiHeader.biSize = sizeof(backBufferInfo.bmiHeader);
  backBufferInfo.bmiHeader.biWidth = windowWidth;
  backBufferInfo.bmiHeader.biHeight = windowHeight;
  backBufferInfo.bmiHeader.biPlanes = 1;
  backBufferInfo.bmiHeader.biBitCount = 32;
  backBufferInfo.bmiHeader.biCompression = BI_RGB;
  u32* backBufferMemory = new u32[windowWidth * windowHeight];

  Bitmap renderBuffer(windowWidth, windowHeight);
  HDC windowDC = GetDC(window);

  MSG message {};
  do
  {
    while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }

    Render(renderBuffer);
    for (int y = 0; y < renderBuffer.Height(); ++y)
    {
      for (int x = 0; x < renderBuffer.Width(); ++x)
      {
        Color& bufferColor = renderBuffer(x, y);

        backBufferMemory[y * windowWidth + x] = 
            CompressColorComponent(bufferColor.b) |
            (CompressColorComponent(bufferColor.g) << 8) |
            (CompressColorComponent(bufferColor.r) << 16);
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
      windowWidth,
      windowHeight,
      backBufferMemory,
      &backBufferInfo,
      DIB_RGB_COLORS,
      SRCCOPY);
  } while (message.message != WM_QUIT);
  
  return 0;
}
