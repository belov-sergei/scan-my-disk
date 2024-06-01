// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "SDLWindowComponent.h"
#include "Window.h"

struct WindowTitleComponent final {
	WindowTitleComponent() {
		Event<SDLWindowComponent::Create>::Receive(this, [](const SDLWindowComponent::Create& event) {
			SetWindowProc(440, 540);

			SDL_AddEventWatch(
			[](void* data, SDL_Event* event) {
				if (event->type == SDL_WINDOWEVENT) {
					// To prevent recursive calls to Draw function when the user clicks on Maximize and Minimize.
					if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
						if (!ImGui::GetCurrentContext()->WithinFrameScope) {
							Event<Application::Draw>::Send();
						}
					}
				}
				return 0;
			},
			event.window);

			if (CustomWindowTitleEnabled()) {
				SDL_SetWindowHitTest(
				event.window,
				[](auto* window, const auto* area, auto*) {
					int w, h;
					SDL_GetWindowSize(window, &w, &h);

					SDL_Rect draggable;
					draggable.x = 48;
					draggable.y = 0;
					draggable.w = w - 48 * 4;
					draggable.h = 30;

					if (SDL_PointInRect(area, &draggable) == SDL_TRUE) {
						return SDL_HITTEST_DRAGGABLE;
					} else if (area->x > w - 10 && area->y > h - 10) {
						return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
					}

					return SDL_HITTEST_NORMAL;
				},
				nullptr);
			}
		});
	}
};
