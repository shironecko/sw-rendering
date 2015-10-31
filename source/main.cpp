#include <windows.h>

LRESULT CALLBACK WindowProc(
  HWND   window,
  UINT   message,
  WPARAM wParam,
  LPARAM lParam)
{
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

  MSG message {};
  do
  {
    while (PeekMessage(&message, window, 0, 0, PM_REMOVE))
    {
      TranslateMessage(&message);
      DispatchMessage(&message);
    }
  } while (message.message != WM_QUIT);
  
  return 0;
}
