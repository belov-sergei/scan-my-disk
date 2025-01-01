// Copyright ❤️ 2023-2024, Sergei Belov

#include "Image.h"

#include "fmt/core.h"
#include "png.h"
#include "Warnings.h"

WARNINGS_IGNORE
WARNINGS_NOTICE

static std::vector<unsigned char*> images;

unsigned char* Image::Load(std::string_view path, int& width, int& height, int channels) {
	std::ignore = channels;

	png_image image;
	memset(&image, 0, sizeof(image));

	image.version = PNG_IMAGE_VERSION;

	if (!png_image_begin_read_from_file(&image, path.data())) {
		return nullptr;
	}

	unsigned char* buffer = images.emplace_back(new unsigned char[PNG_IMAGE_SIZE(image)]);
	if (!buffer) {
		png_image_free(&image);
		auto it = std::find(images.begin(), images.end(), buffer);
		if (it != images.end()) {
			images.erase(it);
		}

		return nullptr;
	}

	if (!png_image_finish_read(&image, 0, buffer, 0, nullptr)) {
		png_image_free(&image);
		auto it = std::find(images.begin(), images.end(), buffer);
		if (it != images.end()) {
			images.erase(it);
		}
		return nullptr;
	}

	width = image.width;
	height = image.height;

	png_image_free(&image);
	return buffer;
}

unsigned char* Image::Load(const unsigned char* buffer, int length, int& width, int& height, int channels) {
	std::ignore = channels;

	png_image image;
	memset(&image, 0, sizeof(image));

	image.version = PNG_IMAGE_VERSION;

	if (!png_image_begin_read_from_memory(&image, buffer, length)) {
		return nullptr;
	}

	unsigned char* mem = images.emplace_back(new unsigned char[PNG_IMAGE_SIZE(image)]);
	if (!mem) {
		png_image_free(&image);
		auto it = std::find(images.begin(), images.end(), mem);
		if (it != images.end()) {
			images.erase(it);
		}

		return nullptr;
	}

	if (!png_image_finish_read(&image, 0, mem, 0, nullptr)) {
		png_image_free(&image);
		auto it = std::find(images.begin(), images.end(), mem);
		if (it != images.end()) {
			images.erase(it);
		}
		return nullptr;
	}

	width = image.width;
	height = image.height;

	png_image_free(&image);
	return mem;
}

void Image::Write(std::string_view path, const unsigned char* buffer, int width, int height, int channels, int stride) {
	png_image image;
	memset(&image, 0, sizeof(image));

	image.version = PNG_IMAGE_VERSION;
	image.width = width;
	image.height = height;
	image.flags = 0;
	image.colormap_entries = 0;

	switch (channels) {
		case 3:
			image.format = PNG_FORMAT_RGB;
			break;
		case 4:
			image.format = PNG_FORMAT_RGBA;
			break;
		default:
			image.format = PNG_FORMAT_GRAY;
	}

	png_image_write_to_file(&image, path.data(), 0, buffer, stride, nullptr);
	png_image_free(&image);
}

unsigned char* Image::Write(const unsigned char* buffer, int width, int height, int channels, int stride, size_t* length) {
	png_image image;
	memset(&image, 0, sizeof(image));

	image.version = PNG_IMAGE_VERSION;
	image.width = width;
	image.height = height;
	image.flags = 0;
	image.colormap_entries = 0;

	switch (channels) {
		case 3:
			image.format = PNG_FORMAT_RGB;
			break;
		case 4:
			image.format = PNG_FORMAT_RGBA;
			break;
		default:
			image.format = PNG_FORMAT_GRAY;
	}

	unsigned char* out = new unsigned char[PNG_IMAGE_SIZE(image)];
	*length = PNG_IMAGE_SIZE(image);

	png_image_write_to_memory(&image, out, length, 0, buffer, stride, nullptr);
	png_image_free(&image);

	return out;
}

void Image::Free(unsigned char* bytes) {
	auto it = std::find(images.begin(), images.end(), bytes);
	if (it != images.end()) {
		delete[] bytes;
		images.erase(it);
	}
}
