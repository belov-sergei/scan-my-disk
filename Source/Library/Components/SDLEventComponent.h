// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include <SDL.h>

struct SDLEventComponent final {
	struct Process {
		SDL_Event& data;
	};

	SDLEventComponent() {
		Event<Application::Update>::Receive(this, [](const auto&) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT:
						Event<Application::Terminate>::Send();
						break;
					default:
						Event<Process>::Send(event);
				}
			}
		});
	}
};
