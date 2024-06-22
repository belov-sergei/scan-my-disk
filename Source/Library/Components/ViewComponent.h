// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#include "Application.h"
#include "Filesystem.h"
#include "Icons.Generated.h"
#include "IMGUIComponent.h"

#include <imgui_internal.h>

void Draw();
void LoadTexture(const unsigned char* buffer, int length, ImTextureID& textureId);

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
	Shadow,
	Back,
	Folder,

	Last
};

inline std::array<ImTextureID, Icons::Last> icons = {};

struct ViewComponent final {
	ViewComponent() {
		Event<GLFWWindowComponent::Create>::Receive(this, [](const GLFWWindowComponent::Create& event) {
			LoadTexture(CloseIcon, sizeof(CloseIcon), icons[Icons::Close]);
			LoadTexture(MaximizeIcon, sizeof(MaximizeIcon), icons[Icons::Maximize]);
			LoadTexture(MenuIcon, sizeof(MenuIcon), icons[Icons::Menu]);
			LoadTexture(MinimizeIcon, sizeof(MinimizeIcon), icons[Icons::Minimize]);
			LoadTexture(RestoreIcon, sizeof(RestoreIcon), icons[Icons::Restore]);
			LoadTexture(ShadowIcon, sizeof(ShadowIcon), icons[Icons::Shadow]);
			LoadTexture(BackIcon, sizeof(BackIcon), icons[Icons::Back]);
			LoadTexture(FolderIcon, sizeof(FolderIcon), icons[Icons::Folder]);

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
