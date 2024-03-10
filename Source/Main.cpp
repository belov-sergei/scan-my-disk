// Copyright ❤️ 2023-2024, Sergei Belov

#include "Components/ComponentGroup.h"
#include "Components/EventLoopComponent.h"
#include "Components/SDLEventComponent.h"
#include "Components/SDLWindowComponent.h"
#include "Components/IMGUIComponent.h"
#include "Components/SettingsComponent.h"
#include "Components/LocalizationComponent.h"
#include "Components/ViewComponent.h"
#include "Components/WindowTitleComponent.h"

int main(int argc, char* argv[]) {
	if (!std::filesystem::exists("README.md")) {
		std::filesystem::current_path(SDL_GetBasePath());
	}

	std::ignore = ComponentGroup<
		EventLoopComponent,
		SDLEventComponent,
		SDLWindowComponent,
		IMGUIComponent,
		SettingsComponent,
		LocalizationComponent,
		ViewComponent,
		WindowTitleComponent
	>();

	Event<Application::Initialize>::Send();
	Event<Application::Launch>::Send();

	Application::Exit exitEvent;
	Event<Application::Exit>::Send(exitEvent);

	return exitEvent.exitCode;
}
