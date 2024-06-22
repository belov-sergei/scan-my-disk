// Copyright ❤️ 2023-2024, Sergei Belov

#include "Image.h"

#include "Warnings.h"

WARNINGS_IGNORE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
WARNINGS_NOTICE

unsigned char* Image::Load(std::string_view path, int& width, int& height, int channels) {
	return stbi_load(path.data(), &width, &height, nullptr, channels);
}

unsigned char* Image::Load(const unsigned char* buffer, int length, int& width, int& height, int channels) {
	return stbi_load_from_memory(buffer, length, &width, &height, nullptr, channels);
}

void Image::Free(void* bytes) {
	stbi_image_free(bytes);
}
