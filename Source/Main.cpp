// Copyright ❤️ 2023-2024, Sergei Belov

#include "Components/ComponentGroup.h"
#include "Components/EventLoopComponent.h"
#include "Components/FrameRateComponent.h"
#include "Components/GLFWEventComponent.h"
#include "Components/GLFWWindowComponent.h"
#include "Components/IMGUIComponent.h"
#include "Components/IMGUIFontComponent.h"
#include "Components/LocalizationComponent.h"
#include "Components/SettingsComponent.h"
#include "Components/SystemFontComponent.h"
#include "Components/TracyFrameComponent.h"
#include "Components/TracyMemoryPlotComponent.h"
#include "Components/ViewComponent.h"
#include "Components/WindowTitleComponent.h"

#include <Main.h>

int main(int argc, char* argv[]) {
	// clang-format off
	const auto componentGroup = ComponentGroup<
		SystemFontComponent,
		EventLoopComponent,
		GLFWEventComponent,
		GLFWWindowComponent,
		IMGUIComponent,
		IMGUIFontComponent,
		SettingsComponent,
		LocalizationComponent,
		ViewComponent,
		WindowTitleComponent,
		FrameRateComponent,
		TracyMemoryPlotComponent,
		TracyFrameComponent
	>();
	// clang-format on

	Event<Application::Loading>::Send();
	Event<Application::Initialize>::Send();
	Event<Application::Launch>::Send();

	Application::Exit exitEvent;
	Event<Application::Exit>::Send(exitEvent);

	return exitEvent.exitCode;
}
