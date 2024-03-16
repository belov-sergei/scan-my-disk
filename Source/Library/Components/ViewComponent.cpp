// Copyright ❤️ 2023-2024, Sergei Belov

#pragma once
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include "ViewComponent.h"
#include "Settings.h"
#include "Localization.h"
#include "Chart.h"
#include "Window.h"

using SliceDrawData = std::vector<std::tuple<float, float, float, float, const Tree::Node<Filesystem::Entry>*>>;

int w = 440, h = 540;

void LoadTexture(std::string_view path, GLuint& textureId) {
	int width, height;
	if (auto* pixels = stbi_load(path.data(), &width, &height, nullptr, 4)) {
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		stbi_image_free(pixels);
	}
}

SliceDrawData BuildDrawData(const Tree::Node<Filesystem::Entry>& node) {
	const size_t root = node->size;
	const size_t depth = node->depth;

	std::vector<float> depthStartAngle(1);

	size_t size = 0;
	for (const auto& child : node) {
		size += child->size;
	}

	SliceDrawData result;
	node.depthTraversal([&](const Tree::Node<Filesystem::Entry>& entry) {
		if (entry->depth - depth >= 7) {
			return false;
		}

		if (depthStartAngle.size() < entry->depth - depth + 2) {
			depthStartAngle.emplace_back();
		}

		float& start = depthStartAngle[entry->depth - depth];
		depthStartAngle[entry->depth - depth + 1] = start;

		const float normalized = entry->size / (float)root;
		const float relative = entry->size / (float)size;
		if (relative > 0.01) {
			result.emplace_back(32 * (entry->depth - depth + 1), start, start + normalized, std::max(0.0f, 0.25f - normalized * 0.75f), &entry);
		}

		start += normalized;

		return false;
	});

	return result;
}

std::pair<size_t, size_t> space = {};

SliceDrawData drawData;

std::atomic<size_t> progress = 0;


Tree::Node<Filesystem::Entry> tree;

std::stack<const Tree::Node<Filesystem::Entry>*> history;

std::filesystem::path path;
std::string size;

namespace ImGui {
	namespace Shadow {
		namespace Details {
			int* Index;

			ImVec2* Position;
			ImVec2* Size;

			ImDrawList* DrawList;

			std::stack<std::tuple<int, ImVec2, ImVec2>> State;

			constexpr float UV[] = {0.00f, 1.0f / 3.0f, 1.0f / 1.5f, 1.00f};

			void AddImage(float minX, float minY, float maxX, float maxY) {
				DrawList->AddImage((void*)icons[Icons::Shadow], {}, {}, {minX, minY}, {maxX, maxY}, IM_COL32(0, 0, 0, 128));
			}
		}

		void Begin() {
			if (Details::State.empty()) {
				Details::DrawList = ImGui::GetWindowDrawList();
			}

			auto& [index, position, size] = Details::State.emplace();

			Details::Index = &index;
			Details::Position = &position;
			Details::Size = &size;

			*Details::Index = Details::DrawList->VtxBuffer.Size;

			Details::AddImage(Details::UV[0], Details::UV[0], Details::UV[1], Details::UV[1]);
			Details::AddImage(Details::UV[1], Details::UV[0], Details::UV[2], Details::UV[1]);
			Details::AddImage(Details::UV[2], Details::UV[0], Details::UV[3], Details::UV[1]);

			Details::AddImage(Details::UV[0], Details::UV[1], Details::UV[1], Details::UV[2]);
			Details::AddImage(Details::UV[1], Details::UV[1], Details::UV[2], Details::UV[2]);
			Details::AddImage(Details::UV[2], Details::UV[1], Details::UV[3], Details::UV[2]);

			Details::AddImage(Details::UV[0], Details::UV[2], Details::UV[1], Details::UV[3]);
			Details::AddImage(Details::UV[1], Details::UV[2], Details::UV[2], Details::UV[3]);
			Details::AddImage(Details::UV[2], Details::UV[2], Details::UV[3], Details::UV[3]);
		}

		void Position(ImVec2 position) {
			*Details::Position = position;

			Details::Position->x -= 130;
			Details::Position->y -= 130;
		}

		void Size(ImVec2 size) {
			*Details::Size = size;

			Details::Size->x += 260;
			Details::Size->y += 260;
		}

		void End() {
			const auto [w, h] = *Details::Size;

			if (!(w > 0.0f && h > 0.0f)) {
				Details::State.pop();

				if (!Details::State.empty()) {
					auto& [index, position, size] = Details::State.top();

					Details::Index = &index;
					Details::Position = &position;
					Details::Size = &size;
				}

				return;
			}

			const auto [x, y] = *Details::Position;

			const std::initializer_list<ImVec2> vertices = {
				{0, 0},
				{130, 0},
				{130, 130},
				{0, 130},

				{130, 0},
				{w - 130, 0},
				{w - 130, 130},
				{130, 130},

				{w - 130, 0},
				{w, 0},
				{w, 130},
				{w - 130, 130},
				// ---
				{0, 130},
				{130, 130},
				{130, h - 130},
				{0, h - 130},

				{130, 130},
				{w - 130, 130},
				{w - 130, h - 130},
				{130, h - 130},

				{w - 130, 130},
				{w, 130},
				{w, h - 130},
				{w - 130, h - 130},
				// ---
				{0, h - 130},
				{130, h - 130},
				{130, h},
				{0, h},

				{130, h - 130},
				{w - 130, h - 130},
				{w - 130, h},
				{130, h},

				{w - 130, h - 130},
				{w, h - 130},
				{w, h},
				{w - 130, h},
			};

			int index = *Details::Index;

			for (const auto& [vx, vy] : vertices) {
				Details::DrawList->VtxBuffer[index++].pos = {x + vx, y + vy};
			}

			Details::State.pop();

			if (!Details::State.empty()) {
				auto& [index, position, size] = Details::State.top();

				Details::Index = &index;
				Details::Position = &position;
				Details::Size = &size;
			}
		}
	}
}

std::string BytesToString(size_t value) {
	constexpr std::array units = {"B", "KB", "MB", "GB", "TB", "PB"};

	auto size = static_cast<double>(value);

	size_t unit = 0;
	while (size >= 1024 && unit < units.size()) {
		size /= 1024;
		unit++;
	}

	return fmt::format("{:.2f} {}", size, units[unit]);
}

void StartedState() {
	ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);

	ImGui::Indent(30);
	ImGui::Text(Localization::Text("StartedState_SelectDisk_Text"));

	{
		const auto& [x, y] = ImGui::GetCursorPos();
		ImGui::GetWindowDrawList()->AddLine({x, y}, {x + ImGui::GetWindowWidth() - 60, y + 1}, IM_COL32(190, 190, 190, 255));
	}

	float buttonWidth = 165.0f;

	for (const auto& volume : volumes) {
		const auto bytesFreeText = BytesToString(volume.bytesFree);
		const auto bytesTotalText = BytesToString(volume.bytesTotal);

		const auto textSize = ImGui::CalcTextSize(fmt::vformat((std::string_view)Localization::Text("StartedState_FreeSpace_Text"), fmt::make_format_args(bytesFreeText, bytesTotalText)).c_str());
		buttonWidth = std::max(buttonWidth, textSize.x + 3.0f);
	}

	int buttonsInRow = 0;

	ImGui::NewLine();
	for (const auto& volume : volumes) {
		ImGui::BeginGroup();
		{
			if (volume.name.empty()) {
				ImGui::Text(volume.rootPath.c_str());
			}
			else {
				ImGui::Text(volume.name.c_str());
			}

			const float scale = 1.0f - volume.bytesFree / (float)volume.bytesTotal;

			const auto [x, y] = ImGui::GetCursorPos();

			if (ImRect(x - 5, y - 25, x + buttonWidth, y + 35).Contains(ImGui::GetMousePos())) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					space = std::make_pair(volume.bytesTotal, volume.bytesFree);

					progress = 0;
					future = std::async(std::launch::async, [volume] {
						return Filesystem::ParallelBuildTree(volume.rootPath, progress);
					});

					state = State::Loading;
				}

				ImGui::GetWindowDrawList()->AddRectFilled({x - 5, y - 25}, {x + buttonWidth, y + 35}, IM_COL32(190, 190, 190, 32), 8);
			}

			ImGui::ItemSize({x, y, x + (buttonWidth + 5.0f), y + 8});

			ImGui::GetWindowDrawList()->AddRectFilled({x, y}, {x + (buttonWidth - 5.0f), y + 8}, IM_COL32(190, 190, 190, 127));
			ImGui::GetWindowDrawList()->AddRectFilled({x, y}, {x + (buttonWidth - 5.0f) * scale, y + 8}, IM_COL32(190, 190, 190, 255));

			const auto bytesFreeText = BytesToString(volume.bytesFree);
			const auto bytesTotalText = BytesToString(volume.bytesTotal);
			ImGui::Text("%s", fmt::vformat((std::string_view)Localization::Text("StartedState_FreeSpace_Text"), fmt::make_format_args(bytesFreeText, bytesTotalText)).c_str());
		}
		ImGui::EndGroup();

		if (++buttonsInRow * (buttonWidth + 5.0f) + (buttonWidth + 5.0f) + 60 < ImGui::GetWindowWidth()) {
			ImGui::SameLine();
		}
		else {
			buttonsInRow = 0;
		}
	}

	ImGui::PopStyleColor();
}

void LoadingState() {
	using namespace std::chrono_literals;

	const float scale = std::min<float>(progress / float(space.first - space.second), 1.0f);

	ImGui::SetCursorPosY(ImGui::GetWindowHeight() * 0.5f);
	ImGui::Indent(30);

	const auto& [x, y] = ImGui::GetCursorPos();

	ImGui::ItemSize({x, y, x + ImGui::GetWindowWidth() - 60, y + 8});
	ImGui::GetWindowDrawList()->AddRect({x, y}, {x + ImGui::GetWindowWidth() - 60, y + 8}, IM_COL32(190, 190, 190, 127));
	ImGui::GetWindowDrawList()->AddRectFilled({x, y}, {x + (ImGui::GetWindowWidth() - 60) * scale, y + 8}, IM_COL32(190, 190, 190, 255));

	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8, 4));
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);

	ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);
	ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(55, 57, 62, 255));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 32));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 32));

	const auto buttonText = Localization::Text("LoadingState_AbortLoading_Button");

	ImGui::NewLine();
	ImGui::SetCursorPosX((ImGui::GetWindowWidth() - (ImGui::CalcTextSize(buttonText).x + ImGui::GetStyle().FramePadding.x * 2.0f)) * 0.5f);

	if (ImGui::Button(buttonText)) {
		Filesystem::CancelBuildTree();
		std::ignore = future.get();

		state = State::Started;
	}

	ImGui::PopStyleColor(4);
	ImGui::PopStyleVar(2);

	if (future.valid() && future.wait_for(0s) == std::future_status::ready) {
		tree = future.get();

		// The size of the root is replaced with the size of the disk. If the size is 0, then it is the folder selected by the user.
		if (space.first > 0) {
			tree->size = space.first;
		}

		drawData = BuildDrawData(tree);

		history.emplace(&tree);
		state = State::Chart;
	}
}

void ChartState() {
	static float scale = 1.0f;
	static float offsetX = 0.0f, offsetY = 0.0f;

	scale += ImGui::GetIO().MouseWheel * scale * 0.25f;
	scale = std::max(scale, 0.5f);

	if (ImGui::IsMouseDragging(ImGuiMouseButton_Middle)) {
		offsetX += ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle).x / scale;
		offsetY += ImGui::GetMouseDragDelta(ImGuiMouseButton_Middle).y / scale;

		ImGui::ResetMouseDragDelta(ImGuiMouseButton_Middle);
	}

	ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);

	ImGui::Indent(30);

	ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 60);

	std::string string = path.u8string();
	std::string text = std::string(string.begin(), string.end());

	text = fmt::vformat((const std::string&)Localization::Text("ChartState_Path_Text"), fmt::make_format_args(text));

	ImGui::TextWrapped("%s", text.c_str());
	ImGui::PopTextWrapPos();

	ImGui::Text("%s", fmt::vformat((std::string_view)Localization::Text("ChartState_Size_Text"), fmt::make_format_args(size)).c_str());

	std::filesystem::path root = (*history.top())->path;
	if (history.size() > 1) {
		root = std::filesystem::relative(root, root.parent_path());
	}

	auto [x, y] = ImGui::GetWindowSize();
	x *= 0.5f;
	y = y * 0.5f + 30;

	const auto mx = ImGui::GetMousePos().x - x - offsetX * scale;
	const auto my = ImGui::GetMousePos().y - y - offsetY * scale;

	const float length = std::sqrt(mx * mx + my * my);
	float angle = ((int)(std::atan2(-my, -mx) * 180 / 3.14) + 180) / 360.0f;

	const float sliceScale = std::min(w, h - 120) * scale;

	const auto cx = x + offsetX * scale;
	const auto cy = y + offsetY * scale;

	Chart::Pie::Begin({cx, cy});

	for (auto it = drawData.rbegin(); it != drawData.rend(); ++it) {
		const auto& [radius, start, end, hue, node] = *it;
		auto* top = history.top();

		if (!ImGui::IsPopupOpen("Menu") && !ImGui::IsPopupOpen("File")) {
			if (length <= (radius * sliceScale / 512) && length >= ((radius - 32) * sliceScale / 512) && angle >= start && angle <= end) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					if (node == top) {
						if (history.size() > 1) {
							history.pop();
							drawData = BuildDrawData(*history.top());
						}
						else {
							history = {};
							state = State::Started;
						}
					}
					else {
						if (!node->isLeaf()) {
							history.emplace(node);
							drawData = BuildDrawData(*node);
						}
					}

					break;
				}

				if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
					ImGui::OpenPopup("Menu");
				}

				if (node == top) {
					Chart::Pie::Color(IM_COL32(94, 97, 101, 255), IM_COL32(55, 57, 62, 255));
				}
				else {
					Chart::Pie::Color(ImColor::HSV(hue, 0.15f, 0.9f));
				}

				path = (*node)->path;
				size = BytesToString((*node)->size);
			}
			else {
				if (node == top) {
					Chart::Pie::Color(IM_COL32(72, 74, 78, 255), IM_COL32(55, 57, 62, 255));
				}
				else {
					Chart::Pie::Color(ImColor::HSV(hue, (1 - radius / 420) * 0.75f, 0.9f));
				}
			}
		}
		else {
			if (node == top) {
				Chart::Pie::Color(IM_COL32(72, 74, 78, 255), IM_COL32(55, 57, 62, 255));
			}
			else {
				Chart::Pie::Color(ImColor::HSV(hue, (1 - radius / 420) * 0.75f, 0.9f));
			}
		}

		Chart::Pie::Slice(radius * sliceScale / 512, start, end);
	}
	Chart::Pie::End();

	ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(55, 57, 62, 255));
	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(67, 69, 74, 255));
	ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);

	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 5));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(80, 5));

	ImGui::Shadow::Begin();
	if (ImGui::BeginPopup("Menu")) {
		ImGui::Shadow::Position(ImGui::GetWindowPos());
		ImGui::Shadow::Size(ImGui::GetWindowSize());

		ImGui::NewLine();
		ImGui::SameLine(18.0f);

		if (ImGui::MenuItem(Localization::Text("ChartState_Explore_Button"))) {
			Filesystem::OpenSystemPath(path);
		}

		ImGui::EndPopup();
	}
	ImGui::Shadow::End();

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(3);

	ImGui::SetCursorPos({cx - 6 * scale, cy - 6 * scale});
	ImGui::Image((void*)icons[Icons::Back], {12 * scale, 12 * scale});

	ImGui::GetWindowDrawList()->AddRectFilled({0, ImGui::GetWindowHeight() - 30}, {ImGui::GetWindowWidth(), ImGui::GetWindowHeight()}, IM_COL32(55, 57, 62, 255));
	ImGui::GetWindowDrawList()->AddLine({0, ImGui::GetWindowHeight() - 30}, {ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 30}, IM_COL32(43, 45, 48, 255));

	ImGui::SetCursorPos({0 + 9, ImGui::GetWindowHeight() - 21});
	ImGui::Image((void*)icons[Icons::Folder], {14, 13});

	ImGui::SameLine();

	ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 24);

	string = root.u8string();
	text = std::string(string.begin(), string.end());

	ImGui::Text("%s", text.c_str());

	ImGui::PopStyleColor(1);
}

void Draw() {
	SDL_GetWindowSize(window, &w, &h);

	ImGui::SetNextWindowPos({});
	ImGui::SetNextWindowSize({(float)w, (float)h});

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());

	ImGui::Begin("Scan My Disk", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

	// Window Background.
	{
		ImGui::GetWindowDrawList()->AddRectFilled({}, ImGui::GetWindowSize(), IM_COL32(55, 57, 62, 255));
	}

	const auto close = []() {
		SDL_Event quit;
		quit.type = SDL_QUIT;

		SDL_PushEvent(&quit);
	};

	ImGui::SetCursorPos({0, 50});

	switch (state) {
		case State::Started:
			StartedState();
			break;
		case State::Loading:
			LoadingState();
			break;
		case State::Chart:
			ChartState();
			break;
		default: ;
	}

	// Window Title.
	{
		ImGui::SetCursorPos({});

		if (CustomWindowTitleEnabled()) {
			ImGui::GetWindowDrawList()->AddRectFilled({}, {ImGui::GetWindowWidth(), 30}, IM_COL32(43, 45, 48, 255));
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(18, 9));

		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 32));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 32));

		if (ImGui::ImageButton("#Menu", (void*)icons[Icons::Menu], {12, 12})) {
			ImGui::OpenPopup("File");
		}

		if (CustomWindowTitleEnabled()) {
			ImGui::SetCursorPos({ImGui::GetWindowWidth() - 48 * 3, 0});
			if (ImGui::ImageButton("#Minimize", (void*)icons[Icons::Minimize], {12, 12})) {
				SDL_MinimizeWindow(window);
			}

			if (SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) {
				ImGui::SetCursorPos({ImGui::GetWindowWidth() - 48 * 2, 0});
				if (ImGui::ImageButton("#Restore", (void*)icons[Icons::Restore], {12, 12})) {
					SDL_RestoreWindow(window);
				}
			}
			else {
				ImGui::SetCursorPos({ImGui::GetWindowWidth() - 48 * 2, 0});
				if (ImGui::ImageButton("#Maximize", (void*)icons[Icons::Maximize], {12, 12})) {
					SDL_MaximizeWindow(window);
				}
			}

			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(196, 43, 23, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(196, 43, 23, 255));

				ImGui::SetCursorPos({ImGui::GetWindowWidth() - 48, 0});
				if (ImGui::ImageButton("#Close", (void*)icons[Icons::Close], {12, 12})) {
					close();
				}

				ImGui::PopStyleColor(2);
			}
		}

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		if (CustomWindowTitleEnabled()) {
			ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);
			{
				const auto* windowTitle = "Scan My Disk";
				ImGui::SetCursorPos({ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(windowTitle).x / 2, 15 - ImGui::CalcTextSize(windowTitle).y / 2});
				ImGui::Text(windowTitle);
			}
			ImGui::PopStyleColor();
		}
	}

	ImGui::SetCursorPos({0, 50});
	ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(55, 57, 62, 255));
	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(67, 69, 74, 255));
	ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);

	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 5));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(80, 5));

	ImGui::SetNextWindowPos({10, 30});

	ImGui::Shadow::Begin();
	if (ImGui::BeginPopup("File")) {
		ImGui::Shadow::Position(ImGui::GetWindowPos());
		ImGui::Shadow::Size(ImGui::GetWindowSize());

		ImGui::NewLine();
		ImGui::SameLine(18.0f);
		if (ImGui::MenuItem(Localization::Text("Menu_OpenFolder_Button"))) {
			const auto path = Filesystem::OpenSelectFolderDialog();
			if (!path.empty() && std::filesystem::exists(path)) {
				space = std::make_pair(0, 0);

				progress = 0;
				future = std::async(std::launch::async, [path] {
					return Filesystem::ParallelBuildTree(path, progress);
				});

				state = State::Loading;
			}
		}

		ImGui::Separator();

		ImGui::NewLine();
		ImGui::SameLine(18.0f);

		ImGui::Shadow::Begin();
		if (ImGui::BeginMenu(Localization::Text("Menu_Language_Button"))) {
			ImGui::Shadow::Position(ImGui::GetWindowPos());
			ImGui::Shadow::Size(ImGui::GetWindowSize());

			for (const auto language : Localization::Languages()) {
				ImGui::NewLine();
				ImGui::SameLine(18.0f);
				if (ImGui::MenuItem(Localization::Text(language))) {
					Localization::Language(language);

					Settings<User>::Language = language;
					Settings<>::Save();
				}
			}

			ImGui::EndMenu();
		}
		ImGui::Shadow::End();

		ImGui::Separator();

		ImGui::NewLine();
		ImGui::SameLine(18.0f);
		if (ImGui::MenuItem(Localization::Text("Menu_Exit_Button"))) {
			close();
		}

		ImGui::EndPopup();
	}
	ImGui::Shadow::End();

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(3);

	ImGui::End();

	ImGui::PopStyleVar();
}
