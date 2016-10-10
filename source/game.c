#define _CRT_SECURE_NO_WARNINGS
#include "game.h"
#include "math3d.cpp"
#include "sw_render.cpp"
#include "gl_render.c"
#include "text.cpp"
#include "assets.c"

#include <stdlib.h>

/* ---STB lib--- */
#define STB_DEFINE
#define STB_NO_REGISTRY
#pragma warning(push, 1)
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
#pragma warning(disable : 4701)
#include "stb.h"
#pragma warning(pop)

/* ---STB rect pack--- */
#define STB_RECT_PACK_IMPLEMENTATION
#include "stb_rect_pack.h"

/* ---STB truetype--- */
#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

/* ---Nuclear IMGUI--- */
#define NK_BYTE u8
#define NK_INT16 s16
#define NK_UINT16 u16
#define NK_INT32 s32
#define NK_UINT32 u32
#define NK_SIZE_TYPE usize
#define NK_POINTER_TYPE uptr

#define NK_IMPLEMENTATION
#define NK_PRIVATE
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_ASSERT SDL_assert
#pragma warning(push)
#pragma warning(disable : 4127)
#include "nuklear.h"
#pragma warning(pop)

#define arr_len(arr) (sizeof(arr) / sizeof(arr[0]))

#define REF_W 640
#define REF_H 480

struct ui_font {
	struct nk_user_font nk_font;
	float height;
	float ascent;
	float spacing;
	stbtt_packedchar packed_chars[256];
	GLuint texture;
	u32 tex_w, tex_h;
};
typedef struct {
	vec4 pos;
	float pitch;
	float yaw;
} Camera;

typedef struct game_state {
	//****************BASICS****************//
	b32 initialized;

	s32 window_w;
	s32 window_h;

	mem_pool pool;

	//****************UI****************//
	struct {
		GLuint id;
		GLint u_mvp;
		GLint a_pos;
		GLint a_col;
		GLuint v_array;
	} minimal_shader;

	struct {
		GLuint id;
		GLint u_tex;
		GLint u_mvp;
		GLint a_pos;
		GLint a_uv;
		GLint a_col;
		GLuint font_tex;
		GLuint v_array, i_array;
	} ui_shader;

	struct ui_font ui_font;

	struct nk_context ui_context;
	struct nk_buffer ui_cmd_buff;
	struct nk_draw_null_texture ui_draw_null_tex;
	struct nk_allocator ui_font_atlas_alloc;
	struct nk_panel ui_panel;

	// TODO: figure out some good numbers
	u8 ui_cmd_buff_mem[1 * Mb];
	u8 ui_pool_buff_mem[1 * Mb];

	u8 ui_vertex_buffer[512 * 1024];
	u8 ui_element_buffer[128 * 1024];
	
	//****************SW RENDERING****************//
	Camera camera;
	model model;
	u32 render_mode;
} game_state;

b32 IsKeyUp(b32 *lastKbState, b32 *kbState, u32 key) {
	assert(key < KbKey::Last);
	return lastKbState[key] && !kbState[key];
}

b32 IsKeyDown(b32 *lastKbState, b32 *kbState, u32 key) {
	assert(key < KbKey::Last);
	return !lastKbState[key] && kbState[key];
}

b32 GameUpdate(game_data *data, gl_functions gl_fns, float delta_time) {
	if (data->kb[SDL_SCANCODE_ESCAPE]) return false;

	game_state *state = (game_state *)data->memory;
	if (!state->initialized) {
		SDL_Log("Initializing game state...");
		state->initialized = true;

		state->render_mode = RenderMode::Shaded | RenderMode::Textured;
		LoadModel("./assets/", "muro.obj", &state->pool, &state->model);

		// minimal shader
		{
			const char *vertex_shader_src = "#version 100\n"
			                                "uniform mat4 mvp;\n"
			                                "attribute vec2 pos;\n"
			                                "attribute vec4 col;\n"
			                                "varying vec4 frag_col;\n"
			                                "void main() {\n"
			                                "   frag_col = col;\n"
			                                "   gl_Position = mvp * vec4(pos.xy, 0, 1);\n"
			                                "}\n";

			const char *fragment_shader_src = "#version 100\n"
			                                  "precision mediump float;\n"
			                                  "varying vec4 frag_col;\n"
			                                  "void main(){\n"
			                                  "   gl_FragColor = frag_col;\n"
			                                  "}\n";

			b32 result = compile_shader_program(gl_fns, &state->minimal_shader.id,
			                                     vertex_shader_src, fragment_shader_src);
			SDL_assert(result);

			gl(glGenBuffers(1, &state->minimal_shader.v_array));
		}
		gl_set(state->minimal_shader.u_mvp, glGetUniformLocation(state->minimal_shader.id, "mvp"));
		gl_set(state->minimal_shader.a_pos, glGetAttribLocation(state->minimal_shader.id, "pos"));
		gl_set(state->minimal_shader.a_col, glGetAttribLocation(state->minimal_shader.id, "col"));

		// UI shader
		{
			const char *vertex_shader_src = "#version 100\n"
			                                "uniform mat4 mvp;\n"
			                                "attribute vec2 pos;\n"
			                                "attribute vec2 uv;\n"
			                                "attribute vec4 col;\n"
			                                "varying vec2 frag_uv;\n"
			                                "varying vec4 frag_col;\n"
			                                "void main() {\n"
			                                "   frag_uv = uv;\n"
			                                "   frag_col = col;\n"
			                                "   gl_Position = mvp * vec4(pos.xy, 0, 1);\n"
			                                "}\n";

			const char *fragment_shader_src =
			    "#version 100\n"
			    "precision mediump float;\n"
			    "uniform sampler2D tex;\n"
			    "varying vec2 frag_uv;\n"
			    "varying vec4 frag_col;\n"
			    "void main(){\n"
			    "   gl_FragColor = frag_col * texture2D(tex, frag_uv.st);\n"
			    "}\n";

			b32 result = compile_shader_program(gl_fns, &state->ui_shader.id, vertex_shader_src,
			                                     fragment_shader_src);
			SDL_assert(result);
		}

		gl_set(state->ui_shader.u_tex, glGetUniformLocation(state->ui_shader.id, "tex"));
		gl_set(state->ui_shader.u_mvp, glGetUniformLocation(state->ui_shader.id, "mvp"));
		gl_set(state->ui_shader.a_pos, glGetAttribLocation(state->ui_shader.id, "pos"));
		gl_set(state->ui_shader.a_uv, glGetAttribLocation(state->ui_shader.id, "uv"));
		gl_set(state->ui_shader.a_col, glGetAttribLocation(state->ui_shader.id, "col"));

		gl(glGenBuffers(1, &state->ui_shader.v_array));
		gl(glGenBuffers(1, &state->ui_shader.i_array));

		// initialize Nuclear
		{
			state->ui_cmd_buff.type = NK_BUFFER_FIXED;
			state->ui_cmd_buff.memory.ptr = state->ui_cmd_buff_mem;
			state->ui_cmd_buff.memory.size = state->ui_cmd_buff.size =
			    sizeof(state->ui_cmd_buff_mem);
			state->ui_cmd_buff.memory = (struct nk_memory){.ptr = state->ui_cmd_buff_mem,
			                                               .size = sizeof(state->ui_cmd_buff_mem)};

			int nk_init_result =
			    nk_init_fixed(&state->ui_context, state->ui_pool_buff_mem,
			                  sizeof(state->ui_pool_buff_mem), &state->ui_font.nk_font);
			SDL_assert(nk_init_result);
		}
	}

	// WINDOW RESIZE HANDLING
	if (state->window_w != data->window_w || state->window_h != data->window_h) {
		char buff[1024];
		stb_snprintf(buff, 1024, "res change from(%d, %d), to(%d, %d)", state->window_w,
		             state->window_h, data->window_w, data->window_h);
		SDL_Log("%s", buff);

		float ref_aspect = (float)REF_H / (float)REF_W;
		float actual_aspect = (float)data->window_h / (float)data->window_w;
		float scale;
		if (ref_aspect > actual_aspect)
			scale = (float)data->window_h / (float)REF_H;
		else
			scale = (float)data->window_w / (float)REF_W;

		float hw = data->window_w * 0.5f;
		float hh = data->window_h * 0.5f;

		float scale_x = 1.0f / hw * scale;
		float scale_y = -1.0f / hh * scale;
		float shift_x = (data->window_w - REF_W * scale) / data->window_w - 1.0f;
		float shift_y = 1.0f - (data->window_h - REF_H * scale) / data->window_h;

		const float mvp[] = {scale_x, 0, 0, 0, 0,       scale_y, 0, 0,
		                     0,       0, 1, 0, shift_x, shift_y, 0, 1};

		// minimal shader MVP setup
		gl(glUseProgram(state->minimal_shader.id));
		gl(glUniformMatrix4fv(state->minimal_shader.u_mvp, 1, false, mvp));

		// ui shader MVP setup
		gl(glUseProgram(state->ui_shader.id));
		gl(glUniformMatrix4fv(state->ui_shader.u_mvp, 1, false, mvp));

		gl(glViewport(0, 0, data->window_w, data->window_h));

		state->window_w = data->window_w;
		state->window_h = data->window_h;

		// bake fonts
		{
			struct mem_pool pool = {.memory = state->memory, .memory_end = state->memory_end};
			struct stbtt_fontinfo font_info;
			SDL_RWops *font_file = SDL_RWFromFile("assets/hack.ttf", "rb");
			SDL_assert(font_file);
			u64 font_size = SDL_RWsize(font_file);
			void *font_contents = mem_push(&pool, font_size);
			SDL_RWread(font_file, font_contents, 1, SDL_RWsize(font_file));
			SDL_RWclose(font_file);

			s32 result = stbtt_InitFont(&font_info, (u8 *)font_contents, 0);
			SDL_assert(result);

			u32 w = 512, h = 512;
			u8 *font_texture_alpha = mem_push(&pool, w * h);
			struct stbtt_pack_context pack_context;
			result = stbtt_PackBegin(&pack_context, font_texture_alpha, w, h, 0, 1, 0);
			SDL_assert(result);

			float bake_height = 13.0f * scale;
			result = stbtt_PackFontRange(&pack_context, font_contents, 0, bake_height, 0x0000,
			                             arr_len(state->ui_font.packed_chars),
			                             state->ui_font.packed_chars);
			SDL_assert(result);
			stbtt_PackEnd(&pack_context);

			u32 *font_texture = mem_push(&pool, w * h * sizeof(*font_texture));
			for (u32 y = 0; y < h; ++y) {
				for (u32 x = 0; x < w; ++x) {
					u32 idx = y * w + x;
					font_texture[idx] = ((u32)font_texture_alpha[idx] << 24) | 0x00FFFFFF;
				}
			}

			state->ui_font.tex_w = w;
			state->ui_font.tex_h = h;
			gl(glGenTextures(1, &state->ui_font.texture));
			gl(glBindTexture(GL_TEXTURE_2D, state->ui_font.texture));
			gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
			gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
			gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			                font_texture));

			struct nk_user_font nk_font;
			nk_font.userdata.ptr = &state->ui_font;
			nk_font.height = 13.0f;
			nk_font.width = ui_text_width_fn;
			nk_font.query = ui_query_font_glyph_fn;
			nk_font.texture.id = (int)state->ui_font.texture;
			float font_scale = stbtt_ScaleForPixelHeight(&font_info, bake_height);
			int unscaled_ascent;
			stbtt_GetFontVMetrics(&font_info, &unscaled_ascent, 0, 0);
			state->ui_font.ascent = unscaled_ascent * font_scale;

			state->ui_font.nk_font = nk_font;
			state->ui_font.height = bake_height;
			state->ui_font.spacing = 0;

			GLuint null_texture;
			gl(glGenTextures(1, &null_texture));
			gl(glBindTexture(GL_TEXTURE_2D, null_texture));
			gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
			gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
			u8 null_texture_data[] = {255, 255, 255, 255};
			gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
			                null_texture_data));
			state->ui_draw_null_tex.texture.id = null_texture;
			state->ui_draw_null_tex.uv.x = 0;
			state->ui_draw_null_tex.uv.y = 0;
		}
	}

	// reset callbacks and do other stuff that needs to be done after every hot-reload of the code
	{
		state->ui_font.nk_font.width = ui_text_width_fn;
		state->ui_font.nk_font.query = ui_query_font_glyph_fn;
	}

	// pump events into UI
	{
		float ref_aspect = (float)REF_H / (float)REF_W;
		float actual_aspect = (float)data->window_h / (float)data->window_w;
		float scale;
		if (ref_aspect > actual_aspect)
			scale = (float)data->window_h / (float)REF_H;
		else
			scale = (float)data->window_w / (float)REF_W;

		float shift_x = (data->window_w - REF_W * scale) * 0.5f;
		float shift_y = (data->window_h - REF_H * scale) * 0.5f;

		s32 mx = (s32)((data->mouse.x - shift_x) * (1.0f / scale));
		s32 my = (s32)((data->mouse.y - shift_y) * (1.0f / scale));

		nk_input_begin(&state->ui_context);

		nk_input_motion(&state->ui_context, mx, my);
		nk_input_button(&state->ui_context, NK_BUTTON_LEFT, mx, my, data->mouse.lmb);
		nk_input_button(&state->ui_context, NK_BUTTON_RIGHT, mx, my, data->mouse.rmb);

		nk_input_end(&state->ui_context);
	}

	{
		// draw UI
		struct nk_panel layout;
		struct nk_context *ctx = &state->ui_context;
		struct nk_style *s = &ctx->style;
		s->button.text_background = nk_rgb(255, 0, 0);
		s->button.border_color = nk_rgb(0, 255, 0);
		s->button.normal = nk_style_item_color(nk_rgb(0, 0, 0));
		if (nk_begin(ctx, &layout, "Hello, Nuklear!", nk_rect(10, 10, 180, 250),
		             NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_MOVABLE)) {
			nk_layout_row_dynamic(ctx, 25.0f, 2);
			if (nk_button_label(ctx, "Button NNN")) SDL_Log("Hot reload, yay!");

			nk_button_label(ctx, "Button Two");
			nk_button_label(ctx, "Button Two");
			nk_button_label(ctx, "Button Two");
			nk_button_label(ctx, "Button Two");

			nk_layout_row_end(ctx);
		}
		nk_end(ctx);
	}

	gl(glClearColor(1.0f, 0.0f, 1.0f, 1.0f));
	gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	// render background (in ref res) fill
	{
		gl(glUseProgram(state->minimal_shader.id));
		gl(glBindBuffer(GL_ARRAY_BUFFER, state->minimal_shader.v_array));

		float r, g, b;
		r = g = b = 0.2f;
		struct {
			float pos[2];
			float col[4];
		} vertices[] = {{.pos = {0, 0}, .col = {r, g, b, 1.0f}},
		                {.pos = {REF_W, 0}, .col = {r, g, b, 1.0f}},
		                {.pos = {0, REF_H}, .col = {r, g, b, 1.0f}},
		                {.pos = {0, REF_H}, .col = {r, g, b, 1.0f}},
		                {.pos = {REF_W, 0}, .col = {r, g, b, 1.0f}},
		                {.pos = {REF_W, REF_H}, .col = {r, g, b, 1.0f}}};

		gl(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW));
		gl(glEnableVertexAttribArray(state->minimal_shader.a_pos));
		gl(glEnableVertexAttribArray(state->minimal_shader.a_col));
		gl(glVertexAttribPointer(state->minimal_shader.a_pos, 2, GL_FLOAT, GL_FALSE,
		                         sizeof(vertices[0]), 0));
		gl(glVertexAttribPointer(state->minimal_shader.a_col, 4, GL_FLOAT, GL_FALSE,
		                         sizeof(vertices[0]),
		                         (void *)((u8 *)&vertices[0].col - (u8 *)vertices)));

		gl(glDrawArrays(GL_TRIANGLES, 0, 6));
		// this fucking shit is not working on GLES2 for some reason
		/* u32 indices[] = { 0, 1, 2, 2, 1, 3 }; */
		/* gl(glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT,
		 * indices)); */
	}

	// render font texture
	{
	    /* gl(glEnable(GL_BLEND)); */
	    /* gl(glBlendEquation(GL_FUNC_ADD)); */
	    /* gl(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA)); */
	    /* gl(glDisable(GL_DEPTH_TEST)); */
	    /* gl(glUseProgram(state->ui_shader.id)); */
	    /* gl(glUniform1i(state->ui_shader.u_tex, 0)); */

	    /* GLsizei vs = sizeof(struct nk_draw_vertex); */
	    /* size_t vp = offsetof(struct nk_draw_vertex, position); */
	    /* size_t vt = offsetof(struct nk_draw_vertex, uv); */
	    /* size_t vc = offsetof(struct nk_draw_vertex, col); */

	    /* gl(glBindBuffer(GL_ARRAY_BUFFER, state->ui_shader.v_array)); */
	    /* /1* gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ui_shader.i_array)); *1/ */

	    /* gl(glEnableVertexAttribArray((GLuint)state->ui_shader.a_pos)); */
	    /* gl(glEnableVertexAttribArray((GLuint)state->ui_shader.a_uv)); */
	    /* gl(glEnableVertexAttribArray((GLuint)state->ui_shader.a_col)); */

	    /* gl(glVertexAttribPointer((GLuint)state->ui_shader.a_pos, 2, GL_FLOAT, GL_FALSE, vs,
	       (void*)vp)); */
	    /* gl(glVertexAttribPointer((GLuint)state->ui_shader.a_uv, 2, GL_FLOAT, GL_FALSE, vs,
	       (void*)vt)); */
	    /* gl(glVertexAttribPointer((GLuint)state->ui_shader.a_col, 4, GL_UNSIGNED_BYTE, GL_TRUE,
	       vs, (void*)vc)); */

	    /* struct nk_draw_vertex vertices[] = */
	    /* { */
	    /*     { { 0, 0 }, { 0, 0 }, 0xFFFFFFFF }, */
	    /*     { { REF_W, 0 }, { 1, 0 }, 0xFFFFFFFF }, */
	    /*     { { 0, REF_H }, { 0, 1 }, 0xFFFFFFFF }, */
	    /*     { { REF_W, REF_H }, { 1, 1 }, 0xFFFFFFFF } */
	    /* }; */

	    /* u32 indices[] = { 0, 1, 2, 2, 1, 3 }; */

	    /* gl(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STREAM_DRAW)); */
	    /* /1* gl(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STREAM_DRAW));
	       *1/ */

	    /* gl(glActiveTexture(GL_TEXTURE0)); */
	    /* gl(glBindTexture(GL_TEXTURE_2D, (GLuint)state->ui_font.texture)); */
	    /* gl(glDrawElements(GL_TRIANGLES, sizeof(indices) / sizeof(indices[0]), GL_UNSIGNED_INT,
	       indices)); */
	}

	// render UI
	{
		/* setup GLOBAL state */
		gl(glEnable(GL_BLEND));
		gl(glBlendEquation(GL_FUNC_ADD));
		gl(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		gl(glDisable(GL_DEPTH_TEST));
		gl(glActiveTexture(GL_TEXTURE0));

		/* setup program */
		gl(glUseProgram(state->ui_shader.id));
		gl(glUniform1i(state->ui_shader.u_tex, 0));
		{
			/* convert from command queue into draw list and draw to screen */
			const struct nk_draw_command *cmd;
			const nk_draw_index *offset = NULL;

			/* load draw vertices & elements directly into vertex + element buffer */
			{
				/* fill converting configuration */
				struct nk_convert_config config;
				memset(&config, 0, sizeof(config));
				config.global_alpha = 1.0f;
				config.shape_AA = NK_ANTI_ALIASING_ON;
				config.line_AA = NK_ANTI_ALIASING_ON;
				config.circle_segment_count = 22;
				config.curve_segment_count = 22;
				config.arc_segment_count = 22;
				config.null = state->ui_draw_null_tex;

				/* setup buffers to load vertices and elements */
				{
					struct nk_buffer vbuf, ebuf;
					nk_buffer_init_fixed(&vbuf, state->ui_vertex_buffer,
					                     sizeof(state->ui_vertex_buffer));
					nk_buffer_init_fixed(&ebuf, state->ui_element_buffer,
					                     sizeof(state->ui_element_buffer));
					nk_convert(&state->ui_context, &state->ui_cmd_buff, &vbuf, &ebuf, &config);
				}
			}

			/* buffer setup */
			GLsizei vs = sizeof(struct nk_draw_vertex);
			size_t vp = offsetof(struct nk_draw_vertex, position);
			size_t vt = offsetof(struct nk_draw_vertex, uv);
			size_t vc = offsetof(struct nk_draw_vertex, col);

			gl(glBindBuffer(GL_ARRAY_BUFFER, state->ui_shader.v_array));
			gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state->ui_shader.i_array));

			gl(glEnableVertexAttribArray((GLuint)state->ui_shader.a_pos));
			gl(glEnableVertexAttribArray((GLuint)state->ui_shader.a_uv));
			gl(glEnableVertexAttribArray((GLuint)state->ui_shader.a_col));

			gl(glVertexAttribPointer((GLuint)state->ui_shader.a_pos, 2, GL_FLOAT, GL_FALSE, vs,
			                         (void *)vp));
			gl(glVertexAttribPointer((GLuint)state->ui_shader.a_uv, 2, GL_FLOAT, GL_FALSE, vs,
			                         (void *)vt));
			gl(glVertexAttribPointer((GLuint)state->ui_shader.a_col, 4, GL_UNSIGNED_BYTE, GL_TRUE,
			                         vs, (void *)vc));

			gl(glBufferData(GL_ARRAY_BUFFER, sizeof(state->ui_vertex_buffer),
			                state->ui_vertex_buffer, GL_STREAM_DRAW));
			gl(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(state->ui_element_buffer),
			                state->ui_element_buffer, GL_STREAM_DRAW));

			/* iterate over and execute each draw command */
			nk_draw_foreach(cmd, &state->ui_context, &state->ui_cmd_buff) {
				if (!cmd->elem_count) continue;
				gl(glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id));
				gl(glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT,
				                  offset));
				offset += cmd->elem_count;
			}
			nk_clear(&state->ui_context);
		}

		/* default OpenGL state */
		gl(glUseProgram(0));
		gl(glBindBuffer(GL_ARRAY_BUFFER, 0));
		gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		gl(glEnable(GL_DEPTH_TEST));
		gl(glDisable(GL_BLEND));
	}

	SDL_GL_SwapWindow(data->window);

	return true;
	/* GameData *gameData = (GameData *)gameMemory; */
	/* const float camRotationSpeed = 2.0f; */
	/* const float camSpeed = 1.0f; */

	/* Camera c = gameData->camera; */
	/* if (kbState[KbKey::Left]) c.yaw -= camRotationSpeed * deltaTime; */
	/* if (kbState[KbKey::Right]) c.yaw += camRotationSpeed * deltaTime; */
	/* if (kbState[KbKey::Up]) c.pitch -= camRotationSpeed * deltaTime; */
	/* if (kbState[KbKey::Down]) c.pitch += camRotationSpeed * deltaTime; */

	/* c.pitch = clamp(c.pitch, -PI * 0.45f, PI * 0.45f); */
	/* c.yaw = (float)fmod(c.yaw, PI * 2); */

	/* mat4 camRotation = rotx_m4(c.yaw) * roty_m4(c.pitch); */
	/* if (kbState[KbKey::W]) */
	/* 	c.pos = c.pos -  vec4{0, 0, camSpeed * deltaTime, 0}; */
	/* if (kbState[KbKey::S]) */
	/* 	c.pos = c.pos +  vec4{0, 0, camSpeed * deltaTime, 0}; */

	/* mat4 camMat = fps_cam(c.pos, c.yaw, c.pitch); */
	/* gameData->camera = c; */

	/* float scale = 0.05f; */
	/* mat4 model = trans_m4(0, -5.0f, -5.0f) * scale_m4(scale, scale, scale); */
	/* mat4 view = camMat; */
	/* mat4 projection = projection(90.0f, float(renderTarget->texture->width) / */
	/*                                                    float(renderTarget->texture->height), */
	/*                                         0.1f, 1000.0f); */

	/* mat4 screenMatrix = */
	/*     screen_space(renderTarget->texture->width - 1, renderTarget->texture->height - 1); */

	/* b32 *kb = kbState; */
	/* b32 *lkb = gameData->lastKbState; */
	/* if (IsKeyDown(lkb, kb, KbKey::N_0)) */
	/* 	gameData->renderMode = RenderMode::Shaded | RenderMode::Textured; */
	/* if (IsKeyDown(lkb, kb, KbKey::N_1)) gameData->renderMode ^= RenderMode::Shaded; */
	/* if (IsKeyDown(lkb, kb, KbKey::N_2)) gameData->renderMode ^= RenderMode::Textured; */
	/* if (IsKeyDown(lkb, kb, KbKey::N_3)) gameData->renderMode ^= RenderMode::Wireframe; */

	/* ClearRenderTarget(renderTarget, {0, 0, 0, 255}); */
	/* Render(renderTarget, gameData->renderMode, &gameData->model, gameData->camera.pos, view * model, */
	/*        projection, screenMatrix, norm_v3(vec4{1, 1, 1, 0}), {255, 255, 255, 255}, 0.05f, */
	/*        &gameData->pool); */

	/* MemoryCopy(gameData->lastKbState, kbState, sizeof(*kbState) * KbKey::Last); */
	/* return true; */
}
