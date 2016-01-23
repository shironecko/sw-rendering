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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

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

  i32 file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0660);
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

void DrawAQuad() {
} 

int main(int argc, char** argv)
{
  Display* display = XOpenDisplay((char *)0);

  u32 windowWidth = 640;
  u32 windowHeight = 480;


  GLint glAttributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo* visualInfo = glXChooseVisual(display, 0, glAttributes);
  Window rootWindow = DefaultRootWindow(display);
  Colormap colorMap = XCreateColormap(display, rootWindow, visualInfo->visual, AllocNone);

  XSetWindowAttributes setWindowAttributes;
  setWindowAttributes.colormap = colorMap;
  setWindowAttributes.event_mask = ExposureMask | KeyPressMask;

  Window window = XCreateWindow(display, rootWindow, 0, 0, 600, 600, 0, visualInfo->depth, InputOutput, visualInfo->visual, CWColormap | CWEventMask, &setWindowAttributes);
  XMapWindow(display, window);
  XSetStandardProperties(display, window,"Software Renderer","Hoi!",None,NULL,0,NULL);
  XSelectInput(display, window, ButtonPressMask | KeyPressMask);

  GLXContext glContext = glXCreateContext(display, visualInfo, NULL, GL_TRUE);
  glXMakeCurrent(display, window, glContext);

   //*****ALLOCATING MEMORY*****//
  u32 platformMemorySize = 128 * Mb;
  void* platformMemory = malloc(platformMemorySize);
  assert(platformMemory);

  u32 gameMemorySize = 512 * Mb;
  void* gameMemory = malloc(gameMemorySize);
  assert(gameMemory);

  //*****SETUP RENDERING STUFF*****//
  RenderTarget renderBuffer{};
  Linux32SetupRenderingBuffers(
      &renderBuffer, 
      windowWidth, 
      windowHeight, 
      platformMemory,
      platformMemorySize);

  GLuint renderTexture;
  glGenTextures(1, &renderTexture);
  glBindTexture(GL_TEXTURE_2D, renderTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glEnable(GL_TEXTURE_2D);

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

    // PUT FRAME ON THE SCREEN
    XWindowAttributes windowAttributes;
    XGetWindowAttributes(display, window, &windowAttributes);
    glViewport(0, 0, windowAttributes.width, windowAttributes.height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, 0.5f, 1.5f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0, 0, 1, 0, 0, 0, 0, 1, 0);

    glBindTexture(GL_TEXTURE_2D, renderTexture);
    glTexImage2D(
      GL_TEXTURE_2D,
      0,
      GL_RGBA,
      renderBuffer.texture->width,
      renderBuffer.texture->height,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      ((u8*)renderBuffer.texture) + sizeof(Texture));

    glBegin(GL_QUADS);
      glTexCoord2f(0, 0);
      glVertex3f( 0, 0, 0);

      glTexCoord2f(1, 0);
      glVertex3f( 1, 0, 0);

      glTexCoord2f(1, 1);
      glVertex3f( 1, 1, 0);

      glTexCoord2f(0, 1);
      glVertex3f( 0, 1, 0);
    glEnd();

    glXSwapBuffers(display, window);

    GLenum glError;
    while ((glError = glGetError()) != GL_NO_ERROR)
    {
      printf("OpenGL error: %s\n", gluErrorString(glError));
    }
    //return 0;
  
  } while (continueRunning);

  glXMakeCurrent(display, None, NULL);
  glXDestroyContext(display, glContext);
  XDestroyWindow(display,window);
  XCloseDisplay(display); 
  return 0;
}
