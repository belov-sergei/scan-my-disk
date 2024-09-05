// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"

struct FrameRateComponent final {
	using Seconds = std::chrono::duration<float>;
	using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	FrameRateComponent() {
		Event<Application::StartFrame>::Receive(this, [this](const Application::StartFrame& startFrame) {
			startFrameTime = startFrame.eventTime;
		});

		Event<Application::EndFrame>::Receive(this, [this](const Application::EndFrame& endFrame) {
			const auto delay = Seconds(1.0f / 24 - Seconds(endFrame.eventTime - startFrameTime).count());
			std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(delay));
		});
	}

private:
	TimePoint startFrameTime;
};
