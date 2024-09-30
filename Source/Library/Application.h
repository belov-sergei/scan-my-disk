// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Event.h"

struct Application {
	using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	struct Loading {};

	struct Initialize {};

	struct Launch {};

	struct Update {
		float deltaTime = 0.0f;
	};

	struct LateUpdate {};

	struct Draw {};

	struct StartFrame {
		TimePoint eventTime = std::chrono::steady_clock::now();
	};

	struct EndFrame {
		TimePoint eventTime = std::chrono::steady_clock::now();
	};

	struct Terminate {};

	struct Exit {
		int exitCode = 0;
	};
};
