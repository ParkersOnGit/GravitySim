#include <vector>
#include <iostream>

#include <SDL3/SDL.h>

struct Vector2f { 
	float x, y; 
};

struct Body {
	Vector2f position; // Meters
	float mass; // Kilograms
	float radius; // Meters
	Vector2f velocity; // Meters

	void update(const std::vector<Body*> &bodies, float deltaTime) {
		for (Body* body : bodies) {
			if (body == this) continue;

			// Using distance formula get distance squared.
			float deltaX = body->position.x - position.x;
			float deltaY = body->position.y - position.y;
			float distance = sqrt(deltaX * deltaX + deltaY * deltaY);

			// Make sure distances isn't 0.
			if (distance < 0.1f) continue;

			// Get the gravitational force using Newton's gravity equation (Minus the gravitational constant).
			float gravitationalForce = (body->mass * mass / (distance * distance)) * 10.0f; // Newtons

			// Normalize vector towards object.
			float directionX = deltaX / distance;
			float directionY = deltaY / distance;

			// Get the delta position to point towards the object.
			velocity.x += directionX * gravitationalForce / mass * deltaTime;
			velocity.y += directionY * gravitationalForce / mass * deltaTime;
		}
	}

	void move(float deltaTime) {
		position.x += velocity.x * deltaTime;
		position.y += velocity.y * deltaTime;
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

		// Draw velocity vector.
		SDL_SetRenderDrawColor(renderer, 255, 55, 35, 255);
		SDL_RenderLine(renderer, position.x, position.y, position.x + velocity.x, position.y + velocity.y);
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
	std::vector<Body*> bodies;

	/*for (int i = 0; i < 50; i++) {
		Body newBody = {
			{ SDL_rand(500), SDL_rand(500)},
			SDL_rand(50) + 25,
			SDL_rand(50) + 1
		};
		bodies.push_back(&newBody);
	}*/

	// Create debug objects.
	Body debbie = {
		{ 500, 550 },
		5.0f,
		10.0f,
	};
	Body debrah = {
		{ 800, 350 },
		25000.0f,
		120.0f,
	};

	debbie.velocity = Vector2f(10, 0);
	bodies.push_back(&debrah);

	bodies.push_back(&debbie);

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
		for (Body* body : bodies) body->update(bodies, deltaTime);

		// Move the bodies.
		for (Body* body : bodies) body->move(deltaTime);

		// Render stuff.
		SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255);
		SDL_RenderClear(renderer);

		for (Body* body : bodies) body->render(renderer);

		SDL_RenderPresent(renderer);
	}
	
	// Cleanup.
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}