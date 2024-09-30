// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "GLFW/glfw3.h"
#include "Icons.Generated.h"
#include "Image.h"

struct GLFWWindowComponent final {
	struct Create {
		GLFWwindow* window = nullptr;
	};

	struct Destroy {};

	GLFWWindowComponent() {
		Event<Application::Initialize>::Receive(this, [this](const Application::Initialize&) {
			glfwInit();

			window = glfwCreateWindow(440, 540, "Scan My Disk", nullptr, nullptr);

			GLFWimage applicationIcon;
			applicationIcon.pixels = Image::Load(ApplicationIcon, sizeof(ApplicationIcon), applicationIcon.width, applicationIcon.height, 4);

			glfwSetWindowIcon(window, 1, &applicationIcon);
			glfwMakeContextCurrent(window);

			Event<Create>::Send(window);
		});

		Event<Application::Exit>::Receive(this, [this](Application::Exit& event) {
			Event<Destroy>::Send();

			glfwDestroyWindow(window);
			glfwTerminate();

			event.exitCode = 0;
		});
	}

private:
	GLFWwindow* window = nullptr;
};
