// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

namespace Image {
	unsigned char* Load(std::string_view path, int& width, int& height, int channels);
	unsigned char* Load(const unsigned char* buffer, int length, int& width, int& height, int channels);
	void Free(void* bytes);
} // namespace Image
