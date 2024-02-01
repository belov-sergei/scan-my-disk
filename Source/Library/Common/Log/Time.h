// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once

namespace Log::Time {
	// Sets the time function.
	void Set(std::function<std::string_view()> value);
	
	// Returns the time as a string.
	std::string_view Get();
}
