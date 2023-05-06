#include <SDL.h>

int main(int argc, char* argv[])
{
	SDL_Init(SDL_INIT_VIDEO);

	auto* window = SDL_CreateWindow("Sample", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL);
	auto* context = SDL_GL_CreateContext(window);

	bool exit = false;
	while (!exit)
	{
		SDL_Event sdlEvent;
		while (SDL_PollEvent(&sdlEvent))
		{
			if (sdlEvent.type == SDL_QUIT)
			{
				exit = true;
			}
		}
	}

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}