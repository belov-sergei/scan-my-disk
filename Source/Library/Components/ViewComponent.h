// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "Filesystem.h"
#include "IMGUIComponent.h"

#include <imgui_internal.h>
#include <SDL_opengl.h>

void Draw();
void LoadTexture(std::string_view path, ImTextureID& textureId);

inline GLFWwindow* window = nullptr;
inline std::vector<Filesystem::VolumeData> volumes;

enum class State { Started, Loading, Chart };

inline State state = State::Started;
inline std::future<Tree::Node<Filesystem::Entry>> future;

enum Icons {
	Close,
	Maximize,
	Menu,
	Minimize,
	Restore,
	Icon,
	Shadow,
	Back,
	Folder,

	Last
};

inline std::array<ImTextureID, Icons::Last> icons = {};

struct ViewComponent final {
	ViewComponent() {
		Event<GLFWWindowComponent::Create>::Receive(this, [](const GLFWWindowComponent::Create& event) {
			LoadTexture("Icons/Close.png", icons[Icons::Close]);
			LoadTexture("Icons/Maximize.png", icons[Icons::Maximize]);
			LoadTexture("Icons/Menu.png", icons[Icons::Menu]);
			LoadTexture("Icons/Minimize.png", icons[Icons::Minimize]);
			LoadTexture("Icons/Restore.png", icons[Icons::Restore]);
			LoadTexture("Icons/Icon.png", icons[Icons::Icon]);
			LoadTexture("Icons/Shadow.png", icons[Icons::Shadow]);
			LoadTexture("Icons/Back.png", icons[Icons::Back]);
			LoadTexture("Icons/Folder.png", icons[Icons::Folder]);

			volumes = Filesystem::GetVolumesData();

			window = event.window;
		});

		Event<IMGUIComponent::Draw>::Receive(this, [](const auto&) {
			Draw();
		});

		Event<Application::Terminate>::Receive(this, [](const auto&) {
			if (state == State::Loading) {
				Filesystem::CancelBuildTree();
				std::ignore = future.get();
			}
		});
	}
};
