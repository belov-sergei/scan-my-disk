#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <Chart.h>
#include <Filesystem.h>
#include <imgui_internal.h>
#include <Localization.h>

using SliceDrawData = std::vector<std::tuple<float, float, float, float, const Tree::Node<Filesystem::Entry>*>>;

int w = 512, h = 544;

enum Icons {
	Close,
	Maximize,
	Menu,
	Minimize,
	Restore,
	Icon,

	Last
};

std::array<void*, Icons::Last> icons;

void LoadTexture(std::string_view path, void*& textureId) {
	int width, height;
	if (auto* pixels = stbi_load(path.data(), &width, &height, nullptr, 4)) {
		glGenTextures(1, (GLuint*)&textureId);
		glBindTexture(GL_TEXTURE_2D, (GLuint)textureId);

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
			result.emplace_back(32 * (entry->depth - depth + 1), start, start + normalized, std::max(0.0f, 0.33f - normalized * 0.66f), &entry);
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

std::pair<size_t, size_t> space = {};

SliceDrawData drawData;
State state = State::Started;
std::atomic<size_t> progress = 0;

std::future<Tree::Node<Filesystem::Entry>> future;

Tree::Node<Filesystem::Entry> tree;

std::stack<const Tree::Node<Filesystem::Entry>*> history;

std::string path;
std::string size;

void StartedState() {
	for (const auto& drive : Filesystem::GetLogicalDrives()) {
		if (ImGui::Button(drive.c_str())) {
			space = Filesystem::GetDriveSpace(drive);

			progress = 0;
			future = std::async(std::launch::async, [drive] {
				return Filesystem::ParallelBuildTree(drive, progress);
			});

			state = State::Loading;
			break;
		}
	}
}

void LoadingState() {
	using namespace std::chrono_literals;

	const float scale = std::min<float>(progress / float(space.first - space.second), 1.0f);

	const float padding = w * 0.1;
	ImGui::GetWindowDrawList()->AddRect({padding, 100}, {(float)w - padding, 105}, ImColor(255, 0, 0, 255), 1.0f);
	ImGui::GetWindowDrawList()->AddRectFilled({padding, 100}, {padding + (w - padding * 2) * scale, 105}, ImColor(255, 0, 0, 255), 1.0f);

	if (ImGui::Button(Localization::Text("ABORT_LOADING_BUTTON"))) {
		Filesystem::CancelBuildTree();
		std::ignore = future.get();

		state = State::Started;
	}

	if (future.valid() && future.wait_for(0s) == std::future_status::ready) {
		tree = future.get();
		tree->size = space.first;

		drawData = BuildDrawData(tree);

		history.emplace(&tree);
		state = State::Chart;
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

	return std::format("{:.2f} {}", size, units[unit]);
}

void ChartState() {
	ImGui::TextWrapped("Path: %s", path.c_str());
	ImGui::Text("Size: %s", size.c_str());

	std::filesystem::path root = (*history.top())->path;
	if (history.size() > 1) {
		root = std::filesystem::relative(root, root.parent_path());
	}

	ImGui::Text("%s", root.string().c_str());

	auto [x, y] = ImGui::GetWindowSize();
	x *= 0.5f;
	y = y * 0.5f + 18;

	const auto mx = ImGui::GetMousePos().x - x;
	const auto my = ImGui::GetMousePos().y - y;

	const float length = std::sqrt(mx * mx + my * my);
	float angle = ((int)(std::atan2(-my, -mx) * 180 / 3.14) + 180) / 360.0f;

	if (ImGui::BeginPopupContextItem("Menu")) {
		if (ImGui::Selectable(Localization::Text("EXPLORE_BUTTON"))) {
			Filesystem::Explore(path);
		}
		ImGui::EndPopup();
	}

	Chart::Pie::Begin({x, y});
	for (const auto& [radius, start, end, hue, node] : drawData | std::views::reverse) {
		if (!ImGui::IsPopupOpen("Menu")) {
			if (length <= (radius * w / 512) && length >= ((radius - 32) * w / 512) && angle >= start && angle <= end) {
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
					auto* top = history.top();

					if (node == top) {
						if (history.size() > 1) {
							history.pop();
							drawData = BuildDrawData(*history.top());
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

				Chart::Pie::Color(ImColor::HSV(hue, 0.25f, 1.0f));

				path = (*node)->path.string();
				size = BytesToString((*node)->size);
			}
			else {
				Chart::Pie::Color(ImColor::HSV(hue, 1 - radius / 420, 1.0f));
			}
		}
		else {
			Chart::Pie::Color(ImColor::HSV(hue, 1 - radius / 420, 1.0f));
		}

		Chart::Pie::Slice(radius * w / 512, start, end);
	}
	Chart::Pie::End();
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

	ImGui::Begin("Demo", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoSavedSettings);

	// Window Background.
	{
		ImGui::GetWindowDrawList()->AddRectFilled({}, ImGui::GetWindowSize(), IM_COL32(55, 57, 62, 255));
		ImGui::GetWindowDrawList()->AddRectFilled({}, {ImGui::GetWindowWidth(), 30}, IM_COL32(43, 45, 48, 255));
	}

	const auto close = []() {
		if (state == State::Loading) {
			Filesystem::CancelBuildTree();
			std::ignore = future.get();
		}

		SDL_Event quit;
		quit.type = SDL_QUIT;

		SDL_PushEvent(&quit);
	};

	// Window Title.
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(18, 9));

		ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(255, 255, 255, 32));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 255, 255, 32));

		if (ImGui::ImageButton("#Menu", icons[Icons::Menu], {12, 12})) {
			ImGui::OpenPopup("File");
		}

		ImGui::SameLine(ImGui::GetWindowWidth() - 48 * 3);
		if (ImGui::ImageButton("#Minimize", icons[Icons::Minimize], {12, 12})) {
			SDL_MinimizeWindow(window);
		}

		if (SDL_GetWindowFlags(window) & SDL_WINDOW_MAXIMIZED) {
			ImGui::SameLine(ImGui::GetWindowWidth() - 48 * 2);
			if (ImGui::ImageButton("#Restore", icons[Icons::Restore], {12, 12})) {
				SDL_RestoreWindow(window);
			}
		}
		else {
			ImGui::SameLine(ImGui::GetWindowWidth() - 48 * 2);
			if (ImGui::ImageButton("#Maximize", icons[Icons::Maximize], {12, 12})) {
				SDL_MaximizeWindow(window);
			}
		}

		{
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(196, 43, 23, 255));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(196, 43, 23, 255));

			ImGui::SameLine(ImGui::GetWindowWidth() - 48);
			if (ImGui::ImageButton("#Close", icons[Icons::Close], {12, 12})) {
				close();
			}

			ImGui::PopStyleColor(2);
		}

		ImGui::PopStyleColor(3);
		ImGui::PopStyleVar();

		const auto* windowTitle = "Disk Chart";
		ImGui::SetCursorPos({ImGui::GetWindowWidth() / 2 - ImGui::CalcTextSize(windowTitle).x / 2, 15 - ImGui::CalcTextSize(windowTitle).y / 2});
		ImGui::Text(windowTitle);
	}

	ImGui::SetCursorPos({0, 50});
	ImGui::PushStyleColor(ImGuiCol_PopupBg, IM_COL32(55, 57, 62, 255));
	ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(67, 69, 74, 255));

	ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 5.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 5));
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(80, 5));

	ImGui::SetNextWindowPos({10, 30});
	if (ImGui::BeginPopup("File")) {
		ImGui::NewLine();
		ImGui::SameLine(18.0f);
		if (ImGui::BeginMenu("Language")) {
			using namespace Localization;

			for (const auto language : {Language::English, Language::French, Language::Spanish, Language::Chinese, Language::Russian}) {
				ImGui::NewLine();
				ImGui::SameLine(18.0f);
				if (ImGui::MenuItem(Text(language))) {
					Text::SetLanguage(language);
				}
			}

			ImGui::EndMenu();
		}

		ImGui::Separator();

		ImGui::NewLine();
		ImGui::SameLine(18.0f);
		if (ImGui::MenuItem("Exit")) {
			close();
		}

		ImGui::EndPopup();
	}

	ImGui::PopStyleColor(2);
	ImGui::PopStyleVar(3);

	ImGui::Image(icons[Icons::Icon], {20, 20});
	ImGui::SameLine();

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

	ImGui::End();

	ImGui::PopStyleVar();

	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w, h, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	auto* context = SDL_GL_CreateContext(window);

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

	SDL_SetWindowHitTest(
		window, [](auto* window, const auto area, auto*) {
			int w, h;
			SDL_GetWindowSize(window, &w, &h);

			if (area->x < w - 17 && area->y < 20) {
				return SDL_HITTEST_DRAGGABLE;
			}
			else if (area->x > w - 10 && area->y > h - 10) {
				return SDL_HITTEST_RESIZE_BOTTOMRIGHT;
			}

			return SDL_HITTEST_NORMAL;
		},
		nullptr);

	ImGui::CreateContext();

	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 150");

	ImFontConfig config;
	ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansSC-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());

	config.MergeMode = true;
	ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/NotoSansSC-Regular.ttf", 18.0f, &config, ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());

	LoadTexture("Icons/Close.png", icons[Icons::Close]);
	LoadTexture("Icons/Maximize.png", icons[Icons::Maximize]);
	LoadTexture("Icons/Menu.png", icons[Icons::Menu]);
	LoadTexture("Icons/Minimize.png", icons[Icons::Minimize]);
	LoadTexture("Icons/Restore.png", icons[Icons::Restore]);
	LoadTexture("Icons/Icon.png", icons[Icons::Icon]);

	bool exit = false;
	while (!exit) {
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent)) {
			ImGui_ImplSDL2_ProcessEvent(&sdlEvent);

			if (sdlEvent.type == SDL_QUIT) {
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
