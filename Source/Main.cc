#include <imgui.h>
#include <imgui_impl_sdl2.h>
#include <imgui_impl_opengl3.h>
#include <SDL.h>
#include <SDL_opengl.h>

#include <Chart.h>
#include <Filesystem.h>

int main(int argc, char* argv[]) {
	SDL_Init(SDL_INIT_VIDEO);

	auto* window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1024, 768, SDL_WINDOW_OPENGL);
	auto* context = SDL_GL_CreateContext(window);

	ImGui::CreateContext();

	ImGui_ImplSDL2_InitForOpenGL(window, context);
	ImGui_ImplOpenGL3_Init("#version 150");

	const auto filesystemTree = Filesystem::BuildTree("C://Projects//Disk Usage");

	std::vector<std::tuple<float, float, float, float>> sliceDrawData;

	std::vector<float> depthStartAngle(1);

	size_t size = filesystemTree->size;
	filesystemTree.depthTraversal([&](const Tree::Node<Filesystem::Entry>& entry) {
		if (depthStartAngle.size() < entry->depth + 2) {
			depthStartAngle.emplace_back();
		}

		float& start = depthStartAngle[entry->depth];
		depthStartAngle[entry->depth + 1] = start;

		const float normalized = entry->size / (float)size;
		if (normalized > 0.01) {
			sliceDrawData.emplace_back(40 * (entry->depth + 1), start, start + normalized, std::max(0.0f, 0.33f - normalized * 0.66f));
		}

		start += normalized;

		return false;
	});

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

		const auto x = 1024 / 2;
		const auto y = 768 / 2;

		const auto mx = ImGui::GetMousePos().x - x;
		const auto my = ImGui::GetMousePos().y - y;

		const float length = std::sqrt(mx * mx + my * my);
		float angle = ((int)(std::atan2(-my, -mx) * 180 / 3.14) + 180) / 360.0f;

		Chart::Pie::Begin({x, y});
		for (const auto& [radius, start, end, hue] : sliceDrawData | std::views::reverse) {
			if (length <= radius && length >= radius - 40 && angle >= start && angle <= end) {
				Chart::Pie::Color(ImColor::HSV(hue, 0.25f, 1.0f));
			}
			else {
				Chart::Pie::Color(ImColor::HSV(hue, 1 - radius / 420, 1.0f));
			}

			Chart::Pie::Slice(radius, start, end);
		}
		Chart::Pie::End();

		ImGui::End();
		// ImGui::ShowDemoWindow();

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
