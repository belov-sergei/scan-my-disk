// Copyright ❤️ 2023-2024, Sergei Belov

#include "Level.h"

namespace Log::Level {
	namespace Detail {
		uint8_t Level = 0;
	} // namespace Detail

	void Set(uint8_t value) {
		auto& level = Detail::Level;
		level = value;
	}

	uint8_t Get() {
		return Detail::Level;
	}
} // namespace Log::Level
