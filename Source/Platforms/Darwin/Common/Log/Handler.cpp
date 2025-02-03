// Copyright ❤️ 2023-2025, Sergei Belov

#include "Common/Log/Handler.h"

namespace Log::Handler {
	void WindowsDebugOutput::operator()(std::string_view value) const {
		std::ignore = value;
	}
}
