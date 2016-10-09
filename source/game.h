#pragma once

#include "platform.h"

struct mouse_info {
	s32 x, y;
	b32 lmb, rmb;
};

typedef struct game_data {
	void *memory;
	u32 memory_size;

	const u8 *kb;
	struct mouse_info mouse;

	SDL_Window *window;
	s32 window_w;
	s32 window_h;

	// TODO: add pointers to platform functions... or not, we'll se how it'll go
} game_data;

b32 game_update(struct game_data *data, struct gl_functions gl, float delta_time);
