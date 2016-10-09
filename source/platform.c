#define _CRT_SECURE_NO_WARNINGS

#include "platform.h"
#include "game.h"

#if defined(__WINDOWS__)
#define PT_GAME_DYNAMIC
#endif

#if !defined(PT_GAME_DYNAMIC)
#include "game.c"
#endif

#if defined(__WINDOWS__)
#include <Windows.h>
#define platform_alloc(size, base_address)                                                         \
	VirtualAlloc((base_address), (size), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE)
#else
#include <stdlib.h>
#define platform_alloc(size, base_address) malloc(size)
#endif

LOCAL s32 load_gl_functions(struct gl_functions *data) {
#if SDL_VIDEO_DRIVER_UIKIT || SDL_VIDEO_DRIVER_ANDROID || SDL_VIDEO_DRIVER_PANDORA
#define __SDL_NOGETPROCADDR__
#endif

#if defined __SDL_NOGETPROCADDR__
#define gl_function(ret, func, params) data->func = func;
#else
#define gl_function(ret, func, params)                                                             \
	do {                                                                                           \
		data->func = SDL_GL_GetProcAddress(#func);                                                 \
		if (!data->func) {                                                                         \
			return SDL_SetError("Couldn't load GL function %s: %s\n", #func, SDL_GetError());      \
		}                                                                                          \
	} while (0);
#endif /* __SDL_NOGETPROCADDR__ */

#include "gl_functions.h"
#undef gl_function
	return 0;
}

struct game_lib_info {
	b32 (*game_update)(struct game_data *data, struct gl_functions gl, float delta_time);

#if defined(PT_GAME_DYNAMIC)
	void *library;
	FILETIME last_write_time;
#endif
};

// TODO: make this shipping friendly way down the line
LOCAL void reload_game_lib(char *src_lib_path, char *temp_lib_path, char *lock_file_path,
                           struct game_lib_info *info) {
#if defined(PT_GAME_DYNAMIC)
	SDL_RWops *lock_file = SDL_RWFromFile(lock_file_path, "r");
	if (lock_file) {
		SDL_assert(info->library);
		SDL_RWclose(lock_file);
		return;
	}

	HANDLE game_lib_file = CreateFileA(src_lib_path, GENERIC_READ, FILE_SHARE_READ,
	                                   0, // security attributes
	                                   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
	                                   0); // template file

	if (game_lib_file == INVALID_HANDLE_VALUE) {
		SDL_assert(info->library);
		return;
	}

	FILETIME last_write_time;
	GetFileTime(game_lib_file, 0, 0, &last_write_time);

	if (info->library &&
	    memcmp(&info->last_write_time, &last_write_time, sizeof(last_write_time)) == 0) {
		CloseHandle(game_lib_file);
		return;
	}

	OutputDebugStringA("Reloading game lib...\n");
	SDL_UnloadObject(info->library);
	HANDLE temp_lib_file = CreateFile(temp_lib_path, GENERIC_WRITE, FILE_SHARE_READ,
	                                  0, // security attributes
	                                  CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
	                                  0); // template file

	// NOTE: all this hassle just cause CopyFile is failing for a second or so
	// after lib compile, thus adding lag to reload
	DWORD bytes_read;
	do {
		u8 buffer[64];
		ReadFile(game_lib_file, buffer, sizeof(buffer), &bytes_read, 0);
		WriteFile(temp_lib_file, buffer, bytes_read, 0, 0);
	} while (bytes_read);

	CloseHandle(game_lib_file);
	CloseHandle(temp_lib_file);

	info->library = SDL_LoadObject(temp_lib_path);
	info->game_update = SDL_LoadFunction(info->library, "game_update");
	info->last_write_time = last_write_time;
#else
	info->game_update = game_update;
#endif
}

int main(int argc, char **argv) {
	struct game_data game_data = {0};
	game_data.memory_size = 64 * Mb;
	void *game_mem_base_address =
#ifdef PT_DEV_BUILD
	    // TODO: make this 32bit friendly (?)
	    (void *)(2048ULL * Gb);
#else
	    0;
#endif
	game_data.memory = platform_alloc(game_data.memory_size, game_mem_base_address);
	memset(game_data.memory, 0, game_data.memory_size);

	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
	game_data.window =
	    SDL_CreateWindow("demo window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
	                     SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 0);
	SDL_GLContext context = SDL_GL_CreateContext(game_data.window);
	SDL_GL_MakeCurrent(game_data.window, context);

	struct gl_functions gl_functions;
	s32 context_load_res = load_gl_functions(&gl_functions);
	SDL_assert(context_load_res >= 0);

	struct game_lib_info game_lib = {0};
	b32 continueRunning = true;

	do {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) continueRunning = false;
		}

		// TODO: strip reloading from shipping version
		reload_game_lib("..\\build\\game.dll", "..\\build\\game_tmp.dll", "..\\build\\game.lock",
		                &game_lib);
		SDL_assert(game_lib.game_update);
		SDL_GL_GetDrawableSize(game_data.window, &game_data.window_w, &game_data.window_h);
		game_data.kb = SDL_GetKeyboardState(0);
		s32 mouse_buttons = SDL_GetMouseState(&game_data.mouse.x, &game_data.mouse.y);
		game_data.mouse.lmb = mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT);
		game_data.mouse.rmb = mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT);

		// TODO: timing
		continueRunning = game_lib.game_update(&game_data, gl_functions, 0.17f);

	} while (continueRunning);

	return 0;
}
