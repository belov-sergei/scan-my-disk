// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"

#include <tracy/Tracy.hpp>

struct EventLoopComponent final {
	using Seconds = std::chrono::duration<float>;
	using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	EventLoopComponent() {
		Event<Application::Launch>::Receive(this, [this](const auto&) {
			const Application::StartFrame startFrame;
			lastFrameTime = startFrame.eventTime;

			while (!exit) {
				Event<Application::StartFrame>::Send();

				Event<Application::Update>::Send(deltaTime);
				Event<Application::LateUpdate>::Send();

				Event<Application::Draw>::Send();

				Event<Application::EndFrame>::Send();
				FrameMark;
			}
		});

		Event<Application::Terminate>::Receive(this, [this](const auto&) {
			exit = true;
		});

		Event<Application::StartFrame>::Receive(this, [this](const Application::StartFrame& startFrame) {
			deltaTime = Seconds(startFrame.eventTime - lastFrameTime).count();
			lastFrameTime = startFrame.eventTime;
		});
	}

private:
	bool exit = false;
	float deltaTime = 0.0f;

	TimePoint lastFrameTime;
};
