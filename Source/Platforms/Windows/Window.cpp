// Copyright ❤️ 2023-2024, Sergei Belov

#include "Window.h"

#include <dwmapi.h>
#include <imgui.h>
#include <windowsx.h>

namespace Detail {
	using Size = std::tuple<int, int>;
	using Point = std::tuple<int, int>;

	Size GetClientSize(HWND hWnd) {
		RECT result;
		GetClientRect(hWnd, &result);

		return { result.right, result.bottom };
	}

	Point GetCursor(HWND hWnd) {
		POINT result;

		GetCursorPos(&result);
		ScreenToClient(hWnd, &result);

		return { result.x, result.y };
	}
} // namespace Detail

bool CustomWindowTitleEnabled() {
	return true;
}

WNDPROC PreviousWindowProcedure = nullptr;

LRESULT CALLBACK CustomWindowProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_NCCALCSIZE: {
			if (wParam == TRUE) {
				return 0;
			}
		} break;
		case WM_NCHITTEST: {
			const auto [x, y] = Detail::GetCursor(hWnd);
			const auto [width, height] = Detail::GetClientSize(hWnd);

			if (x > 48 && x < width - 48 * 3 && y < 30) {
				return HTCAPTION;
			}

			if (x > width - 20 && y > height - 20) {
				return HTBOTTOMRIGHT;
			}

			if (x < width - 20 && y < height - 20) {
				return HTCLIENT;
			}
		} break;
		default:
			break;
	}

	return CallWindowProc(PreviousWindowProcedure, hWnd, msg, wParam, lParam);
}

void SetCustomWindowProcedure() {
	const auto hWnd = GetActiveWindow();

	PreviousWindowProcedure = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)CustomWindowProcedure);

	const MARGINS margins = { 0, 0, 0, 1 };
	DwmExtendFrameIntoClientArea(hWnd, &margins);

	const auto [width, height] = Detail::GetClientSize(hWnd);
	SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOMOVE);
}
