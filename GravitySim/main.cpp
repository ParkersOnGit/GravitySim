#include <SDL3/SDL.h>

int main(int argc, char* argv[]) {
	// Initialize SDL.
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Failed to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	return 0;
}