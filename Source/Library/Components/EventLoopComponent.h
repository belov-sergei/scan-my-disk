// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"

struct EventLoopComponent final {
	using Seconds = std::chrono::duration<float>;
	using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;

	EventLoopComponent() {
		Event<Application::Launch>::Receive(this, [this](const auto&) {
			lastFrameTime = std::chrono::steady_clock::now();

			while (!exit) {
				const auto now = std::chrono::steady_clock::now();

				const Seconds deltaTime = now - lastFrameTime;
				lastFrameTime = now;

				Event<Application::Update>::Send(deltaTime.count());
				Event<Application::LateUpdate>::Send();

				Event<Application::Draw>::Send();
			}
		});

		Event<Application::Terminate>::Receive(this, [this](const auto&) {
			exit = true;
		});
	}

private:
	bool exit = false;
	TimePoint lastFrameTime;
};
