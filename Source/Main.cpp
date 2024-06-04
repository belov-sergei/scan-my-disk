// Copyright ❤️ 2023-2024, Sergei Belov

#include <windows.h>

#include "Components/ComponentGroup.h"
#include "Components/EventLoopComponent.h"
#include "Components/FrameRateComponent.h"
#include "Components/GLFWEventComponent.h"
#include "Components/GLFWWindowComponent.h"
#include "Components/IMGUIComponent.h"
#include "Components/LocalizationComponent.h"
#include "Components/SettingsComponent.h"
#include "Components/ViewComponent.h"
#include "Components/WindowTitleComponent.h"

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow) {
	if (!std::filesystem::exists("README.md")) {
		// std::filesystem::current_path(SDL_GetBasePath());
	}

	// clang-format off
	std::ignore = ComponentGroup<
		EventLoopComponent,
		GLFWEventComponent,
		GLFWWindowComponent,
		IMGUIComponent,
		SettingsComponent,
		LocalizationComponent,
		ViewComponent,
		WindowTitleComponent,
		FrameRateComponent
	>();
	// clang-format on

	Event<Application::Initialize>::Send();
	Event<Application::Launch>::Send();

	Application::Exit exitEvent;
	Event<Application::Exit>::Send(exitEvent);

	return exitEvent.exitCode;
}
