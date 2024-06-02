// Copyright ❤️ 2023-2024, Sergei Belov

#include "Time.h"
#include <fmt/chrono.h>

namespace Log::Time {
	namespace Detail {
		std::function<std::string_view()> Time = [result = std::string()]() mutable -> std::string_view {
			result.clear();
			fmt::format_to(std::back_inserter(result), "{:%y.%j-%T}", std::chrono::system_clock::now());

			return result;
		};
	} // namespace Detail

	void Set(std::function<std::string_view()> value) {
		auto& time = Detail::Time;
		time       = std::move(value);
	}

	std::string_view Get() {
		return Detail::Time();
	}
} // namespace Log::Time
