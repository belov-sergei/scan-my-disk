// Copyright ❤️ 2023-2024, Sergei Belov

#include "Window.h"

bool CustomWindowTitleEnabled() {
	return false;
}

void SetWindowProc(int w, int h) {
	std::ignore = w;
	std::ignore = h;
}
