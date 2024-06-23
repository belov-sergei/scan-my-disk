// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "MemoryRegionIterator.h"

#include <imgui_internal.h>

struct IMGUIFontComponent final {
	using BaseImageIterator = MemoryRegionIterator<unsigned char>;
	using TargetImageIterator = MemoryRegionIterator<ImU32>;

	inline static ImFont* DefaultFont = nullptr;
	inline static ImFont* SystemFont = nullptr;

	struct ImageData {
		int stride = 0;
		unsigned char* data = nullptr;
	};

	struct Glyph {
		int index;
		ImWchar codepoint;
		int x, y;
		int width, height;
		float advanceX;
		ImVec2 offset;
	};

	IMGUIFontComponent();

private:
	// Generated array of glyph data for the font.
	static Glyph Glyphs[];

	// Generated array of pixel data for the font texture.
	static unsigned char TextureData[];
};
