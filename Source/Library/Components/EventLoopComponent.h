// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"

struct EventLoopComponent final {
	EventLoopComponent() {
		Event<Application::Launch>::Receive(this, [this](const auto&) {
			while (!exit) {
				Event<Application::Update>::Send();
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
};
