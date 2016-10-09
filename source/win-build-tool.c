#define _CRT_SECURE_NO_WARNINGS

#include <stdbool.h>
#include <stdio.h>
#include <conio.h>
#include <windows.h>
#include <assert.h>

#define STB_DEFINE
#pragma warning(push, 1)
#pragma warning(disable : 4311)
#pragma warning(disable : 4312)
#pragma warning(disable : 4701)
#pragma warning(disable : 4047)
#pragma warning(disable : 4024)
#include "stb.h"
#pragma warning(pop)

enum {
	FG_RED = FOREGROUND_RED,
	FG_GREEN = FOREGROUND_GREEN,
	FG_BLUE = FOREGROUND_BLUE,
	FG_YELLOW = FOREGROUND_RED | FOREGROUND_GREEN,
	FG_PURPLE = FOREGROUND_RED | FOREGROUND_BLUE,
	FG_TEAL = FOREGROUND_GREEN | FOREGROUND_BLUE,
	FG_WHITE = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,
	FG_BOLD = FOREGROUND_INTENSITY
};

void do_build(const char *build_command, HANDLE con_handle, const char **ignore_patterns,
              int ignore_patterns_count);
int execute_command(const char *build_command, HANDLE con_handle, const char **ignore_patterns,
                    int ignore_patterns_count);
char *strip_tag(char *str, bool *out_tag_stripped);
void cls(HANDLE hConsole);

int main(int argc, char **argv) {
	const char *ignore_patterns[] = {
	    "   Creating library .\\build\\game.lib and object .\\build\\game.exp\n", "platform.c\n",
	    "game.c\n"};

	HANDLE con_handle = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_SCREEN_BUFFER_INFO con_info;

	int conf_script_result =
	    execute_command("..\\source\\configure-msvc.bat 2>&1", con_handle, ignore_patterns,
	                    sizeof(ignore_patterns) / sizeof(ignore_patterns[0]));
	assert(conf_script_result == 0);
	FILE *env_vars = fopen("env.txt", "r");
	assert(env_vars);
	{
		char buffer[1024 * 10]; // because PATH can be fucking huge
		while (fgets(buffer, sizeof(buffer), env_vars)) {
			char *var_value = buffer;
			do { ++var_value; } while (*var_value != '=');
			*var_value = 0;
			++var_value;

			size_t n = strlen(var_value);
			assert(var_value[n - 1] == '\n');
			var_value[n - 1] = 0;

			SetEnvironmentVariable(buffer, var_value);
		}
	}

	int user_choice;
	do {
		SetConsoleTextAttribute(con_handle, FG_WHITE | FG_BOLD);
		printf("{ [b]uild, [a]ndroid build, [c]lear, [q]uit } $> ");
		user_choice = _getch();
		printf("\n");

		switch (user_choice) {
		case 'b': {
			do_build("..\\source\\build.bat 2>&1", con_handle, ignore_patterns,
			         sizeof(ignore_patterns) / sizeof(ignore_patterns[0]));
		} break;
		case 'a': {
			do_build("..\\source\\build-android.bat 2>&1", con_handle, ignore_patterns,
			         sizeof(ignore_patterns) / sizeof(ignore_patterns[0]));
		} break;
		case 'c': {
			cls(con_handle);
		} break;
		}
	} while (user_choice != 'q');

	SetConsoleTextAttribute(con_handle, FG_WHITE);
	return 0;
}

void do_build(const char *build_command, HANDLE con_handle, const char **ignore_patterns,
              int ignore_patterns_count) {
	SetConsoleTextAttribute(con_handle, FG_PURPLE);
	printf("[BUILD STARTED]\n");

	int result = execute_command(build_command, con_handle, ignore_patterns, ignore_patterns_count);

	if (result == 0) {
		SetConsoleTextAttribute(con_handle, FG_GREEN);
		printf("[BUILD SUCCEDED]\n");
	} else {
		SetConsoleTextAttribute(con_handle, FG_PURPLE);
		printf("[BUILD FAILED]\n");
	}
}

int execute_command(const char *build_command, HANDLE con_handle, const char **ignore_patterns,
                    int ignore_patterns_count) {
	FILE *build_fp = _popen(build_command, "r");
	assert(build_fp);

	char str_buff[1024];
	while (fgets(str_buff, sizeof(str_buff), build_fp)) {
		bool ignore_patterns_match = false;
		for (int i = 0; i < ignore_patterns_count; ++i) {
			if (strcmp(str_buff, ignore_patterns[i]) == 0) {
				ignore_patterns_match = true;
				break;
			}
		}

		if (ignore_patterns_match) continue;

		char str_buff_lc[sizeof(str_buff)];
		strcpy(str_buff_lc, str_buff);
		stb_tolower(str_buff_lc);
		unsigned int color_mask;
		if (strstr(str_buff_lc, "error"))
			color_mask = FG_RED;
		else if (strstr(str_buff_lc, "warning"))
			color_mask = FG_YELLOW;
		else
			color_mask = FG_WHITE;

		bool tag_stripped;
		char *output_str = strip_tag(str_buff, &tag_stripped);
		SetConsoleTextAttribute(con_handle, color_mask | (tag_stripped ? FG_BOLD : 0));
		printf("%s", output_str);
	}

	return _pclose(build_fp);
}

char *strip_tag(char *str, bool *out_tag_stripped) {
	char *res = str;
	*out_tag_stripped = false;

	if (*res == '[') {
		*out_tag_stripped = true;

		do { ++res; } while (*res != ']' && *res != 0);

		if (*res != 0) ++res;

		while (*res == ' ') ++res;
	}

	return res;
}

// lifted from https://msdn.microsoft.com/en-us/library/windows/desktop/ms682022(v=vs.85).aspx
void cls(HANDLE hConsole) {
	COORD coordScreen = {0, 0}; // home for the cursor
	DWORD cCharsWritten;
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	DWORD dwConSize;

	// Get the number of character cells in the current buffer.
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) { return; }

	dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

	// Fill the entire screen with blanks.
	if (!FillConsoleOutputCharacter(hConsole,        // Handle to console screen buffer
	                                (TCHAR)' ',      // Character to write to the buffer
	                                dwConSize,       // Number of cells to write
	                                coordScreen,     // Coordinates of first cell
	                                &cCharsWritten)) // Receive number of characters written
	{
		return;
	}

	// Get the current text attribute.
	if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) { return; }

	// Set the buffer's attributes accordingly.
	if (!FillConsoleOutputAttribute(hConsole,         // Handle to console screen buffer
	                                csbi.wAttributes, // Character attributes to use
	                                dwConSize,        // Number of cells to set attribute
	                                coordScreen,      // Coordinates of first cell
	                                &cCharsWritten))  // Receive number of characters written
	{
		return;
	}

	// Put the cursor at its home coordinates.
	SetConsoleCursorPosition(hConsole, coordScreen);
}
