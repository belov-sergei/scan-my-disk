// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"

struct FrameRateComponent final {
	FrameRateComponent() {
		Event<Application::Update>::Receive(this, [this](const Application::Update& event) {
			const auto delay = std::chrono::duration<float>(1.0f / 24 - event.deltaTime);
			std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(delay));
		});
	}
};
