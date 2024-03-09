// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"

#include <SDL.h>

struct PollEventComponent final {
	PollEventComponent() {
		Event<Application::Update>::Receive(this, [](const auto&) {
			SDL_Event sdlEvent;
			while (SDL_PollEvent(&sdlEvent)) {
				switch (sdlEvent.type) {
					case SDL_QUIT:
						Event<Application::Terminate>::Send();
						break;
					default:
						break;
				}
			}
		});
	}
};
