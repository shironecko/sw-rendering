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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <stdlib.h>
#include <stdio.h>

u64 PlatformGetFileSize(const char* path)
{
  struct stat fileStats;
  i32 statResult = stat(path, &fileStats);

  if (statResult != 0)
    return 0;

  return fileStats.st_size;
}

u32 PlatformLoadFile(const char* path, void* memory, u32 memorySize)
{
  i32 file = open(path, O_RDONLY);

  if (file == -1)
    return 0;

  ssize_t readResult = read(file, memory, memorySize);
  close(file);

  return readResult == -1 ? 0 : readResult;
}

bool PlatformWriteFile(const char* path, void* memory, u32 bytesToWrite)
{
  // TODO: handle directory creation

  i32 file = open(path, O_WRONLY | O_CREAT | O_TRUNC);
  if (file == -1)
    return false;

  ssize_t writeResult = write(file, memory, bytesToWrite);
  close(file);

  return writeResult == bytesToWrite;
}

void Linux32SetupRenderingBuffers(
    RenderTarget* renderTarget, 
    u32 width, 
    u32 height, 
    void* buffersMemory, 
    u32 buffersMemorySize)
{
  u8* memory = (u8*)buffersMemory;
  renderTarget->texture = (Texture*)memory;
  memory += sizeof(Texture) + width * height * sizeof(Color32);
  renderTarget->texture->width = width;
  renderTarget->texture->height = height;

  renderTarget->zBuffer = (float*)memory;
  memory += sizeof(float) * width * height;
}

int main(int argc, char** argv)
{
  Display* display = XOpenDisplay((char *)0);
  i32 screen = DefaultScreen(display);
  u32 black = BlackPixel(display,screen);
  u32 white = WhitePixel(display, screen);

  u32 windowWidth = 640;
  u32 windowHeight = 480;

  Window window = XCreateSimpleWindow(display, DefaultRootWindow(display),0,0, windowWidth, windowHeight, 5, white, black);
  XSetStandardProperties(display, window,"Software Renderer","Hoi!",None,NULL,0,NULL);
  XSelectInput(display, window, ButtonPressMask | KeyPressMask);

  GC graphicsContext = XCreateGC(display, window, 0,0);        
  XSetBackground(display,graphicsContext,white);
  XSetForeground(display,graphicsContext,black);

  XClearWindow(display, window);
  XMapRaised(display, window);

   //*****ALLOCATING MEMORY*****//
  u32 platformMemorySize = 128 * Mb;
  void* platformMemory = malloc(platformMemorySize);
  assert(platformMemory);

  u32 gameMemorySize = 512 * Mb;
  void* gameMemory = malloc(gameMemorySize);
  assert(gameMemory);

  RenderTarget renderBuffer{};
  Linux32SetupRenderingBuffers(
      &renderBuffer, 
      windowWidth, 
      windowHeight, 
      platformMemory,
      platformMemorySize);

  XEvent event;
  KeySym key;
  char text[255];
  Input input{};

  GameInitialize(gameMemory, gameMemorySize);

  bool continueRunning = true;
  do
  {    
    while (XPending(display))
    {
      XNextEvent(display, &event);
      if (event.type == KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) 
      {
        if (text[0] == 'q') 
          continueRunning = false;

        printf("You pressed the %c key!\n",text[0]);
      }

      if (event.type == ButtonPress) 
        printf("You pressed a button at (%i,%i)\n", event.xbutton.x,event.xbutton.y);
    }
  
    // GAME UPDATE
    bool gameWantsToContinue = GameUpdate(
        0.1f,
        gameMemory,
        gameMemorySize,
        &renderBuffer,
        &input);

    continueRunning &= gameWantsToContinue;
  } while (continueRunning);

  XFreeGC(display, graphicsContext);
  XDestroyWindow(display,window);
  XCloseDisplay(display); 
  return 0;
}
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
