// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Event.h"

struct Application {
	struct Loading {};

	struct Initialize {};

	struct Launch {};

	struct Update {
		float deltaTime = 0.0f;
	};

	struct LateUpdate {};

	struct Draw {};

	struct Terminate {};

	struct Exit {
		int exitCode = 0;
	};
};
