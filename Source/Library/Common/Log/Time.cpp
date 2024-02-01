// Copyright ❤️ 2023-2024, Sergei Belov

#include "Time.h"

namespace Log::Time {
	namespace Detail {
		std::function<std::string_view()> Time = [result = std::string(16, 0)]() mutable {
			const std::time_t now = std::time(nullptr);
			std::ignore = std::strftime(result.data(), result.size(), "%y.%j-%T", std::gmtime(&now));

			return result;
		};
	}

	void Set(std::function<std::string_view()> value) {
		auto& time = Detail::Time;
		time = std::move(value);
	}

	std::string_view Get() {
		return Detail::Time();
	}
}
