// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

namespace Encoding {
	// Converts a multi-byte string to a wide character string.
	std::wstring MultiByteToWideChar(std::string_view value);

	// Converts a wide character string to a multibyte string using UTF-8 encoding.
	std::string WideCharToMultiByte(std::wstring_view value);
} // namespace Encoding
