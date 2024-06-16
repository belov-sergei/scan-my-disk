// Copyright ❤️ 2023-2024, Sergei Belov

#include "Common/Encoding.h"

#include <Windows.h>

namespace Encoding {
	std::wstring MultiByteToWideChar(std::string_view value) {
		const int bufferSize = ::MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0);

		std::wstring result(bufferSize, 0);
		::MultiByteToWideChar(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), static_cast<int>(result.size()));

		return result;
	}

	std::string WideCharToMultiByte(std::wstring_view value) {
		const int bufferSize = ::WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);

		std::string result(bufferSize, 0);
		::WideCharToMultiByte(CP_UTF8, 0, value.data(), static_cast<int>(value.size()), result.data(), static_cast<int>(result.size()), nullptr, nullptr);

		return result;
	}
} // namespace Encoding
