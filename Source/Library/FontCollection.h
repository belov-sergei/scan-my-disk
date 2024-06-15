// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Identifier.h"

struct FontCollection final {
	using DataType = FontCollection;
	using BinaryFontDataType = std::vector<uint8_t>;

	void CreateFont(Identifier fontId, BinaryFontDataType binaryFontData) {
		collection[fontId] = std::move(binaryFontData);
	}

	void DeleteFont(Identifier fontId) {
		collection.erase(fontId);
	}

	BinaryFontDataType& GetFont(Identifier fontId) {
		return collection[fontId];
	}

private:
	std::map<Identifier, BinaryFontDataType> collection;
};
