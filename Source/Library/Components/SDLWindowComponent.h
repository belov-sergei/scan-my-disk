// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"

#include <SDL.h>

struct SDLWindowComponent final {
	struct Create {
		SDL_Window* window    = nullptr;
		SDL_GLContext context = nullptr;
	};

	struct Destroy {};

	SDLWindowComponent() {
		Event<Application::Initialize>::Receive(this, [this](const auto&) {
			SDL_Init(SDL_INIT_VIDEO);

			window  = SDL_CreateWindow("Scan My Disk", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 440, 540, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
			context = SDL_GL_CreateContext(window);

			Event<Create>::Send(window, context);
		});

		Event<Application::Exit>::Receive(this, [this](Application::Exit& event) {
			Event<Destroy>::Send();

			SDL_GL_DeleteContext(context);
			SDL_DestroyWindow(window);

			SDL_Quit();

			event.exitCode = 0;
		});
	}

private:
	SDL_Window* window    = nullptr;
	SDL_GLContext context = nullptr;
};
