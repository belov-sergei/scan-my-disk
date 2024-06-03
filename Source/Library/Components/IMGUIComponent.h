// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "GLFWWindowComponent.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl2.h>

struct IMGUIComponent final {
	struct Draw {};

	IMGUIComponent() {
		Event<GLFWWindowComponent::Create>::Receive(this, [this](const GLFWWindowComponent::Create& event) {
			ImGui::CreateContext();

			ImGui::GetIO().IniFilename = nullptr;
			ImGui::GetIO().LogFilename = nullptr;

			ImGui_ImplGlfw_InitForOpenGL(event.window, true);
			ImGui_ImplOpenGL2_Init();

			ImFontConfig config;
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSans-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

			config.MergeMode = true;
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansSC-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansJP-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
			ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansKR-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesKorean());

			window = event.window;
		});

		Event<GLFWWindowComponent::Destroy>::Receive(this, [](const GLFWWindowComponent::Destroy&) {
			ImGui_ImplOpenGL2_Shutdown();
			ImGui_ImplGlfw_Shutdown();
			ImGui::DestroyContext();
		});

		Event<Application::Draw>::Receive(this, [this](const Application::Draw&) {
			ImGui_ImplOpenGL2_NewFrame();
			ImGui_ImplGlfw_NewFrame();

			ImGui::NewFrame();
			Event<Draw>::Send();
			ImGui::Render();

			ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
			glfwSwapBuffers(window);
		});
	}

private:
	GLFWwindow* window = nullptr;
};
