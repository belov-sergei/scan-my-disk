// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "Settings.h"

struct LocalizationComponent final {
	LocalizationComponent() {
		Event<Application::Launch>::Receive(this, [](const auto&) {
			LoadText();
			Localization::Language(Settings<User>::Language);
		});
	}

private:
	static void LoadText();
};
