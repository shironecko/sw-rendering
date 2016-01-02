#define assert(x) __nop()

#include "types.cpp"

u64 PlatformGetFileSize(const char* path);
u32 PlatformLoadFile(const char* path, void* memory, u32 memorySize);
bool PlatformWriteFile(const char* path, void* memory, u32 bytesToWrite);

#ifdef GAME_PROJECT
#include "game.cpp"
#elif defined(RESOURCE_CONVERTER_PROJECT)
#include "resource_converter.cpp"
#else
#error "You did not specified project type!"
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <stdio.h>

u64 PlatformGetFileSize(const char* path)
{
  return 0;
}

u32 PlatformLoadFile(const char* path, void* memory, u32 memorySize)
{
  return 0;
}

bool PlatformWriteFile(const char* path, void* memory, u32 bytesToWrite)
{
  return false;
}

struct
{
  bool            shouldRun           = true;
  u32             windowWidth         = 640;
  u32             windowHeight        = 480;

  void*           memory              = nullptr;
  u32             memorySize          = 0;
  RenderTarget    renderBuffer        {};

} g_platformData;

int main(int argc, char** argv)
{
  Display *display;
  int screen;
  Window window;
  GC graphicsContext;
  
  const u32 windowWidth = 1280;
  const u32 windowHeight = 720;

  /* get the colors black and white (see section for details) */
  unsigned long black,white;

  /* use the information from the environment variable DISPLAY 
     to create the X connection:
  */  
  display = XOpenDisplay((char *)0);
  screen = DefaultScreen(display);
  black = BlackPixel(display,screen), /* get color black */
  white = WhitePixel(display, screen);  /* get color white */

  /* once the display is initialized, create the window.
     This window will be have be 200 pixels across and 300 down.
     It will have the foreground white and background black
  */
  window = XCreateSimpleWindow(display, DefaultRootWindow(display),0,0, windowWidth, windowHeight, 5, white, black);

  /* here is where some properties of the window can be set.
     The third and fourth items indicate the name which appears
     at the top of the window and the name of the minimized window
     respectively.
  */
  XSetStandardProperties(display, window,"Software Renderer","Hoi!",None,NULL,0,NULL);

  /* this routine determines which types of input are allowed in
     the input.  see the appropriate section for details...
  */
  XSelectInput(display, window, ExposureMask|ButtonPressMask|KeyPressMask);

  /* create the Graphics Context */
  graphicsContext = XCreateGC(display, window, 0,0);        

  /* here is another routine to set the foreground and background
     colors _currently_ in use in the window.
  */
  XSetBackground(display,graphicsContext,white);
  XSetForeground(display,graphicsContext,black);

  /* clear the window and bring it on top of the other windows */
  XClearWindow(display, window);
  XMapRaised(display, window);

  // GAME LOOP
  XEvent event;   /* the XEvent declaration !!! */
  KeySym key;   /* a dealie-bob to handle KeyPress Events */  
  char text[255];   /* a char buffer for KeyPress Events */

  /* look for events forever... */
  while(1) 
  {    
    /* get the next event and stuff it into our event variable.
       Note:  only events we set the mask for are detected!
    */
    XNextEvent(display, &event);
  
    if (event.type == Expose && event.xexpose.count == 0) 
    {
      /* the window was exposed redraw it! */
      //redraw();
    }

    if (event.type == KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) 
    {
      /* use the XLookupString routine to convert the invent
         KeyPress data into regular text.  Weird but necessary...
      */
      if (text[0] == 'q') 
        break;

      printf("You pressed the %c key!\n",text[0]);
    }
    if (event.type == ButtonPress) 
    {
      /* tell where the mouse Button was Pressed */
      printf("You pressed a button at (%i,%i)\n", event.xbutton.x,event.xbutton.y);
    }
  }

  XFreeGC(display, graphicsContext);
  XDestroyWindow(display,window);
  XCloseDisplay(display); 

  return 0;
}

/* int CALLBACK WinMain( */
/*   HINSTANCE instance, */
/*   HINSTANCE /1* prevInstance *1/, */
/*   char*     /1* cmdLine *1/, */
/*   int       /1* cmdShow *1/) */
/* { */
   //*****CREATING A WINDOW*****//
/*   WNDCLASSEX wndClass {}; */
/*   wndClass.cbSize = sizeof(wndClass); */
/*   wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC; */
/*   wndClass.lpfnWndProc = Win32WindowProc; */
/*   wndClass.hInstance = instance; */
/*   wndClass.lpszClassName = "Software Renderer Window Class Name"; */

/*   RegisterClassEx(&wndClass); */

/*   HWND window = CreateWindowEx( */
/*     0, */
/*     wndClass.lpszClassName, */
/*     "Software Renderer", */
/*     WS_OVERLAPPEDWINDOW | WS_VISIBLE, */
/*     CW_USEDEFAULT, */
/*     CW_USEDEFAULT, */
/*     g_platformData.windowWidth, */
/*     g_platformData.windowHeight, */
/*     0, */
/*     0, */
/*     instance, */
/*     nullptr */
/*   ); */

   //*****ALLOCATING MEMORY*****//
/*   g_platformData.memorySize = 128 * Mb; */
/*   g_platformData.memory = VirtualAlloc( */
/*     nullptr, */
/*     g_platformData.memorySize, */
/*     MEM_RESERVE | MEM_COMMIT, */
/*     PAGE_READWRITE */
/*   ); */
/*   assert(g_platformData.memory); */

/*   u32   gameMemorySize = 512 * Mb; */
/*   void* gameMemory = VirtualAlloc( */
/*     nullptr, */
/*     gameMemorySize, */
/*     MEM_RESERVE | MEM_COMMIT, */
/*     PAGE_READWRITE */
/*   ); */
/*   assert(gameMemory); */

   //*****MISC SETUP*****//
/*   Win32SetupRenderingBuffers(g_platformData.windowWidth, g_platformData.windowHeight); */

/*   HDC windowDC = GetDC(window); */
/*   MSG message {}; */
/*   Input input {}; */

/*   LARGE_INTEGER lastFrameTime; */
/*   LARGE_INTEGER queryFrequency; */
/*   QueryPerformanceCounter(&lastFrameTime); */
/*   QueryPerformanceFrequency(&queryFrequency); */

/*   GameInitialize(gameMemory, gameMemorySize); */

   //*****RENDERING LOOP*****//
/*   while (g_platformData.shouldRun) */
/*   { */
/*     LARGE_INTEGER currentFrameTime; */
/*     QueryPerformanceCounter(&currentFrameTime); */
/*     u64 ticksElapsed = currentFrameTime.QuadPart - lastFrameTime.QuadPart; */
/*     float deltaTime = float(ticksElapsed) / float(queryFrequency.QuadPart); */
/*     lastFrameTime = currentFrameTime; */

/*     // TODO: sort this out */
/*     char windowTitle[256]; */
/*     wsprintf(windowTitle, "Software Renderer \t %ums per frame", u32(deltaTime * 1000.0f)); */
/*     SetWindowText(window, windowTitle); */

/*     while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) */
/*     { */
/*       TranslateMessage(&message); */

/*       if (message.message == WM_KEYDOWN) */
/*         input.keyboard[message.wParam] = true; */
/*       else if (message.message == WM_KEYUP) */
/*         input.keyboard[message.wParam] = false; */

/*       DispatchMessage(&message); */
/*     } */

/*     bool gameWantsToContinue = GameUpdate( */
/*         deltaTime, */
/*         gameMemory, */
/*         gameMemorySize, */
/*         &g_platformData.renderBuffer, */
/*         &input); */

/*     g_platformData.shouldRun &= gameWantsToContinue; */

/*     Win32PresentToWindow( */
/*       windowDC, */
/*       g_platformData.windowWidth, */
/*       g_platformData.windowHeight, */
/*       &g_platformData.backBuffer, */
/*       &g_platformData.renderBuffer); */
/*   } */
  
/*   return 0; */
/* } */
