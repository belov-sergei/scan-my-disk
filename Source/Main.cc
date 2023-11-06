#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <Chart.h>
#include <Filesystem.h>

using SliceDrawData = std::vector<std::tuple<float, float, float, float, const Tree::Node<Filesystem::Entry>*>>;

SliceDrawData BuildDrawData(const Tree::Node<Filesystem::Entry>& node) {
	const size_t size = node->size;
	const size_t depth = node->depth;
	std::vector<float> depthStartAngle(1);

	SliceDrawData result;
	node.depthTraversal([&](const Tree::Node<Filesystem::Entry>& entry) {
		if (depthStartAngle.size() < entry->depth - depth + 2) {
			depthStartAngle.emplace_back();
		}

		float& start = depthStartAngle[entry->depth - depth];
		depthStartAngle[entry->depth - depth + 1] = start;

		const float normalized = entry->size / (float)size;
		if (normalized > 0.01) {
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

const auto space = Filesystem::GetDriveSpace("C://");

SliceDrawData drawData;
State state = State::Started;
std::atomic<size_t> progress = 0;

std::future<Tree::Node<Filesystem::Entry>> future;

Tree::Node<Filesystem::Entry> tree;

std::stack<const Tree::Node<Filesystem::Entry>*> history;

const auto x = 1024 / 2;
const auto y = 768 / 2;

std::string path;
size_t size = 0;

void StartedState() {
	ImGui::Text("Disk Chart");

	for (const auto& drive : Filesystem::GetLogicalDrives()) { 
		if (ImGui::Button(drive.c_str())) {
			future = std::async(std::launch::async, [drive] {
				return Filesystem::BuildTree(drive, progress);
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

void ChartState() {
	ImGui::Text("Path: %s", path.c_str());
	ImGui::Text("Size: %u Byte", size);

	const auto mx = ImGui::GetMousePos().x - x;
	const auto my = ImGui::GetMousePos().y - y;

	const float length = std::sqrt(mx * mx + my * my);
	float angle = ((int)(std::atan2(-my, -mx) * 180 / 3.14) + 180) / 360.0f;

	Chart::Pie::Begin({x, y});
	for (const auto& [radius, start, end, hue, node] : drawData | std::views::reverse) {
		if (length <= radius && length >= radius - 40 && angle >= start && angle <= end) {
			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
				auto* top = history.top();

				if (node == top && history.size() > 1) {
					history.pop();
					drawData = BuildDrawData(*history.top());
				}
				else {
					history.emplace(node);
					drawData = BuildDrawData(*node);
				}

				break;
			}
			Chart::Pie::Color(ImColor::HSV(hue, 0.25f, 1.0f));

			path = (*node)->path.string();
			size = (*node)->size;
		}
		else {
			Chart::Pie::Color(ImColor::HSV(hue, 1 - radius / 420, 1.0f));
		}

		Chart::Pie::Slice(radius, start, end);
	}
	Chart::Pie::End();
}

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);

	auto* window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_OPENGL);
	auto* context = SDL_GL_CreateContext(window);

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

		glClear(GL_COLOR_BUFFER_BIT);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Demo");

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

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
