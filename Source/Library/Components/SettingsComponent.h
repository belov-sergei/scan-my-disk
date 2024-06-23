// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "Settings.h"

struct SettingsComponent final {
	SettingsComponent() {
		Event<Application::Loading>::Receive(this, [](const Application::Loading&) {
			Settings<>::Load();
		});
	}
};
