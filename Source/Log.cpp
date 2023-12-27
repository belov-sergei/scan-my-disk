// Copyright ❤️ 2023-2024, Sergei Belov

#include <Log.h>

namespace Log {
	void SetLevel(int level) {
		Details::Level = level;
	}

	void Reset() {
		Details::Level = 0;
		Details::Loggers = {};
	}
} // namespace Log
