#define _HAS_EXCEPTIONS 0
#define _STATIC_CPPLIB

#include <windows.h>
#include <cassert>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "types.h"
#include "math3d.cpp"
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
  g_windowWidth = 1280;
  g_windowHeight = 720;

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

  // model loading will go here
  const char* modelPath = "../data/Grimoire.obj";
  //const char* modelPath = "../data/head.obj";
  std::vector<Vector4> vertices;
  std::vector<ModelFace> faces;

  {
    std::fstream file;
    file.open(modelPath, std::ios_base::in);
    assert(file.is_open());
    std::string input;

    while (std::getline(file, input))
    {
      std::stringstream stream;
      stream.str(input);
      char lineHeader;
      stream >> lineHeader;
      switch (lineHeader)
      {
        case 'v':
        {
          float x, y, z;
          stream >> x >> y >> z;

          vertices.push_back(Vector4 { x, y, z, 1.0f });
        } break;
        case 'f':
        {
          u32 v1, v2, v3;
          stream >> v1;
          stream.ignore(u32(-1), ' ');
          stream >> v2;
          stream.ignore(u32(-1), ' ');
          stream >> v3;

          faces.push_back(ModelFace { { v1 - 1, v2 - 1, v3 - 1 } });

          stream.ignore(u32(-1), ' ');
          u32 v4;
          if (stream >> v4)
          {
            faces.push_back(ModelFace { { v3 - 1, v4 - 1, v1 - 1 } });
          }
        } break;
      }
    }
  }


  HDC windowDC = GetDC(window);
  MSG message {};
  bool keys[256] {};
  float camDistance = 2.0f;
  float camMoveSpeed = 0.05f;
  float camRotation = 0;
  float camRotationSpeed = 0.1f;
  while (g_shouldRun)
  {
    while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&message);

      if (message.message == WM_KEYDOWN)
        keys[message.wParam] = true;
      else if (message.message == WM_KEYUP)
        keys[message.wParam] = false;

      DispatchMessage(&message);
    }

    if (keys[VK_DOWN])
      camDistance += camMoveSpeed;
    if (keys[VK_UP])
      camDistance -= camMoveSpeed;
    if (keys[VK_RIGHT])
      camRotation += camRotationSpeed;
    if (keys[VK_LEFT])
      camRotation -= camRotationSpeed;

    Render(*g_renderBuffer, vertices, faces, camDistance, camRotation);
    for (u32 y = 0; y < g_renderBuffer->Height(); ++y)
    {
      for (u32 x = 0; x < g_renderBuffer->Width(); ++x)
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
