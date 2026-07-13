#include <vector>
#include <string>
#include <iostream>

#include <SDL3/SDL.h>

struct Vector2f { 
	float x, y; 
};

struct Camera {
	Vector2f position = Vector2f(0.0f, 0.0f);
	float zoom = 1.0f; // Not yet implemented. Do later!
};

struct Body {
	Vector2f position; // Meters
	float mass; // Kilograms
	float radius; // Meters
	Vector2f velocity; // Meters

	void update(const std::vector<Body> &bodies, float deltaTime) {
		for (const Body &body : bodies) {
			if (&body == this) continue;

			// Using distance formula get distance.
			float deltaX = body.position.x - position.x;
			float deltaY = body.position.y - position.y;
			float distance = sqrt(deltaX * deltaX + deltaY * deltaY);

			// Make sure distances isn't 0.
			if (distance < 0.1f) continue;

			// Get the gravitational force using Newton's gravity equation (Minus the gravitational constant).
			float gravitationalForce = (body.mass * mass / (distance * distance)) * 150.0f; // Newtons

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

	void collisionCheck(std::vector<Body> &bodies) {
		for (Body& body : bodies) {
			if (&body == this) continue;

			// Using distance formula get distance.
			float deltaX = body.position.x - position.x;
			float deltaY = body.position.y - position.y;
			float distance = sqrt(deltaX * deltaX + deltaY * deltaY);

			if (distance < radius + body.radius) {
				// Divide by zero check.
				if (distance < 0.001f) distance = 0.01f;

				// Calculate the normal vector.
				float normalDeltaX = deltaX / distance;
				float normalDeltaY = deltaY / distance;

				// Get the overlap ammount.
				float depth = (radius + body.radius) - distance;

				// Now move the circles out of eachother.
				position.x += -normalDeltaX * depth / mass;
				position.y += -normalDeltaY * depth / mass;

				// Get the relative velocity.
				float relativeVelocityX = body.velocity.x - velocity.x;
				float relativeVelocityY = body.velocity.y - velocity.y;

				// Velocity along the normal using dot product.
				float velocityNormal = relativeVelocityX * normalDeltaX + relativeVelocityY * normalDeltaY;

				// If objects moving away, continue.
				if (velocityNormal > 0) continue;

				float bounciness = 0.7f;
				float push = (1.0f + bounciness) * -velocityNormal / (1 / mass + 1 / body.mass);

				// Now change the velocities.
				velocity.x -= push * normalDeltaX / mass;
				velocity.y -= push * normalDeltaY / mass;

				body.velocity.x += push * normalDeltaX / body.mass;
				body.velocity.y += push * normalDeltaY / body.mass;
			}
		}
	}
	
	

	void render(SDL_Renderer* renderer, const Camera &camera) {
		const int resolution = 16;
		SDL_FColor massColor = { 1.0f, 1.0f, 1.0f, 1.0f };

		std::vector<float> colorPosition = {
			0.0f, 0.25f, 0.5f, 0.75f, 1.0f
		};

		std::vector<SDL_FColor> massColors = {
			{0.255f, 0.341f, 0.910f, 1.0f}, // Blue
			{0.596f, 0.871f, 0.322f, 1.0f}, // Green
			{0.941f, 0.918f, 0.263f, 1.0f}, // Yellow
			{0.941f, 0.682f, 0.263f, 1.0f}, // Orange
			{0.878f, 0.157f, 0.157f, 1.0f}  // Red
		};

		float t = SDL_clamp(log10(mass + 1.0f) / 5.0f, 0.0f, 1.0f);
		
		for (int i = 0; i < colorPosition.size() - 1; i++) {
			// If is inbetween.
			if (t >= colorPosition[i] && t <= colorPosition[i + 1]) {
				// Normalize the t value for the linear interpolation.
				float normalizedT = (t - colorPosition[i]) / (colorPosition[i + 1] - colorPosition[i]);
				
				// Now lerp the color.
				massColor.r = massColors[i].r + (massColors[i + 1].r - massColors[i].r) * normalizedT;
				massColor.g = massColors[i].g + (massColors[i + 1].g - massColors[i].g) * normalizedT;
				massColor.b = massColors[i].b + (massColors[i + 1].b - massColors[i].b) * normalizedT;

				break;
			}
		}

		SDL_Vertex vertices[resolution + 1];
		vertices[0].position = SDL_FPoint{(position.x - camera.position.x) * camera.zoom, (position.y + camera.position.y) * camera.zoom };
		vertices[0].color = massColor;

		int indices[resolution * 3];

		for (int i = 1; i <= resolution; i++) {
			float angle = i * 2.0f * 3.14159f / resolution;

			vertices[i].position = SDL_FPoint{ (position.x + (float)cos(angle) * radius - camera.position.x) * camera.zoom, (position.y + (float)sin(angle) * radius + camera.position.y) * camera.zoom };
			vertices[i].color = SDL_FColor{ massColor.r * 0.85f, massColor.g * 0.85f, massColor.b * 0.85f, 1.0f };

			indices[(i - 1) * 3 + 0] = 0;
			indices[(i - 1) * 3 + 1] = i;
			indices[(i - 1) * 3 + 2] = (i % resolution) + 1;
		}

		SDL_RenderGeometry(renderer, NULL, vertices, resolution + 1, indices, resolution * 3);

		// Draw velocity vector.
		SDL_SetRenderDrawColor(renderer, 255, 55, 35, 255);
		SDL_RenderLine(renderer, (position.x - camera.position.x) * camera.zoom, (position.y + camera.position.y) * camera.zoom, (position.x + velocity.x - camera.position.x) * camera.zoom, (position.y + velocity.y + camera.position.y) * camera.zoom);
	}
};

int main(int argc, char* argv[]) {
	// Initialize SDL.
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		SDL_Log("Failed to initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	// Create window.
	SDL_Window* window = SDL_CreateWindow("2D Gravity Simulator", 1440, 900, SDL_WINDOW_RESIZABLE);
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

	// Get width and height of screen.
	int w, h;
	SDL_GetWindowSize(window, &w, &h);

	// Create camera.
	Camera camera;

	// Creation mode body.
	Body creationBody;
	int creationSelection = 0;

	// Create a vector of all bodies.
	std::vector<Body> bodies;

	for (int i = 0; i < 120; i++) {
		Body newBody = {
			{ SDL_rand(w), SDL_rand(h)},
			SDL_rand(5000) / 40.0f + 1,
			SDL_rand(25) + 5,
			{ SDL_rand(50) - 25, SDL_rand(50) - 25}
		};
		bodies.push_back(newBody);
	}

	// Create debug objects.
	Body debrah = {
		{ 800, 350 },
		25000.0f,
		120.0f,
	};
	//bodies.push_back(debrah);

	// Useful variables.
	Uint64 prevTime = SDL_GetPerformanceCounter();
	Uint64 currentTime = 0;
	float deltaTime = 0.0f;

	bool close = false;
	bool runSimulation = true;
	bool createMode = false;
	SDL_Event e;

	// Main loop.
	while (!close) {
		// Get width and height of screen.
		SDL_GetWindowSize(window, &w, &h);

		// Calculate deltaTime.
		currentTime = SDL_GetPerformanceCounter();
		deltaTime = (float)(currentTime - prevTime) / (float)SDL_GetPerformanceFrequency();
		prevTime = currentTime;

		// Prevent deltaTime from being too large.
		if (deltaTime > 0.01f) deltaTime = 0.001f;

		// Get events.
		while (SDL_PollEvent(&e)) {
			if (e.type == SDL_EVENT_QUIT) close = true;
			if (e.type == SDL_EVENT_KEY_DOWN) {
				if (e.key.key == SDLK_W) {
					if (!createMode) camera.position.y += 7.5f / camera.zoom;
					else if (creationSelection > 0) creationSelection--;
				}
				if (e.key.key == SDLK_A) {
					if (!createMode) camera.position.x -= 7.5f / camera.zoom;
					else {

					}
				}
				if (e.key.key == SDLK_S) {
					if (!createMode) camera.position.y -= 7.5f / camera.zoom;
					else if (creationSelection < 3) creationSelection++;
				}
				if (e.key.key == SDLK_D) {
					if (!createMode) camera.position.x += 7.5f / camera.zoom;
					else {

					}
				}
				if (e.key.key == SDLK_E) {
					camera.zoom *= 1.1f;
				}
				if (e.key.key == SDLK_Q && camera.zoom > 0.001f) {
					camera.zoom *= 0.9f;
				}
				if (e.key.key == SDLK_ESCAPE) {
					close = true;
				}
				if (e.key.key == SDLK_SPACE) {
					runSimulation = !runSimulation;
				}
				if (e.key.key == SDLK_C) {
					creationBody = Body{
						{ camera.position.x + w / 2.0f, -camera.position.y + h / 2.0f},
						1.0f,
						16.0f,
						{ 0.0f, 0.0f }
					};
					createMode = !createMode;
				}
			}
		}

		if (runSimulation && !createMode) {
			// Update the bodies.
			for (Body& body : bodies) body.update(bodies, deltaTime);

			// Move the bodies.
			for (Body& body : bodies) body.move(deltaTime);

			// Check collisions.
			for (Body& body : bodies) body.collisionCheck(bodies);
		}

		// Render stuff.
		SDL_SetRenderDrawColor(renderer, 5, 5, 10, 255);
		SDL_RenderClear(renderer);

		for (Body &body : bodies) body.render(renderer, camera);

		// Debug text.
		SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 155);
		SDL_FRect debugTextBackground = SDL_FRect(0, 0, 275, 100);
		SDL_RenderFillRect(renderer, &debugTextBackground);

		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDebugText(renderer, 10.0f, 10.0f, "Camera:");
		SDL_RenderDebugText(renderer, 20.0f, 20.0f, ("Pos: (" + std::to_string(camera.position.x) + ", " + std::to_string(camera.position.y) + ")").c_str());
		SDL_RenderDebugText(renderer, 20.0f, 30.0f, ("Zoom: " + std::to_string(camera.zoom)).c_str());

		SDL_RenderDebugText(renderer, 10.0f, 50.0f, "Simulation:");
		SDL_RenderDebugText(renderer, 20.0f, 60.0f, ("State: " + std::string(runSimulation ? "Running" : "Paused")).c_str());
		SDL_RenderDebugText(renderer, 20.0f, 70.0f, ("Body Count: " + std::to_string(bodies.size())).c_str());

		// Create mode text.
		if (createMode) {
			float xOffset = -100;
			float yOffset = -100;

			creationBody.render(renderer, camera);

			SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
			SDL_RenderDebugText(renderer, w / 2.0f + xOffset, h / 2.0f + yOffset, "Creation Mode");
			SDL_RenderDebugText(renderer, w / 2.0f + xOffset, h / 2.0f + yOffset + 20, ("Position: (" + std::to_string(creationBody.position.x) + ", " + std::to_string(creationBody.position.y) + ")").c_str());
			SDL_RenderDebugText(renderer, w / 2.0f + xOffset, h / 2.0f + yOffset + 30, ("Mass: " + std::to_string(creationBody.mass)).c_str());
			SDL_RenderDebugText(renderer, w / 2.0f + xOffset, h / 2.0f + yOffset + 40, ("Radius: " + std::to_string(creationBody.radius)).c_str());
			SDL_RenderDebugText(renderer, w / 2.0f + xOffset, h / 2.0f + yOffset + 50, ("Velocity: (" + std::to_string(creationBody.velocity.x) + ", " + std::to_string(creationBody.velocity.y) + ")").c_str());

			// Selection cursor.
			SDL_FRect debugTextBackground = SDL_FRect(w / 2.0f + xOffset - 10, h / 2.0f + yOffset + 18 + creationSelection * 10, 5, 10);
			SDL_RenderFillRect(renderer, &debugTextBackground);
		}

		SDL_RenderPresent(renderer);
	}
	
	// Cleanup.
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}