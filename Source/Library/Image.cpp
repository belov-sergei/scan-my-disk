// Copyright ❤️ 2023-2024, Sergei Belov

#include "Image.h"

#include "Warnings.h"

WARNINGS_IGNORE
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
WARNINGS_NOTICE

void* Image::Load(std::string_view path, int& width, int& height, int channels) {
	return stbi_load(path.data(), &width, &height, nullptr, channels);
}

void Image::Free(void* bytes) {
	stbi_image_free(bytes);
}
