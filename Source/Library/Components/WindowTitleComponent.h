// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "SDLWindowComponent.h"
#include "Window.h"

struct WindowTitleComponent final {
	WindowTitleComponent() {
		Event<GLFWWindowComponent::Create>::Receive(this, [](const GLFWWindowComponent::Create& event) {
			glfwSetWindowSizeCallback(event.window, [](GLFWwindow*, int, int) {
				if (!ImGui::GetCurrentContext()->WithinFrameScope) {
					Event<Application::Draw>::Send();
				}
			});

			SetCustomWindowProcedure();
		});
	}
};
