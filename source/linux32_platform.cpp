#include "platform_api.h"

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
#include <X11/XKBlib.h>
#include<GL/gl.h>
#include<GL/glx.h>
#include<GL/glu.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

u64 PlatformGetFileSize(const char* path)
{
  struct stat fileStats;
  s32 statResult = stat(path, &fileStats);

  if (statResult != 0)
    return 0;

  return fileStats.st_size;
}

u32 PlatformLoadFile(const char* path, void* memory, u32 memorySize)
{
  s32 file = open(path, O_RDONLY);

  if (file == -1)
    return 0;

  ssize_t readResult = read(file, memory, memorySize);
  close(file);

  return readResult == -1 ? 0 : readResult;
}

bool PlatformWriteFile(const char* path, void* memory, u32 bytesToWrite)
{
  // TODO: handle directory creation

  s32 file = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0660);
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

timespec TimespecDiff(timespec start, timespec end)
{
  timespec temp;
  if ((end.tv_nsec-start.tv_nsec) < 0) 
  {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } 
  else 
  {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }

  return temp;
}

global const u32 g_keyMap[0xffff + 1] =
{
  [XK_BackSpace]    = KbKey::Backspace,
  [XK_Tab]          = KbKey::Tab,
  [XK_Return]       = KbKey::Return,
  [XK_Escape]       = KbKey::Escape,
  [XK_Delete]       = KbKey::Delete,

  [XK_Left]         = KbKey::Left,
  [XK_Up]           = KbKey::Up,
  [XK_Right]        = KbKey::Right,
  [XK_Down]         = KbKey::Down,
  [XK_Page_Up]      = KbKey::PageUp,
  [XK_Page_Down]    = KbKey::PageDown,
  [XK_End]          = KbKey::End,
  [XK_Home]         = KbKey::Home,

  [XK_F1]           = KbKey::F1,
  [XK_F2]           = KbKey::F2,
  [XK_F3]           = KbKey::F3,
  [XK_F4]           = KbKey::F4,
  [XK_F5]           = KbKey::F5,
  [XK_F6]           = KbKey::F6,
  [XK_F7]           = KbKey::F7,
  [XK_F8]           = KbKey::F8,
  [XK_F9]           = KbKey::F9,
  [XK_F10]          = KbKey::F10,
  [XK_F11]          = KbKey::F11,
  [XK_F12]          = KbKey::F12,

  [XK_Shift_L]      = KbKey::ShiftL,
  [XK_Shift_R]      = KbKey::ShiftR,
  [XK_Control_L]    = KbKey::ControlL,
  [XK_Control_R]    = KbKey::ControlR,

  [XK_0]            = KbKey::N_0,
  [XK_1]            = KbKey::N_1,
  [XK_2]            = KbKey::N_2,
  [XK_3]            = KbKey::N_3,
  [XK_4]            = KbKey::N_4,
  [XK_5]            = KbKey::N_5,
  [XK_6]            = KbKey::N_6,
  [XK_7]            = KbKey::N_7,
  [XK_8]            = KbKey::N_8,
  [XK_9]            = KbKey::N_9,
  [XK_colon]        = KbKey::Colon,
  [XK_semicolon]    = KbKey::Semicolon,
  [XK_less]         = KbKey::Comma,
  [XK_greater]      = KbKey::Period,
  [XK_A]            = KbKey::A,
  [XK_B]            = KbKey::B,
  [XK_C]            = KbKey::C,
  [XK_D]            = KbKey::D,
  [XK_E]            = KbKey::E,
  [XK_F]            = KbKey::F,
  [XK_G]            = KbKey::G,
  [XK_H]            = KbKey::H,
  [XK_I]            = KbKey::I,
  [XK_J]            = KbKey::J,
  [XK_K]            = KbKey::K,
  [XK_L]            = KbKey::L,
  [XK_M]            = KbKey::M,
  [XK_N]            = KbKey::N,
  [XK_O]            = KbKey::O,
  [XK_P]            = KbKey::P,
  [XK_Q]            = KbKey::Q,
  [XK_R]            = KbKey::R,
  [XK_S]            = KbKey::S,
  [XK_T]            = KbKey::T,
  [XK_U]            = KbKey::U,
  [XK_V]            = KbKey::V,
  [XK_W]            = KbKey::W,
  [XK_X]            = KbKey::X,
  [XK_Y]            = KbKey::Y,
  [XK_Z]            = KbKey::Z,

  [XK_a]            = KbKey::A,
  [XK_b]            = KbKey::B,
  [XK_c]            = KbKey::C,
  [XK_d]            = KbKey::D,
  [XK_e]            = KbKey::E,
  [XK_f]            = KbKey::F,
  [XK_g]            = KbKey::G,
  [XK_h]            = KbKey::H,
  [XK_i]            = KbKey::I,
  [XK_j]            = KbKey::J,
  [XK_k]            = KbKey::K,
  [XK_l]            = KbKey::L,
  [XK_m]            = KbKey::M,
  [XK_n]            = KbKey::N,
  [XK_o]            = KbKey::O,
  [XK_p]            = KbKey::P,
  [XK_q]            = KbKey::Q,
  [XK_r]            = KbKey::R,
  [XK_s]            = KbKey::S,
  [XK_t]            = KbKey::T,
  [XK_u]            = KbKey::U,
  [XK_v]            = KbKey::V,
  [XK_w]            = KbKey::W,
  [XK_x]            = KbKey::X,
  [XK_y]            = KbKey::Y,
  [XK_z]            = KbKey::Z,

  [XK_grave]        = KbKey::Grave,
};

int main(int argc, char** argv)
{
  //*****CREATING A WINDOW*****//
  Display* display = XOpenDisplay((char *)0);

  GLint glAttributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
  XVisualInfo* visualInfo = glXChooseVisual(display, 0, glAttributes);
  Window rootWindow = DefaultRootWindow(display);
  Colormap colorMap = XCreateColormap(display, rootWindow, visualInfo->visual, AllocNone);

  XSetWindowAttributes setWindowAttributes;
  setWindowAttributes.colormap = colorMap;
  setWindowAttributes.event_mask = ExposureMask | KeyPressMask;

  u32 windowWidth = 640;
  u32 windowHeight = 480;
  Window window = XCreateWindow(display, rootWindow, 0, 0, 600, 600, 0, visualInfo->depth, InputOutput, visualInfo->visual, CWColormap | CWEventMask, &setWindowAttributes);
  XMapWindow(display, window);
  XSetStandardProperties(display, window,"Software Renderer","Hoi!",None,NULL,0,NULL);
  XSelectInput(display, window, ButtonPressMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask);
  XkbSetDetectableAutoRepeat(display, True, nullptr);

  GLXContext glContext = glXCreateContext(display, visualInfo, NULL, GL_TRUE);
  glXMakeCurrent(display, window, glContext);
  Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
  XSetWMProtocols(display, window, &wmDeleteMessage, 1);

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
  bool kbState[KbKey::LastKey] {};

  GameInitialize(gameMemory, gameMemorySize);

  bool continueRunning = true;
  float deltaTime = 0;
  do
  {
    timespec startFrameTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startFrameTime);

    while (XPending(display))
    {
      XNextEvent(display, &event);
      if (event.type == KeyPress || event.type == KeyRelease)
      {
        KeySym key = XLookupKeysym(&event.xkey, 0);
        assert(u32(key) <= 0xffff + 1);

        u32 kbStateIdx = g_keyMap[u32(key)];
        assert(kbStateIdx < KbKey::LastKey);

        kbState[kbStateIdx] = event.type == KeyPress;
      }

      switch (event.type)
      {
        case ConfigureNotify:
        {
          XConfigureEvent xce = event.xconfigure;

          if (xce.width != windowWidth ||
              xce.height != windowHeight) 
          {
            windowWidth = xce.width;
            windowHeight = xce.height;

            Linux32SetupRenderingBuffers(
                &renderBuffer, 
                windowWidth, 
                windowHeight, 
                platformMemory,
                platformMemorySize);
          }
        } break;

        case ClientMessage:
        {
          // yeah, this is bizzare, can't do much about it :(
          if (event.xclient.data.l[0] == wmDeleteMessage)
              continueRunning = false;

        } break;
      };
    }

    // GAME UPDATE
    bool gameWantsToContinue = GameUpdate(
        deltaTime,
        gameMemory,
        gameMemorySize,
        &renderBuffer,
        kbState);

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

    timespec endFrameTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endFrameTime);
    timespec elapsedTime = TimespecDiff(startFrameTime, endFrameTime);
    s32 elapsedMlsec = elapsedTime.tv_nsec / 1000 / 1000;
    elapsedMlsec += elapsedTime.tv_sec * 1000;
    deltaTime = float(elapsedMlsec) / 1000.0f;

    char windowTitle[255];
    snprintf(windowTitle, 255, "Software Renderer: %3dms", elapsedMlsec);
    XStoreName(display, window, windowTitle);
  
  } while (continueRunning);

  glXMakeCurrent(display, None, NULL);
  glXDestroyContext(display, glContext);
  XDestroyWindow(display,window);
  XCloseDisplay(display); 
  return 0;
}
