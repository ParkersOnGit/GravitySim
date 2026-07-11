#include <vector>
#include <iostream>

#include <SDL3/SDL.h>

struct Vector2f { 
	float x, y; 
	Vector2f operator+=(const Vector2f &vector) { return Vector2f(this->x + vector.x, this->y + vector.y); }
	Vector2f operator*(const float value) { return Vector2f(this->x * value, this->y * value); }
};

struct Body {
	Vector2f position;
	float mass;
	float radius;

	Vector2f velocity;

	void update(std::vector<Body> bodies) {
		for (Body &body : bodies) {
			if (&body == this) continue;
		}
	}

	void move(float deltaTime) {
		position += velocity * deltaTime;
	}
	
	void render(SDL_Renderer* renderer) {
		const int resolution = 16;

		SDL_Vertex vertices[resolution + 1];
		vertices[0].position = SDL_FPoint{position.x, position.y};
		vertices[0].color = SDL_FColor{ 1.0f, 1.0f, 1.0f, 1.0f };

		int indices[resolution * 3];

		for (int i = 1; i <= resolution; i++) {
			float angle = i * 2.0f * 3.14159f / resolution;

			vertices[i].position = SDL_FPoint{ position.x + (float)cos(angle) * radius, position.y + (float)sin(angle) * radius };
			vertices[i].color = SDL_FColor{ 0.85f, 0.85f, 0.85f, 1.0f };

			indices[(i - 1) * 3 + 0] = 0;
			indices[(i - 1) * 3 + 1] = i;
			indices[(i - 1) * 3 + 2] = (i % resolution) + 1;
		}

		SDL_RenderGeometry(renderer, NULL, vertices, resolution + 1, indices, resolution * 3);
	}
};

int main(int argc, char* argv[]) {
	// Initialize SDL.
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Failed to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	// Create window.
	SDL_Window* window = SDL_CreateWindow("2D Gravity Simulator", 1440, 900, NULL);
	if (!window) {
		SDL_Log("Failed to create window: %s\n", SDL_GetError());
		SDL_Quit();
		return 1;
	}

	// Create renderer.
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	if (!renderer) {
		SDL_Log("Failed to create renderer: %s\n", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		return 1;
	}

	// Create a vector of all bodies.
	std::vector<Body> bodies;

	// Create debug objects.
	Body debbie = {
		{ 500, 450 },
		50.0f,
		10.0f,
	};
	Body debrah = {
		{ 800, 350 },
		50.0f,
		20.0f,
	};

	debbie.velocity = Vector2f(1000, 1000);

	bodies.push_back(debbie);
	bodies.push_back(debrah);

	// Useful variables.
	Uint64 prevTime = SDL_GetPerformanceCounter();
	Uint64 currentTime = 0;
	float deltaTime = 0.0f;

	bool close = false;
	SDL_Event e;

	// Main loop.
	while (!close) {
		// Calculate deltaTime.
		currentTime = SDL_GetPerformanceCounter();
		deltaTime = (float)(currentTime - prevTime) / (float)SDL_GetPerformanceFrequency();
		prevTime = currentTime;

		// Get events.
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT) close = true;
		}

		// Update the bodies.
		for (Body body : bodies) body.update(bodies);

		// Move the bodies.
		for (Body body : bodies) body.move(deltaTime);

		// Render stuff.
		SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255);
		SDL_RenderClear(renderer);

		for (Body body : bodies) body.render(renderer);

		SDL_RenderPresent(renderer);
	}
	
	// Cleanup.
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}