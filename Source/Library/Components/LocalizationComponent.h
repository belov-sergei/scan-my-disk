// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "Settings.h"

struct LocalizationComponent final {
	LocalizationComponent() {
		Event<Application::Loading>::Receive(this, [](const Application::Loading&) {
			LoadText();
			Localization::Language(Settings<User>::Language);
		});
	}

private:
	static void LoadText();
};
