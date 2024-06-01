// Copyright ❤️ 2023-2024, Sergei Belov

#include "Window.h"

#include <dwmapi.h>

bool CustomWindowTitleEnabled() {
	return true;
}

WNDPROC SDLWndProc;

LRESULT CALLBACK MyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_NCCALCSIZE:
			if (wParam == TRUE) {
				return 0;
			}
			break;
	}

	return CallWindowProc(SDLWndProc, hwnd, msg, wParam, lParam);
}

void SetWindowProc(int w, int h) {
	MARGINS margins = { -1 };
	DwmExtendFrameIntoClientArea(GetActiveWindow(), &margins);

	SDLWndProc = (WNDPROC)SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, (LONG_PTR)MyWndProc);
	SetWindowPos(GetActiveWindow(), NULL, 0, 0, w, h, SWP_FRAMECHANGED | SWP_NOMOVE);
}
