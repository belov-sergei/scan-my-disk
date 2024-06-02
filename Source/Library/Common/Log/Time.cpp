// Copyright ❤️ 2023-2024, Sergei Belov

#include "Time.h"

namespace Log::Time {
	namespace Detail {
		std::function<std::string_view()> Time = [result = std::string(16, 0)]() mutable {
			const std::time_t now = std::time(nullptr);

			tm tm;
			std::ignore = gmtime_s(&tm, &now);
			std::ignore = std::strftime(result.data(), result.size(), "%y.%j-%T", &tm);

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
