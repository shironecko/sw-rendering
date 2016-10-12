typedef struct {
	s32 x, y;
	b32 lmb, rmb;
} mouse_info;

typedef struct {
	void *memory;
	u32 memory_size;

	const u8 *kb;
	mouse_info mouse;

	SDL_Window *window;
	s32 window_w;
	s32 window_h;

	// TODO: add pointers to platform functions... or not, we'll se how it'll go
} game_data;

#if defined(__IPHONEOS__) || defined(__ANDROID__) || defined(__EMSCRIPTEN__) || defined(__NACL__)
#define HAVE_OPENGLES
#endif

#if defined(HAVE_OPENGLES)
#include <SDL_opengles2.h>
#else
#include <SDL_opengl.h>
#endif

typedef struct {
#define gl_function(ret, func, params) ret(APIENTRY *func) params;
#include <gl_functions.h>
#undef gl_function
} gl_functions ;

b32 game_update(game_data *data, gl_functions gl, float delta_time);
