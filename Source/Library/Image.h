// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

namespace Image {
	unsigned char* Load(std::string_view path, int& width, int& height, int channels);
	unsigned char* Load(const unsigned char* buffer, int length, int& width, int& height, int channels);
	void Write(std::string_view path, const unsigned char* buffer, int width, int height, int channels, int stride);
	unsigned char* Write(const unsigned char* buffer, int width, int height, int channels, int stride, size_t* length);

	void Free(void* bytes);
} // namespace Image
