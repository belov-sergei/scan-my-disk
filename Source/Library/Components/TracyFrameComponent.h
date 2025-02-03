// Copyright ❤️ 2023-2025, Sergei Belov

#pragma once
#include "Application.h"

#include <tracy/Tracy.hpp>

struct TracyFrameComponent final {
	TracyFrameComponent() {
		Event<Application::EndFrame>::Receive(this, [](const auto&) {
			FrameMark;
		});
	}
};
