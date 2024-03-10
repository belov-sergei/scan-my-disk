// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "SDLWindowComponent.h"
#include "SDLEventComponent.h"

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>

struct IMGUIComponent final {
	struct Draw {};

	IMGUIComponent() {
		Event<SDLWindowComponent::Create>::Receive(this, [this](const SDLWindowComponent::Create& event) {
			ImGui::CreateContext();

			ImGui::GetIO().IniFilename = nullptr;
			ImGui::GetIO().LogFilename = nullptr;

			ImGui_ImplSDL2_InitForOpenGL(event.window, event.context);
			ImGui_ImplOpenGL3_Init("#version 120");
			
			ImFontConfig config;
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSans-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

			config.MergeMode = true;
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansSC-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansJP-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansKR-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesKorean());

			window = event.window;
		});

		Event<SDLEventComponent::Process>::Receive(this, [this](const SDLEventComponent::Process& event) {
			ImGui_ImplSDL2_ProcessEvent(&event.data);
		});

		Event<SDLWindowComponent::Destroy>::Receive(this, [this](const auto&) {
			ImGui_ImplOpenGL3_Shutdown();
			ImGui_ImplSDL2_Shutdown();
			ImGui::DestroyContext();
		});

		Event<Application::Draw>::Receive(this, [this](const auto&) {
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplSDL2_NewFrame();

			ImGui::NewFrame();
			Event<Draw>::Send();
			ImGui::Render();

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
			SDL_GL_SwapWindow(window);
		});
	}

private:
	SDL_Window* window = nullptr;
};
