#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <cstdlib> // For rand()

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int LANE_WIDTH = 80;
const int NUM_LANES = 6;
const int ROAD_SPEED = 5; // Adjust for scrolling speed

// Define a structure for vehicles
struct Vehicle {
  SDL_Texture* texture;
  SDL_Rect rect;
  int speed;
  bool goingSouth; // True if going south, false if going north
};

// Function to load a texture from an image file
SDL_Texture* loadTexture(SDL_Renderer* renderer, const char* filename) {
  SDL_Surface* surface = IMG_Load(filename);
  if (!surface) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load image: %s", IMG_GetError());
    return nullptr;
  }
  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  SDL_FreeSurface(surface);
  return texture;
}

int main(int argc, char* argv[]) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
    return 1;
  }

  // Create window and renderer
  SDL_Window* window = SDL_CreateWindow("Vertical Driving Game", SDL_WINDOWPOS_UNDEFINED, 
                                        SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  if (!window) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
    return 1;
  }
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
    return 1;
  }

  // Load vehicle textures (replace with your image files)
  SDL_Texture* playerTexture = loadTexture(renderer, "player_car.png");
  SDL_Texture* car1Texture = loadTexture(renderer, "car1.png");
  SDL_Texture* car2Texture = loadTexture(renderer, "car2.png");
  SDL_Texture* car3Texture = loadTexture(renderer, "car3.png"); 
  SDL_Texture* car4Texture = loadTexture(renderer, "car4.png"); 
  // ... load other car textures (car5.png, car6.png, etc.)

  // Create player vehicle
  Vehicle player;
  player.texture = playerTexture;
  player.rect = {SCREEN_WIDTH / 2 - 30, SCREEN_HEIGHT - 100, 60, 80}; // Initial position
  player.speed = 0;
  player.goingSouth = true; 

  // Create other vehicles 
  std::vector<Vehicle> vehicles;
  for (int i = 0; i < 22; ++i) { 
    Vehicle vehicle;
    // Choose a texture (example with 4 textures)
    switch (i % 4) {
      case 0: vehicle.texture = car1Texture; break;
      case 1: vehicle.texture = car2Texture; break;
      case 2: vehicle.texture = car3Texture; break;
      case 3: vehicle.texture = car4Texture; break;
      // ... add cases for more textures
    }

    // Determine direction based on lane
    int lane = rand() % (NUM_LANES * 2); // Choose from all lanes (south and north)
    if (lane < NUM_LANES) {
      vehicle.goingSouth = true;  // Southbound lanes
      vehicle.rect = {LANE_WIDTH * lane + 10, -80 * i, 60, 80}; 
    } else {
      vehicle.goingSouth = false; // Northbound lanes
      vehicle.rect = {LANE_WIDTH * (lane - NUM_LANES) + 10 + (LANE_WIDTH * NUM_LANES), SCREEN_HEIGHT + 80 * i, 60, 80}; 
    }

    vehicle.speed = ROAD_SPEED + rand() % 5; 
    vehicles.push_back(vehicle);
  }

  // Game loop
  bool running = true;
  SDL_Event event;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        running = false;
      } 
      // Mouse control for horizontal movement
      else if (event.type == SDL_MOUSEMOTION) {
        int mouseX = event.motion.x;
        player.rect.x = mouseX - player.rect.w / 2; 
        // Keep player within screen bounds
        if (player.rect.x < 0) player.rect.x = 0;
        if (player.rect.x > SCREEN_WIDTH - player.rect.w) player.rect.x = SCREEN_WIDTH - player.rect.w;
      }
      else if (event.type == SDL_KEYDOWN) {
        if (event.key.keysym.sym == SDLK_w) {
          player.speed += 2; 
        } else if (event.key.keysym.sym == SDLK_s) {
          player.speed -= 2;
          if (player.speed < 0) player.speed = 0; // Prevent reversing
        }
      }
    }

    // Update vehicle positions
    for (auto& vehicle : vehicles) {
      if (vehicle.goingSouth) {
        vehicle.rect.y += vehicle.speed;
        if (vehicle.rect.y > SCREEN_HEIGHT) { 
          vehicle.rect.y = -80; 
          int lane = rand() % NUM_LANES; // Choose a southbound lane
          vehicle.rect.x = LANE_WIDTH * lane + 10; 
        }
      } else {
        vehicle.rect.y -= vehicle.speed;
        if (vehicle.rect.y < -80) { 
          vehicle.rect.y = SCREEN_HEIGHT; 
          int lane = rand() % NUM_LANES; // Choose a northbound lane
          vehicle.rect.x = LANE_WIDTH * lane + 10 + (LANE_WIDTH * NUM_LANES);
        }
      }
    }

    // Collision detection (basic AABB collision)
    for (const auto& vehicle : vehicles) {
      if (SDL_HasIntersection(&player.rect, &vehicle.rect)) {
        SDL_Log("Collision detected!");
        // Handle collision (e.g., game over, reduce speed, etc.)
      }
    }

    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0x00, 0x7A, 0x33, 0xFF); 
    SDL_RenderClear(renderer);

    // Draw lanes 
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF); 
    for (int i = 1; i < NUM_LANES; ++i) {
      SDL_RenderDrawLine(renderer, LANE_WIDTH * i, 0, LANE_WIDTH * i, SCREEN_HEIGHT);
      SDL_RenderDrawLine(renderer, LANE_WIDTH * i + (LANE_WIDTH * NUM_LANES), 0, LANE_WIDTH * i + (LANE_WIDTH * NUM_LANES), SCREEN_HEIGHT);
    }

    // Draw vehicles
    for (const auto& vehicle : vehicles) {
      SDL_RenderCopy(renderer, vehicle.texture, nullptr, &vehicle.rect);
    }
    SDL_RenderCopy(renderer, player.texture, nullptr, &player.rect);

    // Update the screen
    SDL_RenderPresent(renderer);

    // Cap the frame rate (adjust as needed)
    SDL_Delay(1000 / 60); 
  }

  // Clean up
  SDL_DestroyTexture(playerTexture);
  SDL_DestroyTexture(car1Texture);
  SDL_DestroyTexture(car2Texture);
  SDL_DestroyTexture(car3Texture);
  SDL_DestroyTexture(car4Texture);
  // ... destroy other textures
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

