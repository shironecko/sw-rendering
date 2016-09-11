#define _HAS_EXCEPTIONS 0
#define _STATIC_CPPLIB

#include "platform_api.h"
#include "utility.cpp"
#include "game.cpp"

#include <windows.h>

void PlatformAssert(usize condition) {
	if (!condition) DebugBreak();
}

u64 PlatformGetFileSize(const char *path) {
	HANDLE fileHandle = CreateFile(path, FILE_READ_ATTRIBUTES, FILE_SHARE_READ, nullptr,
	                               OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);

	if (fileHandle == INVALID_HANDLE_VALUE) {
		OutputDebugStringA("Was not able to open a file!\n");
		return 0;
	}

	LARGE_INTEGER size;
	BOOL getFileSizeResult = GetFileSizeEx(fileHandle, &size);

	if (!getFileSizeResult) {
		OutputDebugStringA("Was not able to get a file size!\n");
		return 0;
	}

	CloseHandle(fileHandle);
	return u64(size.QuadPart);
}

u32 PlatformLoadFile(const char *path, void *memory, u32 memorySize) {
	HANDLE fileHandle = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
	                               FILE_ATTRIBUTE_NORMAL, 0);

	if (fileHandle == INVALID_HANDLE_VALUE) {
		OutputDebugStringA("Was not able to open a file!\n");
		return 0;
	}

	DWORD bytesRead;
	BOOL readResult = ReadFile(fileHandle, memory, memorySize, &bytesRead, nullptr);

	if (!readResult) { OutputDebugStringA("Was not able to read from a file!\n"); }

	CloseHandle(fileHandle);
	return bytesRead;
}

b32 PlatformWriteFile(const char *path, void *memory, u32 bytesToWrite) {
	// TODO: handle directory creation

	HANDLE fileHandle =
	    CreateFile(path, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	if (fileHandle == INVALID_HANDLE_VALUE) {
		OutputDebugStringA("Was not able to create or owerwrite a file!\n");
		return false;
	}

	DWORD bytesWritten;
	BOOL writeResult = WriteFile(fileHandle, memory, bytesToWrite, &bytesWritten, nullptr);

	if (!writeResult) {
		OutputDebugStringA("Was not able to write to a file!\n");
		CloseHandle(fileHandle);
		return false;
	}

	if (bytesToWrite != bytesWritten) {
		OutputDebugStringA("Bytes to write was not equal to bytes written!\n");
		CloseHandle(fileHandle);
		return false;
	}

	CloseHandle(fileHandle);
	return true;
}

struct Win32BackBuffer {
	BITMAPINFO info;
	u32 *memory;
};

struct {
	b32 shouldRun;
	u32 windowWidth;
	u32 windowHeight;

	void *memory;
	u32 memorySize;
	RenderTarget renderBuffer;
	Win32BackBuffer backBuffer;

} g_platformData{true, 640, 480, nullptr, 0, {}, {}};

void Win32SetupRenderingBuffers(u32 width, u32 height) {
	u8 *platformMemory = (u8 *)g_platformData.memory;

	BITMAPINFO info{};
	info.bmiHeader.biSize = sizeof(info.bmiHeader);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = BI_RGB;

	g_platformData.backBuffer.info = info;
	g_platformData.backBuffer.memory = (u32 *)platformMemory;
	platformMemory += width * height * sizeof(u32);

	Texture **texture = &g_platformData.renderBuffer.texture;
	*texture = (Texture *)platformMemory;
	(*texture)->width = width;
	(*texture)->height = height;
	(*texture)->texels = (Color32 *)(platformMemory + sizeof(Texture));
	platformMemory += width * height * sizeof(Color32) + sizeof(Texture);

	g_platformData.renderBuffer.zBuffer = (float *)platformMemory;
	platformMemory += width * height * sizeof(float);

	assert(u32(platformMemory - (u8 *)g_platformData.memory) <= g_platformData.memorySize);
}

void Win32PresentToWindow(HDC windowDC, u32 windowWidth, u32 windowHeight,
                          Win32BackBuffer *backBuffer, RenderTarget *renderBuffer) {
	u32 bufferWidth = renderBuffer->texture->width;
	u32 bufferHeight = renderBuffer->texture->height;

	// TODO: think about just allocating back-buffer here, on the stack
	for (u32 y = 0; y < bufferHeight; ++y) {
		for (u32 x = 0; x < bufferWidth; ++x) {
			Color32 bufferColor = renderBuffer->texture->GetTexel(x, y);

			backBuffer->memory[y * bufferWidth + x] =
			    u32(bufferColor.b) | u32(bufferColor.g) << 8 | u32(bufferColor.r) << 16;
		}
	}

	StretchDIBits(windowDC, 0, 0, windowWidth, windowHeight, 0, 0,
	              backBuffer->info.bmiHeader.biWidth, backBuffer->info.bmiHeader.biHeight,
	              backBuffer->memory, &backBuffer->info, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_SIZE: {
		g_platformData.windowWidth = LOWORD(lParam);
		g_platformData.windowHeight = HIWORD(lParam);
	} break;
	case WM_EXITSIZEMOVE: {
		// this is basically free with manual memory management
		Win32SetupRenderingBuffers(g_platformData.windowWidth, g_platformData.windowHeight);
	} break;
	case WM_PAINT: {
		PAINTSTRUCT ps;
		HDC windowDC = BeginPaint(window, &ps);

		Win32PresentToWindow(windowDC, g_platformData.windowWidth, g_platformData.windowHeight,
		                     &g_platformData.backBuffer, &g_platformData.renderBuffer);

		EndPaint(window, &ps);
	} break;
	case WM_CLOSE:
	case WM_DESTROY: {
		g_platformData.shouldRun = false;
	} break;
	default: { return DefWindowProc(window, message, wParam, lParam); } break;
	}

	return 0;
}

global const u32 g_keyMapSize = 0xA5 + 1;
global u32 g_keyMap[g_keyMapSize]{};

int CALLBACK WinMain(HINSTANCE instance, HINSTANCE /* prevInstance */, char * /* cmdLine */,
                     int /* cmdShow */) {
	// damn vs2013 can't do array initialization proper
	{
		g_keyMap[VK_BACK] = KbKey::Backspace;
		g_keyMap[VK_TAB] = KbKey::Tab;
		g_keyMap[VK_RETURN] = KbKey::Return;
		g_keyMap[VK_ESCAPE] = KbKey::Escape;
		g_keyMap[VK_SPACE] = KbKey::Space;
		g_keyMap[VK_END] = KbKey::End;
		g_keyMap[VK_HOME] = KbKey::Home;
		g_keyMap[VK_LEFT] = KbKey::Left;
		g_keyMap[VK_UP] = KbKey::Up;
		g_keyMap[VK_RIGHT] = KbKey::Right;
		g_keyMap[VK_DOWN] = KbKey::Down;
		g_keyMap[VK_DELETE] = KbKey::Delete;
		g_keyMap[0x30] = KbKey::N_0;
		g_keyMap[0x31] = KbKey::N_1;
		g_keyMap[0x32] = KbKey::N_2;
		g_keyMap[0x33] = KbKey::N_3;
		g_keyMap[0x34] = KbKey::N_4;
		g_keyMap[0x35] = KbKey::N_5;
		g_keyMap[0x36] = KbKey::N_6;
		g_keyMap[0x37] = KbKey::N_7;
		g_keyMap[0x38] = KbKey::N_8;
		g_keyMap[0x39] = KbKey::N_9;
		g_keyMap[0x41] = KbKey::A;
		g_keyMap[0x42] = KbKey::B;
		g_keyMap[0x43] = KbKey::C;
		g_keyMap[0x44] = KbKey::D;
		g_keyMap[0x45] = KbKey::E;
		g_keyMap[0x46] = KbKey::F;
		g_keyMap[0x47] = KbKey::G;
		g_keyMap[0x48] = KbKey::H;
		g_keyMap[0x49] = KbKey::I;
		g_keyMap[0x4A] = KbKey::J;
		g_keyMap[0x4B] = KbKey::K;
		g_keyMap[0x4C] = KbKey::L;
		g_keyMap[0x4D] = KbKey::M;
		g_keyMap[0x4E] = KbKey::N;
		g_keyMap[0x4F] = KbKey::O;
		g_keyMap[0x50] = KbKey::P;
		g_keyMap[0x51] = KbKey::Q;
		g_keyMap[0x52] = KbKey::R;
		g_keyMap[0x53] = KbKey::S;
		g_keyMap[0x54] = KbKey::T;
		g_keyMap[0x55] = KbKey::U;
		g_keyMap[0x56] = KbKey::V;
		g_keyMap[0x57] = KbKey::W;
		g_keyMap[0x58] = KbKey::X;
		g_keyMap[0x59] = KbKey::Y;
		g_keyMap[0x5A] = KbKey::Z;
		g_keyMap[VK_F1] = KbKey::F1;
		g_keyMap[VK_F2] = KbKey::F2;
		g_keyMap[VK_F3] = KbKey::F3;
		g_keyMap[VK_F4] = KbKey::F4;
		g_keyMap[VK_F5] = KbKey::F5;
		g_keyMap[VK_F6] = KbKey::F6;
		g_keyMap[VK_F7] = KbKey::F7;
		g_keyMap[VK_F8] = KbKey::F8;
		g_keyMap[VK_F9] = KbKey::F9;
		g_keyMap[VK_F10] = KbKey::F10;
		g_keyMap[VK_F11] = KbKey::F11;
		g_keyMap[VK_F12] = KbKey::F12;
		g_keyMap[VK_LSHIFT] = KbKey::ShiftL;
		g_keyMap[VK_RSHIFT] = KbKey::ShiftR;
		g_keyMap[VK_LCONTROL] = KbKey::ControlL;
		g_keyMap[VK_RCONTROL] = KbKey::ControlR;
		g_keyMap[VK_LMENU] = KbKey::AltL;
		g_keyMap[VK_RMENU] = KbKey::AltR; // 0xA5 Right MENU key
	}

	//*****CREATING A WINDOW*****//
	WNDCLASSEX wndClass{};
	wndClass.cbSize = sizeof(wndClass);
	wndClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndClass.lpfnWndProc = Win32WindowProc;
	wndClass.hInstance = instance;
	wndClass.lpszClassName = "Software Renderer Window Class Name";
	RegisterClassEx(&wndClass);

	HWND window = CreateWindowEx(0, wndClass.lpszClassName, "Software Renderer",
	                             WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
	                             g_platformData.windowWidth, g_platformData.windowHeight, 0, 0,
	                             instance, nullptr);

	//*****ALLOCATING MEMORY*****//
	g_platformData.memorySize = 128 * Mb;
	g_platformData.memory =
	    VirtualAlloc(nullptr, g_platformData.memorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	assert(g_platformData.memory);

	u32 gameMemorySize = 512 * Mb;
	void *gameMemory =
	    VirtualAlloc(nullptr, gameMemorySize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	assert(gameMemory);

	//*****MISC SETUP*****//
	Win32SetupRenderingBuffers(g_platformData.windowWidth, g_platformData.windowHeight);

	HDC windowDC = GetDC(window);
	MSG message{};
	b32 kbState[KbKey::LastKey]{};

	LARGE_INTEGER lastFrameTime;
	LARGE_INTEGER queryFrequency;
	QueryPerformanceCounter(&lastFrameTime);
	QueryPerformanceFrequency(&queryFrequency);

	GameInitialize(gameMemory, gameMemorySize);

	//*****RENDERING LOOP*****//
	while (g_platformData.shouldRun) {
		LARGE_INTEGER currentFrameTime;
		QueryPerformanceCounter(&currentFrameTime);
		u64 ticksElapsed = currentFrameTime.QuadPart - lastFrameTime.QuadPart;
		float deltaTime = float(ticksElapsed) / float(queryFrequency.QuadPart);
		lastFrameTime = currentFrameTime;

		// TODO: sort this out
		char windowTitle[256];
		wsprintf(windowTitle, "Software Renderer \t %ums per frame", u32(deltaTime * 1000.0f));
		SetWindowText(window, windowTitle);

		while (PeekMessage(&message, window, 0, 0, PM_REMOVE)) {
			TranslateMessage(&message);
			if (message.message == WM_KEYDOWN || message.message == WM_KEYUP) {
				u32 keyMapIdx = message.wParam;
				if (keyMapIdx < g_keyMapSize) {
					u32 kbStateIdx = g_keyMap[keyMapIdx];
					assert(kbStateIdx < KbKey::LastKey);

					kbState[kbStateIdx] = message.message == WM_KEYDOWN;
				}
			}

			DispatchMessage(&message);
		}

		b32 gameWantsToContinue = GameUpdate(deltaTime, gameMemory, gameMemorySize,
		                                      &g_platformData.renderBuffer, kbState);
		g_platformData.shouldRun &= gameWantsToContinue;
		Win32PresentToWindow(windowDC, g_platformData.windowWidth, g_platformData.windowHeight,
		                     &g_platformData.backBuffer, &g_platformData.renderBuffer);
	}

	return 0;
}
