#ifdef GL_RIGOROUS_CHECKS
#define gl(expression) gl_assert(gl_fns.expression)
#define gl_set(assignee, expression) gl_set_assert(assignee, gl_fns.expression)
#else
#define gl(expression)                                                                             \
	do { (gl_fns.expression); } while (0)
#define gl_set(assignee, expression)                                                               \
	do { (assignee) = (gl_fns.expression); } while (0)
#endif

#define gl_check_error(expression)                                                                 \
	GLenum gl_error = gl_fns.glGetError();                                                         \
	if (gl_error != GL_NO_ERROR) {                                                                 \
		char gl_error_str[64];                                                                     \
		get_gl_error_str(gl_error, gl_error_str, sizeof(gl_error_str));                            \
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO,                                                    \
		                "OpenGL call " #expression " failed with error %s(0x%x)! %s:%d",           \
		                gl_error_str, gl_error, __FILE__, __LINE__);                               \
		SDL_TriggerBreakpoint();                                                                   \
		exit(1);                                                                                   \
	}

#define gl_assert(expression)                                                                      \
	do {                                                                                           \
		(expression);                                                                              \
		gl_check_error(expression);                                                                \
	} while (0)

#define gl_set_assert(assignee, expression)                                                        \
	do {                                                                                           \
		(assignee) = (expression);                                                                 \
		gl_check_error(expression);                                                                \
	} while (0)

LOCAL void get_gl_error_str(GLenum error_code, char *buffer, u32 buffer_size) {
#ifndef GL_STACK_OVERFLOW
#define GL_STACK_OVERFLOW 0x0503
#endif

#ifndef GL_STACK_UNDERFLOW
#define GL_STACK_UNDERFLOW 0x0504
#endif

#ifndef GL_CONTEXT_LOST
#define GL_CONTEXT_LOST 0x0507
#endif

#ifndef GL_TABLE_TOO_LARGE
#define GL_TABLE_TOO_LARGE 0x8031
#endif

	struct {
		GLenum code;
		const char *string;
	} error_strings[] = {{GL_INVALID_ENUM, "GL_INVALID_ENUM"},
	                     {GL_INVALID_VALUE, "GL_INVALID_VALUE"},
	                     {GL_INVALID_OPERATION, "GL_INVALID_OPERATION"},
	                     {GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW"},
	                     {GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW"},
	                     {GL_OUT_OF_MEMORY, "GL_OUT_OF_MEMORY"},
	                     {GL_INVALID_FRAMEBUFFER_OPERATION, "GL_INVALID_FRAMEBUFFER_OPERATION"},
	                     {GL_CONTEXT_LOST, "GL_CONTEXT_LOST"},
	                     {GL_TABLE_TOO_LARGE, "GL_TABLE_TOO_LARGE"}};

	s32 error_string_idx = -1;
	for (u32 i = 0; i < sizeof(error_strings) / sizeof(error_strings[0]); ++i) {
		if (error_strings[i].code == error_code) {
			error_string_idx = i;
			break;
		}
	}

	if (error_string_idx != -1)
		stb_strncpy(buffer, error_strings[error_string_idx].string, buffer_size);
	else
		stb_snprintf(buffer, buffer_size, "0x%x", error_code);
}

LOCAL b32 compile_shader(gl_functions gl_fns, GLuint *out_shader, const char *src,
                          GLenum shader_type, char *error_buff, u32 error_buff_size) {
	GLuint shader;
	gl_set(shader, glCreateShader(shader_type));
	gl(glShaderSource(shader, 1, &src, 0));
	gl(glCompileShader(shader));

	GLint status;
	gl(glGetShaderiv(shader, GL_COMPILE_STATUS, &status));
	if (status != GL_TRUE) {
		GLsizei log_length;
		gl(glGetShaderInfoLog(shader, error_buff_size, &log_length, error_buff));
		error_buff[log_length] = '\0';
		gl(glDeleteShader(shader));
		return false;
	}

	*out_shader = shader;
	return true;
}

#define compile_shader_program(gl_fns, out_program, vert_src, frag_src)                            \
	compile_shader_program_fn(gl_fns, out_program, vert_src, frag_src, __FILE__, __LINE__)

LOCAL b32 compile_shader_program_fn(gl_functions gl_fns, GLuint *out_program,
                                     const char *vert_src, const char *frag_src, const char *file,
                                     u32 line) {
	GLuint vertex_shader;
	{
		char error_buff[1024];
		b32 compile_res = compile_shader(gl_fns, &vertex_shader, vert_src, GL_VERTEX_SHADER,
		                                  error_buff, sizeof(error_buff));
		if (!compile_res) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "[VERTEX SHADER COMPILATION] %s %s:%u",
			                error_buff, file, line);
			return false;
		}
	}

	GLuint fragment_shader;
	{
		char error_buff[1024];
		b32 compile_res = compile_shader(gl_fns, &fragment_shader, frag_src, GL_FRAGMENT_SHADER,
		                                  error_buff, sizeof(error_buff));
		if (!compile_res) {
			SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "[FRAGMENT SHADER COMPILATION] %s %s:%d",
			                error_buff, file, line);
			gl(glDeleteShader(vertex_shader));
			return false;
		}
	}

	GLuint program;
	gl_set(program, glCreateProgram());
	gl(glAttachShader(program, vertex_shader));
	gl(glAttachShader(program, fragment_shader));
	gl(glLinkProgram(program));

	gl(glDetachShader(program, vertex_shader));
	gl(glDeleteShader(vertex_shader));
	gl(glDetachShader(program, fragment_shader));
	gl(glDeleteShader(fragment_shader));

	GLint status;
	gl(glGetProgramiv(program, GL_LINK_STATUS, &status));
	if (status != GL_TRUE) {
		GLsizei log_length;
		char error_buff[1024];
		gl(glGetProgramInfoLog(program, sizeof(error_buff), &log_length, error_buff));
		error_buff[log_length] = '\0';
		gl(glDeleteProgram(program));
		SDL_LogCritical(SDL_LOG_CATEGORY_VIDEO, "[SHADER PROGRAM LINKAGE] %s %s:%d", error_buff,
		                file, line);
		return false;
	}

	*out_program = program;
	return true;
}
