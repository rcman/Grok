#include <SDL.h>
#include <iostream>

const int SCREEN_WIDTH = 1920;
const int SCREEN_HEIGHT = 1080;

int main(int argc, char* argv[]) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  // Create window
  SDL_Window* window = SDL_CreateWindow("Dual Playfield Scroller", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  // Create renderer
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (renderer == nullptr) {
    std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
    return 1;
  }

  // Load background and foreground images
  SDL_Surface* backgroundSurface = SDL_LoadBMP("background.bmp");
  if (backgroundSurface == nullptr) {
    std::cerr << "Unable to load background image! SDL_Error: " << SDL_GetError() << std::endl;
    return 1;
  }
  SDL_Texture* backgroundTexture = SDL_CreateTextureFromSurface(renderer, backgroundSurface);
  SDL_FreeSurface(backgroundSurface);

  SDL_Surface* foregroundSurface = SDL_LoadBMP("foreground.bmp");
  if (foregroundSurface == nullptr) {
    std::cerr << "Unable to load foreground image! SDL_Error: " << SDL_GetError() << std::endl;
    return 1;
  }
  SDL_Texture* foregroundTexture = SDL_CreateTextureFromSurface(renderer, foregroundSurface);
  SDL_FreeSurface(foregroundSurface);

  // Scrolling offsets
  int backgroundX = 0;
  int foregroundX = 0;

  // Scrolling speeds
  int backgroundSpeed = 2;
  int foregroundSpeed = 5;

  // Game loop
  bool quit = false;
  SDL_Event e;
  while (!quit) {
    // Handle events on queue
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      }
    }

    // Update scrolling offsets
    backgroundX -= backgroundSpeed;
    foregroundX -= foregroundSpeed;

    // Reset offsets when images scroll off screen
    if (backgroundX <= -SCREEN_WIDTH) {
      backgroundX = 0;
    }
    if (foregroundX <= -SCREEN_WIDTH) {
      foregroundX = 0;
    }

    // Clear screen
    SDL_RenderClear(renderer);

    // Render background
    SDL_Rect backgroundRect = { backgroundX, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderCopy(renderer, backgroundTexture, NULL, &backgroundRect);
    SDL_Rect backgroundRect2 = { backgroundX + SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT }; // Second copy for continuous scrolling
    SDL_RenderCopy(renderer, backgroundTexture, NULL, &backgroundRect2);

    // Render foreground
    SDL_Rect foregroundRect = { foregroundX, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
    SDL_RenderCopy(renderer, foregroundTexture, NULL, &foregroundRect);
    SDL_Rect foregroundRect2 = { foregroundX + SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT }; // Second copy for continuous scrolling
    SDL_RenderCopy(renderer, foregroundTexture, NULL, &foregroundRect2);

    // Update screen
    SDL_RenderPresent(renderer);

    // Cap the frame rate
    SDL_Delay(16); // Approximately 60 FPS
  }

  // Destroy textures
  SDL_DestroyTexture(backgroundTexture);
  SDL_DestroyTexture(foregroundTexture);

  // Destroy renderer and window
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  // Quit SDL subsystems
  SDL_Quit();

  return 0;
}
