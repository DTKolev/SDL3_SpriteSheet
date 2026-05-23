//Includes
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>

//Definitions
typedef std::vector<std::pair<int, int>> animVector;

class ErrorProcessing {
	public:
		void init() {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error initializing SDL", nullptr);
			SDL_Quit();
		}
		void window(SDL_Window* window) {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating window", window);
			SDL_DestroyWindow(window);
			SDL_Quit();
		}
		void renderer(SDL_Renderer* renderer, SDL_Window* window) {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating renderer", window);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_Quit();
		}
		void surface(SDL_Surface* surface, SDL_Renderer* renderer, SDL_Window* window) {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error loading image", window);
			SDL_DestroySurface(surface);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_Quit();
		}
		void texture(SDL_Texture* texture, SDL_Renderer* renderer, SDL_Window* window) {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Error creating texture", window);
			SDL_DestroyTexture(texture);
			SDL_DestroyRenderer(renderer);
			SDL_DestroyWindow(window);
			SDL_Quit();
		}
};

//Function definition
animVector constructAnimation(int rowNumber, int columnAmount) {
	animVector vector;
	for (int i = 0; i < columnAmount; i++) {
		vector.push_back({ rowNumber, i });
	}

	return vector;
}

animVector mergeAttacks(animVector attack01, animVector attack02) {
	animVector attackCombo = attack01;
	for (std::pair<int, int> frame : attack02) {
		attackCombo.push_back(frame);
	}

	return attackCombo;
}

void resetAnimation(double* timeBuffer, double* animationBuffer, int* currentFrame) {
	*timeBuffer = 0;
	*animationBuffer = 0;
	*currentFrame = 0;
}

int main(int argc, char* argv[]) {
	ErrorProcessing processError;

	//Initialize SDL
	if (!SDL_Init(SDL_INIT_VIDEO)) {
		processError.init();
		return 1;
	}

	//Create window
	SDL_Window* window = SDL_CreateWindow("Sprite Viewer", 300, 200, SDL_WINDOW_BORDERLESS);
	if (window == nullptr) {
		processError.window(window);
		return 1;
	}

	//Create renderer
	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	if (renderer == nullptr) {
		processError.renderer(renderer, window);
		return 1;
	}

	//Load sprite sheet and create texture
	const char* binaryPath = SDL_GetBasePath();
	std::string imagePath = std::string(binaryPath) + "../src/adventurer_sprite_sheet.png";
	SDL_Surface* surface = IMG_Load(imagePath.c_str());
	if (surface == nullptr) {
		processError.surface(surface, renderer, window);
		return 1;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	SDL_DestroySurface(surface);
	if (texture == nullptr) {
		processError.texture(texture, renderer, window);
		return 1;
	}

	//Split texture
	int rowNumber = 8;
	int columnNumber = 13;
	int spriteWidthPixels = 128;
	int spriteHeightPixels = 128;

	std::vector<SDL_FRect> rectangles;
	for (int i = 0; i < rowNumber; i++) {
		for (int j = 0; j < columnNumber; j++) {
			rectangles.push_back(SDL_FRect{ (float)(j * spriteWidthPixels), (float)(i * spriteHeightPixels), (float)spriteWidthPixels, (float)spriteHeightPixels });
		}
	}

	//Split animations
	animVector idle = constructAnimation(0, 13);
	animVector run = constructAnimation(1, 8);
	animVector attack1 = constructAnimation(2, 10);
	animVector attack2 = constructAnimation(3, 10);
	animVector attack3 = constructAnimation(4, 10);
	animVector jump = constructAnimation(5, 6);
	animVector attackCombo = mergeAttacks(mergeAttacks(attack1, attack2), attack3);

	//Declare loop variables
	SDL_Event event;
	bool running = true;
	animVector currentAnimation = idle;
	int currentFrame = 0;

	//Time variables
	double maxDuration = 0.1;
	double timeBuffer = 0;
	double deltaTime = 0;
	double animationBuffer = 0;
	double animationDuration;

	//Frame begin
	Uint64 frameBeginTime = SDL_GetPerformanceCounter();
	Uint64 performanceFrequency = SDL_GetPerformanceFrequency();

	//Core loop
	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_EVENT_QUIT) running = false;
			if (event.type == SDL_EVENT_KEY_DOWN) {
				SDL_Keycode keyCode = event.key.key;
				switch (keyCode) {
					case SDLK_ESCAPE:
						running = false;
						break;
					case SDLK_Q:
						currentAnimation = run;
						resetAnimation(&timeBuffer, &animationBuffer, &currentFrame);
						break;
					case SDLK_W:
						currentAnimation = attack1;
						resetAnimation(&timeBuffer, &animationBuffer, &currentFrame);
						break;
					case SDLK_E:
						currentAnimation = attack2;
						resetAnimation(&timeBuffer, &animationBuffer, &currentFrame);
						break;
					case SDLK_R:
						currentAnimation = attack3;
						resetAnimation(&timeBuffer, &animationBuffer, &currentFrame);
						break;
					case SDLK_T:
						currentAnimation = attackCombo;
						resetAnimation(&timeBuffer, &animationBuffer, &currentFrame);
						break;
					case SDLK_U:
						currentAnimation = jump;
						resetAnimation(&timeBuffer, &animationBuffer, &currentFrame);
						break;
					case SDLK_SPACE:
						currentAnimation = idle;
						resetAnimation(&timeBuffer, &animationBuffer, &currentFrame);
						break;
				}
				currentFrame = 0;
			}
		}

		//Clear screen
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
		SDL_RenderClear(renderer);

		//Pick frame
		std::pair<int, int> currentPair = currentAnimation[currentFrame];
		int position = currentPair.second + currentPair.first * columnNumber;

		//Define frame screenspace
		SDL_FRect source = rectangles[position];
		SDL_FRect destination = { 50, 75, (float)spriteWidthPixels, (float)spriteHeightPixels };
		SDL_RenderTexture(renderer, texture, &source, &destination);

		//Draw to screen
		SDL_RenderPresent(renderer);

		//Update animation
		timeBuffer += deltaTime;
		if (timeBuffer > maxDuration) {
			timeBuffer = 0;
			currentFrame = (currentFrame + 1) % currentAnimation.size();
		}

		//Animation timers
		animationDuration = maxDuration * (double)currentAnimation.size();
		if (currentAnimation == jump) {
			animationBuffer += deltaTime;
			if (animationBuffer > animationDuration) {
				currentAnimation = run;
				animationBuffer = 0;
				timeBuffer = 0;
				currentFrame = run.size() - 2;
			}
		}

		if (currentAnimation == attackCombo) {
			animationBuffer += deltaTime;
			if (animationBuffer > animationDuration) {
				currentAnimation = idle;
				animationBuffer = 0;
				timeBuffer = 0;
				currentFrame = idle.size() - 1;
			}
		}
		
		//Delta time
		Uint64 frameEndTime = SDL_GetPerformanceCounter();
		deltaTime = (double)(frameEndTime - frameBeginTime) / performanceFrequency;

		frameBeginTime = frameEndTime;
	}
	
	//Exit program
	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}