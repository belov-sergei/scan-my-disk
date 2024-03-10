// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "Settings.h"

struct SettingsComponent final {
	SettingsComponent() {
		Event<Application::Initialize>::Receive(this, [](const auto&) {
			Settings<>::Load();
		});
	}
};
