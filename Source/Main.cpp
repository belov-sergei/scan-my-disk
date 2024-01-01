// Copyright ❤️ 2023-2024, Sergei Belov

#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#if defined(WINDOWS)
#include <dwmapi.h>
#endif

#include <Chart.h>
#include <Filesystem.h>
#include <imgui_internal.h>
#include <Localization.h>

#include "Settings.h"

using SliceDrawData = std::vector<std::tuple<float, float, float, float, const Tree::Node<Filesystem::Entry>*>>;

int w = 440, h = 540;

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

std::array<GLuint, Icons::Last> icons;

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
		} // namespace Details

		void Begin() {
			using namespace Details;

			if (State.empty()) {
				DrawList = ImGui::GetWindowDrawList();
			}

			auto& [index, position, size] = State.emplace();

			Index = &index;
			Position = &position;
			Size = &size;

			*Index = DrawList->VtxBuffer.Size;

			AddImage(UV[0], UV[0], UV[1], UV[1]);
			AddImage(UV[1], UV[0], UV[2], UV[1]);
			AddImage(UV[2], UV[0], UV[3], UV[1]);

			AddImage(UV[0], UV[1], UV[1], UV[2]);
			AddImage(UV[1], UV[1], UV[2], UV[2]);
			AddImage(UV[2], UV[1], UV[3], UV[2]);

			AddImage(UV[0], UV[2], UV[1], UV[3]);
			AddImage(UV[1], UV[2], UV[2], UV[3]);
			AddImage(UV[2], UV[2], UV[3], UV[3]);
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
	} // namespace Shadow
} // namespace ImGui

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

enum class State {
	Started,
	Loading,
	Chart
};

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

std::pair<size_t, size_t> space = {};

SliceDrawData drawData;
State state = State::Started;
std::atomic<size_t> progress = 0;

std::future<Tree::Node<Filesystem::Entry>> future;

Tree::Node<Filesystem::Entry> tree;

std::stack<const Tree::Node<Filesystem::Entry>*> history;

std::filesystem::path path;
std::string size;

void StartedState() {
	ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);

	ImGui::Indent(30);
	ImGui::Text(Localization::Text("StartedState_SelectDisk_Text"));

	{
		const auto& [x, y] = ImGui::GetCursorPos();
		ImGui::GetWindowDrawList()->AddLine({x, y}, {x + ImGui::GetWindowWidth() - 60, y + 1}, IM_COL32(190, 190, 190, 255));
	}

	float buttonWidth = 165.0f;

	const auto drives = Filesystem::GetLogicalDrives();
	for (const auto& drive : drives) {
		const auto [bytesTotal, bytesFree] = Filesystem::GetDriveSpace(drive);

		const auto bytesFreeText = BytesToString(bytesFree);
		const auto bytesTotalText = BytesToString(bytesTotal);

		const auto textSize = ImGui::CalcTextSize(fmt::vformat((std::string_view)Localization::Text("StartedState_FreeSpace_Text"), fmt::make_format_args(bytesFreeText, bytesTotalText)).c_str());
		buttonWidth = std::max(buttonWidth, textSize.x + 3.0f);
	}

	int buttonsInRow = 0;

	ImGui::NewLine();
	for (const auto& drive : drives) {
		const auto [bytesTotal, bytesFree] = Filesystem::GetDriveSpace(drive);

		ImGui::BeginGroup();
		{
			ImGui::Text(drive.c_str());

			const float scale = 1.0f - bytesFree / (float)bytesTotal;

			const auto [x, y] = ImGui::GetCursorPos();

			if (ImRect(x - 5, y - 25, x + buttonWidth, y + 35).Contains(ImGui::GetMousePos())) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					space = Filesystem::GetDriveSpace(drive);

					progress = 0;
					future = std::async(std::launch::async, [drive] {
						return Filesystem::ParallelBuildTree(
							#if defined(MACOS)
							"/Volumes/" + 
							#endif
							drive, progress
							);
					});

					state = State::Loading;
				}

				ImGui::GetWindowDrawList()->AddRectFilled({x - 5, y - 25}, {x + buttonWidth, y + 35}, IM_COL32(190, 190, 190, 32), 8);
			}

			ImGui::ItemSize({x, y, x + (buttonWidth + 5.0f), y + 8});

			ImGui::GetWindowDrawList()->AddRectFilled({x, y}, {x + (buttonWidth - 5.0f), y + 8}, IM_COL32(190, 190, 190, 127));
			ImGui::GetWindowDrawList()->AddRectFilled({x, y}, {x + (buttonWidth - 5.0f) * scale, y + 8}, IM_COL32(190, 190, 190, 255));

			const auto bytesFreeText = BytesToString(bytesFree);
			const auto bytesTotalText = BytesToString(bytesTotal);
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
		tree->size = space.first;

		drawData = BuildDrawData(tree);

		history.emplace(&tree);
		state = State::Chart;
	}
}

void ChartState() {
	ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);

	ImGui::Indent(30);

	ImGui::PushTextWrapPos(ImGui::GetWindowWidth() - 60);

	std::u8string string = path.u8string();
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

	const auto mx = ImGui::GetMousePos().x - x;
	const auto my = ImGui::GetMousePos().y - y;

	const float length = std::sqrt(mx * mx + my * my);
	float angle = ((int)(std::atan2(-my, -mx) * 180 / 3.14) + 180) / 360.0f;

	const float scale = std::min(w, h - 120);

	Chart::Pie::Begin({x, y});
	for (const auto& [radius, start, end, hue, node] : drawData | std::views::reverse) {
		auto* top = history.top();

		if (!ImGui::IsPopupOpen("Menu") && !ImGui::IsPopupOpen("File")) {
			if (length <= (radius * scale / 512) && length >= ((radius - 32) * scale / 512) && angle >= start && angle <= end) {
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

		Chart::Pie::Slice(radius * scale / 512, start, end);
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
			Filesystem::OpenPath(path);
		}

		ImGui::EndPopup();
	}
	ImGui::Shadow::End();

	ImGui::PopStyleColor(3);
	ImGui::PopStyleVar(3);

	ImGui::SetCursorPos({x - 6, y - 6});
	ImGui::Image((void*)icons[Icons::Back], {12, 12});

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

SDL_Window* window = nullptr;

void Draw() {
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

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
		default:;
	}

	// Window Title.
	{
		ImGui::SetCursorPos({});

		#if defined(WINDOWS)
		ImGui::GetWindowDrawList()->AddRectFilled({}, {ImGui::GetWindowWidth(), 30}, IM_COL32(43, 45, 48, 255));
		#endif

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(18, 9));

		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 32));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 32));

		if (ImGui::ImageButton("#Menu", (void*)icons[Icons::Menu], {12, 12})) {
			ImGui::OpenPopup("File");
		}

		#if defined(WINDOWS)
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
		#endif

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		#if defined(WINDOWS)
		ImGui::PushStyleColor(ImGuiCol_Text, Settings<Color>::Text);
		{
			const auto* windowTitle = "Scan My Disk";
			ImGui::SetCursorPos({ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(windowTitle).x / 2, 15 - ImGui::CalcTextSize(windowTitle).y / 2});
			ImGui::Text(windowTitle);
		}
		ImGui::PopStyleColor();
		#endif
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

	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}

void LocalizationInitialization() {
	pugi::xml_document xml;
	xml.load_file("Translations.xml");

	const auto root = xml.document_element();

	for (const auto language : root) {
		Localization::Language(language.name());

		for (const auto text : language) {
			const auto textId = text.attribute("Id").as_string();
			const auto value = text.attribute("Value").as_string();

			Localization::Text::Add(textId, value);
		}
	}

	Localization::Language(Settings<User>::Language);
}

#if defined(WINDOWS)
WNDPROC SDLWndProc;

LRESULT CALLBACK MyWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_NCCALCSIZE:
			if (wParam == TRUE) {
				return 0;
			}
			break;
	}

	return CallWindowProc(SDLWndProc, hwnd, msg, wParam, lParam);
}
#endif

int main(int argc, char* argv[]) {
	if (!std::filesystem::exists("README.md")) {
		std::filesystem::current_path(SDL_GetBasePath());
	}

	Settings<>::Load();

	LocalizationInitialization();

	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("Scan My Disk", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	auto* context = SDL_GL_CreateContext(window);

	#if defined(WINDOWS)
	MARGINS margins = {-1};
	DwmExtendFrameIntoClientArea(GetActiveWindow(), &margins);

	SDLWndProc = (WNDPROC)SetWindowLongPtr(GetActiveWindow(), GWLP_WNDPROC, (LONG_PTR)MyWndProc);
	SetWindowPos(GetActiveWindow(), NULL, 0, 0, w, h, SWP_FRAMECHANGED | SWP_NOMOVE);
	#endif

	SDL_AddEventWatch([](void* data, SDL_Event* event) {
			if (event->type == SDL_WINDOWEVENT) {
				// To prevent recursive calls to Draw function when the user clicks on Maximize and Minimize.
				if (event->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
					if (!ImGui::GetCurrentContext()->WithinFrameScope) {
						Draw();
					}
				}
			}
			return 0;
		},
		window);

	#if defined(WINDOWS)
	SDL_SetWindowHitTest(
		window,
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
			}
			else if (area->x > w - 10 && area->y > h - 10) {
				return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
			}

			return SDL_HITTEST_NORMAL;
		},
		nullptr);
	#endif

	ImGui::CreateContext();

	ImGui::GetIO().IniFilename = nullptr;
	ImGui::GetIO().LogFilename = nullptr;

	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 120");

	ImFontConfig config;
	ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSans-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	config.MergeMode = true;
	ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansSC-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
	ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansJP-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesJapanese());
	ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansKR-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesKorean());

	LoadTexture("Icons/Close.png", icons[Icons::Close]);
	LoadTexture("Icons/Maximize.png", icons[Icons::Maximize]);
	LoadTexture("Icons/Menu.png", icons[Icons::Menu]);
	LoadTexture("Icons/Minimize.png", icons[Icons::Minimize]);
	LoadTexture("Icons/Restore.png", icons[Icons::Restore]);
	LoadTexture("Icons/Icon.png", icons[Icons::Icon]);
	LoadTexture("Icons/Shadow.png", icons[Icons::Shadow]);
	LoadTexture("Icons/Back.png", icons[Icons::Back]);
	LoadTexture("Icons/Folder.png", icons[Icons::Folder]);

	bool exit = false;
	while (!exit) {
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent)) {
			ImGui_ImplSDL2_ProcessEvent(&sdlEvent);

			if (sdlEvent.type == SDL_QUIT) {
				if (state == State::Loading) {
					Filesystem::CancelBuildTree();
					std::ignore = future.get();
				}

				exit = true;
			}
		}

		Draw();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
