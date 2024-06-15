// Copyright ❤️ 2023-2024, Sergei Belov

#include "ViewComponent.h"

#include "Chart.h"
#include "Image.h"
#include "Localization.h"
#include "Settings.h"
#include "Window.h"

using SliceDrawData = std::vector<std::tuple<float, float, float, float, const Tree::Node<Filesystem::Entry>*>>;

int WindowWidth { 440 };
int WindowHeight { 540 };

void LoadTexture(std::string_view path, ImTextureID& textureId) {
	int width, height;
	if (auto* pixels = Image::Load(path, width, height, 4)) {
		GLuint glTextureId;
		glGenTextures(1, &glTextureId);
		glBindTexture(GL_TEXTURE_2D, glTextureId);

		textureId = (ImTextureID)(intptr_t)glTextureId;

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		Image::Free(pixels);
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
		if (relative > 0.01f) {
			result.emplace_back(32.0f * static_cast<float>(entry->depth - depth + 1), start, start + normalized, std::max(0.0f, 0.25f - normalized * 0.75f), &entry);
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

std::filesystem::path CurrentPath;
std::string CurrentSize;

namespace ImGui {
	namespace Shadow {
		namespace Details {
			int* Index;

			ImVec2* Position;
			ImVec2* Size;

			ImDrawList* DrawList;

			std::stack<std::tuple<int, ImVec2, ImVec2>> State;

			constexpr float UV[] = { 0.00f, 1.0f / 3.0f, 1.0f / 1.5f, 1.00f };

			void AddImage(float minX, float minY, float maxX, float maxY) {
				DrawList->AddImage(icons[Icons::Shadow], {}, {}, { minX, minY }, { maxX, maxY }, IM_COL32(0, 0, 0, 128));
			}
		} // namespace Details

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
				{       0,       0 },
				{     130,       0 },
				{     130,     130 },
				{       0,     130 },

				{     130,       0 },
				{ w - 130,       0 },
				{ w - 130,     130 },
				{     130,     130 },

				{ w - 130,       0 },
				{       w,       0 },
				{       w,     130 },
				{ w - 130,     130 },
				// ---
				{       0,     130 },
				{     130,     130 },
				{     130, h - 130 },
				{       0, h - 130 },

				{     130,     130 },
				{ w - 130,     130 },
				{ w - 130, h - 130 },
				{     130, h - 130 },

				{ w - 130,     130 },
				{       w,     130 },
				{       w, h - 130 },
				{ w - 130, h - 130 },
				// ---
				{       0, h - 130 },
				{     130, h - 130 },
				{     130,       h },
				{       0,       h },

				{     130, h - 130 },
				{ w - 130, h - 130 },
				{ w - 130,       h },
				{     130,       h },

				{ w - 130, h - 130 },
				{       w, h - 130 },
				{       w,       h },
				{ w - 130,       h },
			};

			{
				int index = *Details::Index;
				for (const auto& [vx, vy] : vertices) {
					Details::DrawList->VtxBuffer[index++].pos = { x + vx, y + vy };
				}
			}

			Details::State.pop();

			if (!Details::State.empty()) {
				auto& [index, position, size] = Details::State.top();

				Details::Index = &index;
				Details::Position = &position;
				Details::Size = &size;
			}
		}
	} // namespace Shadow
} // namespace ImGui

void StartedState() {
	ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);

	ImGui::Indent(30);
	ImGui::Text("%s", (const char*)Localization::Text("StartedState_SelectDisk_Text"));

	{
		const auto& [x, y] = ImGui::GetCursorPos();
		ImGui::GetWindowDrawList()->AddLine({ x, y }, { x + ImGui::GetWindowWidth() - 60, y + 1 }, IM_COL32(190, 190, 190, 255));
	}

	float buttonWidth = 165.0f;

	for (const auto& volume : volumes) {
		const auto bytesFreeText = Filesystem::BytesToString(volume.bytesFree);
		const auto bytesTotalText = Filesystem::BytesToString(volume.bytesTotal);

		const auto textSize = ImGui::CalcTextSize(fmt::vformat((std::string_view)Localization::Text("StartedState_FreeSpace_Text"), fmt::make_format_args(bytesFreeText, bytesTotalText)).c_str());
		buttonWidth = std::max(buttonWidth, textSize.x + 3.0f);
	}

	int buttonsInRow = 0;

	ImGui::NewLine();
	for (const auto& volume : volumes) {
		ImGui::BeginGroup();
		{
			if (volume.name.empty()) {
				ImGui::Text("%s", volume.rootPath.c_str());
			} else {
				ImGui::Text("%s", volume.name.c_str());
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

				ImGui::GetWindowDrawList()->AddRectFilled({ x - 5, y - 25 }, { x + buttonWidth, y + 35 }, IM_COL32(190, 190, 190, 32), 8);
			}

			ImGui::ItemSize({ x, y, x + (buttonWidth + 5.0f), y + 8 });

			ImGui::GetWindowDrawList()->AddRectFilled({ x, y }, { x + (buttonWidth - 5.0f), y + 8 }, IM_COL32(190, 190, 190, 127));
			ImGui::GetWindowDrawList()->AddRectFilled({ x, y }, { x + (buttonWidth - 5.0f) * scale, y + 8 }, IM_COL32(190, 190, 190, 255));

			const auto bytesFreeText = Filesystem::BytesToString(volume.bytesFree);
			const auto bytesTotalText = Filesystem::BytesToString(volume.bytesTotal);
			ImGui::Text("%s", fmt::vformat((std::string_view)Localization::Text("StartedState_FreeSpace_Text"), fmt::make_format_args(bytesFreeText, bytesTotalText)).c_str());
		}
		ImGui::EndGroup();

		if (++buttonsInRow * (buttonWidth + 5.0f) + (buttonWidth + 5.0f) + 60 < ImGui::GetWindowWidth()) {
			ImGui::SameLine();
		} else {
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

	ImGui::ItemSize({ x, y, x + ImGui::GetWindowWidth() - 60, y + 8 });
	ImGui::GetWindowDrawList()->AddRect({ x, y }, { x + ImGui::GetWindowWidth() - 60, y + 8 }, IM_COL32(190, 190, 190, 127));
	ImGui::GetWindowDrawList()->AddRectFilled({ x, y }, { x + (ImGui::GetWindowWidth() - 60) * scale, y + 8 }, IM_COL32(190, 190, 190, 255));

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

	std::string string = CurrentPath.u8string();
	std::string text = std::string(string.begin(), string.end());

	text = fmt::vformat((const std::string&)Localization::Text("ChartState_Path_Text"), fmt::make_format_args(text));

	ImGui::TextWrapped("%s", text.c_str());
	ImGui::PopTextWrapPos();

	ImGui::Text("%s", fmt::vformat((std::string_view)Localization::Text("ChartState_Size_Text"), fmt::make_format_args(CurrentSize)).c_str());

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

	const float sliceScale = std::min(WindowWidth, WindowHeight - 120) * scale;

	const auto cx = x + offsetX * scale;
	const auto cy = y + offsetY * scale;

	Chart::Pie::Begin({ cx, cy });

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
						} else {
							history = {};
							state = State::Started;
						}
					} else {
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
				} else {
					Chart::Pie::Color(ImColor::HSV(hue, 0.15f, 0.9f));
				}

				CurrentPath = (*node)->path;
				CurrentSize = Filesystem::BytesToString((*node)->size);
			} else {
				if (node == top) {
					Chart::Pie::Color(IM_COL32(72, 74, 78, 255), IM_COL32(55, 57, 62, 255));
				} else {
					Chart::Pie::Color(ImColor::HSV(hue, (1 - radius / 420) * 0.75f, 0.9f));
				}
			}
		} else {
			if (node == top) {
				Chart::Pie::Color(IM_COL32(72, 74, 78, 255), IM_COL32(55, 57, 62, 255));
			} else {
				Chart::Pie::Color(ImColor::HSV(hue, (1 - radius / 420) * 0.75f, 0.9f));
			}
		}

		Chart::Pie::Slice(radius * sliceScale / 512, start, end);
	}
	Chart::Pie::End();

	if (ImGui::IsMouseClicked(3)) {
		if (history.size() > 1) {
			history.pop();
			drawData = BuildDrawData(*history.top());
		} else {
			history = {};
			state = State::Started;
		}
	}

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
			Filesystem::OpenSystemPath(CurrentPath);
		}

		ImGui::EndPopup();
	}
	ImGui::Shadow::End();

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(3);

	ImGui::SetCursorPos({ cx - 6 * scale, cy - 6 * scale });
	ImGui::Image(icons[Icons::Back], { 12 * scale, 12 * scale });

	ImGui::GetWindowDrawList()->AddRectFilled({ 0, ImGui::GetWindowHeight() - 30 }, { ImGui::GetWindowWidth(), ImGui::GetWindowHeight() }, IM_COL32(55, 57, 62, 255));
	ImGui::GetWindowDrawList()->AddLine({ 0, ImGui::GetWindowHeight() - 30 }, { ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 30 }, IM_COL32(43, 45, 48, 255));

	ImGui::SetCursorPos({ 0 + 9, ImGui::GetWindowHeight() - 21 });
	ImGui::Image(icons[Icons::Folder], { 14, 13 });

	ImGui::SameLine();

	ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 24);

	string = root.u8string();
	text = std::string(string.begin(), string.end());

	ImGui::Text("%s", text.c_str());

	ImGui::PopStyleColor(1);
}

void Draw() {
	glfwGetWindowSize(window, &WindowWidth, &WindowHeight);

	ImGui::SetNextWindowPos({});
	ImGui::SetNextWindowSize({ (float)WindowWidth, (float)WindowHeight });

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2());

	ImGui::Begin("Scan My Disk", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

	// Window Background.
	{ ImGui::GetWindowDrawList()->AddRectFilled({}, ImGui::GetWindowSize(), IM_COL32(55, 57, 62, 255)); }

	const auto close = []() {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	};

	ImGui::SetCursorPos({ 0, 50 });

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
		default:;
	}

	// Window Title.
	{
		ImGui::SetCursorPos({});

		if (CustomWindowTitleEnabled()) {
			ImGui::GetWindowDrawList()->AddRectFilled({}, { ImGui::GetWindowWidth(), 30 }, IM_COL32(43, 45, 48, 255));
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(18, 9));

		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 32));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 32));

		if (ImGui::ImageButton("#Menu", icons[Icons::Menu], { 12, 12 })) {
			ImGui::OpenPopup("File");
		}

		if (CustomWindowTitleEnabled()) {
			ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 48 * 3, 0 });
			if (ImGui::ImageButton("#Minimize", icons[Icons::Minimize], { 12, 12 })) {
				glfwIconifyWindow(window);
			}

			if (glfwGetWindowAttrib(window, GLFW_MAXIMIZED)) {
				ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 48 * 2, 0 });
				if (ImGui::ImageButton("#Restore", icons[Icons::Restore], { 12, 12 })) {
					glfwRestoreWindow(window);
				}
			} else {
				ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 48 * 2, 0 });
				if (ImGui::ImageButton("#Maximize", icons[Icons::Maximize], { 12, 12 })) {
					glfwMaximizeWindow(window);
				}
			}

			{
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(196, 43, 23, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(196, 43, 23, 255));

				ImGui::SetCursorPos({ ImGui::GetWindowWidth() - 48, 0 });
				if (ImGui::ImageButton("#Close", icons[Icons::Close], { 12, 12 })) {
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
				ImGui::SetCursorPos({ ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(windowTitle).x / 2, 15 - ImGui::CalcTextSize(windowTitle).y / 2 });
				ImGui::Text("%s", windowTitle);
			}
			ImGui::PopStyleColor();
		}
	}

	ImGui::SetCursorPos({ 0, 50 });
	ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(55, 57, 62, 255));
	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(67, 69, 74, 255));
	ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);

	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 5));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(80, 5));

	ImGui::SetNextWindowPos({ 10, 30 });

	ImGui::Shadow::Begin();
	if (ImGui::BeginPopup("File")) {
		ImGui::Shadow::Position(ImGui::GetWindowPos());
		ImGui::Shadow::Size(ImGui::GetWindowSize());

		ImGui::NewLine();
		ImGui::SameLine(18.0f);
		if (ImGui::MenuItem(Localization::Text("Menu_OpenFolder_Button"))) {
			const auto folderPath = Filesystem::OpenSelectFolderDialog();
			if (!folderPath.empty() && std::filesystem::exists(folderPath)) {
				progress = 0;

				space = std::make_pair(0, 0);
				future = std::async(std::launch::async, [folderPath] {
					return Filesystem::ParallelBuildTree(folderPath, progress);
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

	if (ImGui::GetMousePos().x > WindowWidth - 10 && ImGui::GetMousePos().y > WindowHeight - 10) {
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNWSE);
	}

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(3);

	ImGui::End();

	ImGui::PopStyleVar();
}
