// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include <Windows.h>

int Start(int, char*[]);

inline int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	return Start(__argc, __argv);
}

#define main Start
