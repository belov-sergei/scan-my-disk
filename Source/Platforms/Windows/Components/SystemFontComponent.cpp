// Copyright ❤️ 2023-2024, Sergei Belov

#include "Components/SystemFontComponent.h"

#include <Windows.h>

FontCollection::BinaryFontDataType SystemFontComponent::GetBinaryFontData() {
	NONCLIENTMETRICSW nonClientMetrics;
	nonClientMetrics.cbSize = sizeof(NONCLIENTMETRICSW);

	SystemParametersInfoW(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICSW), &nonClientMetrics, 0);

	FontCollection::BinaryFontDataType binaryFontData;
	if (HDC hDC = CreateCompatibleDC(NULL)) {
		if (HFONT hFont = CreateFontIndirectW(&nonClientMetrics.lfMessageFont)) {
			SelectObject(hDC, hFont);

			DWORD dwSize = GetFontData(hDC, 0, 0, NULL, 0);
			if (dwSize != GDI_ERROR) {
				binaryFontData.resize(dwSize);

				GetFontData(hDC, 0, 0, binaryFontData.data(), dwSize);
			}

			DeleteObject(hFont);
		}

		DeleteDC(hDC);
	}

	return binaryFontData;
}
