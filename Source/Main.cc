#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <Chart.h>
#include <Filesystem.h>

using SliceDrawData = std::vector<std::tuple<float, float, float, float, const Tree::Node<Filesystem::Entry>*>>;

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
		if (depthStartAngle.size() < entry->depth - depth + 2) {
			depthStartAngle.emplace_back();
		}

		float& start = depthStartAngle[entry->depth - depth];
		depthStartAngle[entry->depth - depth + 1] = start;

		const float normalized = entry->size / (float)root;
		const float relative = entry->size / (float)size;
		if (relative > 0.01) {
			result.emplace_back(40 * (entry->depth - depth + 1), start, start + normalized, std::max(0.0f, 0.33f - normalized * 0.66f), &entry);
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

	ImGui::GetWindowDrawList()->AddRect({100, 100}, {924, 105}, ImColor(255, 0, 0, 255), 1.0f);
	ImGui::GetWindowDrawList()->AddRectFilled({100, 100}, {100 + 824 * scale, 105}, ImColor(255, 0, 0, 255));

	if (future.wait_for(0s) == std::future_status::ready) {
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
	ImGui::Text("Path: %s", path.c_str());
	ImGui::Text("Size: %s", size.c_str());

	auto [x, y] = ImGui::GetWindowSize();
	x *= 0.5f;
	y *= 0.5f;

	const auto mx = ImGui::GetMousePos().x - x;
	const auto my = ImGui::GetMousePos().y - y;

	const float length = std::sqrt(mx * mx + my * my);
	float angle = ((int)(std::atan2(-my, -mx) * 180 / 3.14) + 180) / 360.0f;

	Chart::Pie::Begin({x, y});
	for (const auto& [radius, start, end, hue, node] : drawData | std::views::reverse) {
		if (length <= radius && length >= radius - 40 && angle >= start && angle <= end) {
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				auto* top = history.top();

				if (node == top) {
					if (history.size() > 1) {
						history.pop();
						drawData = BuildDrawData(*history.top());
					}
				}
				else {
					history.emplace(node);
					drawData = BuildDrawData(*node);
				}

				break;
			}
			Chart::Pie::Color(ImColor::HSV(hue, 0.25f, 1.0f));

			path = (*node)->path.string();
			size = BytesToString((*node)->size);
		}
		else {
			Chart::Pie::Color(ImColor::HSV(hue, 1 - radius / 420, 1.0f));
		}

		Chart::Pie::Slice(radius, start, end);
	}
	Chart::Pie::End();
}

SDL_Window* window = nullptr;

void Draw() {
	glClear(GL_COLOR_BUFFER_BIT);

	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	ImGui::SetNextWindowPos({});
	ImGui::SetNextWindowSize({(float)w, (float)h});

	ImGui::Begin("Demo", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoSavedSettings);

	if (ImGui::BeginMenuBar()) {
		ImGui::Text("Disk Chart");
		ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 17);
		if (ImGui::Button("X")) {
			SDL_Event quit;
			quit.type = SDL_QUIT;

			SDL_PushEvent(&quit);
		}

		ImGui::EndMenuBar();
	}

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
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);

	window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS | SDL_WINDOW_RESIZABLE);
	auto* context = SDL_GL_CreateContext(window);

	SDL_SetWindowResizable(window, SDL_TRUE);
	SDL_AddEventWatch([](void* data, SDL_Event* event) {
		if (event->type == SDL_WINDOWEVENT) {
			if (event->window.event == SDL_WINDOWEVENT_RESIZED) {
				Draw();
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
