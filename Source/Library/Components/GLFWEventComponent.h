// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "GLFWWindowComponent.h"

struct GLFWEventComponent final {
	GLFWEventComponent() {
		Event<GLFWWindowComponent::Create>::Receive(this, [this](const GLFWWindowComponent::Create& event) {
			window = event.window;
		});

		Event<Application::Update>::Receive(this, [this](const Application::Update&) {
			if (glfwWindowShouldClose(window)) {
				Event<Application::Terminate>::Send();
				return;
			}

			glfwPollEvents();
		});
	}

private:
	GLFWwindow* window = nullptr;
};
