// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

namespace Image {
	void* Load(std::string_view path, int& width, int& height, int channels);
	void Free(void* bytes);
} // namespace Image
