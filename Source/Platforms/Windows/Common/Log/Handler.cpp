// Copyright ❤️ 2023-2025, Sergei Belov

#include "Common/Log/Handler.h"

#include <Windows.h>

namespace Log::Handler {
	void WindowsDebugOutput::operator()(std::string_view value) const {
		OutputDebugString(value.data());
	}
} // namespace Log::Handler
