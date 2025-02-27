#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <random>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int TILE_SIZE = 32;

// Player structure
struct Player {
  SDL_Rect rect;
  int speed;
  int direction; // 0 = up, 1 = right, 2 = down, 3 = left
  bool smoke;
};

// Enemy structure
struct Enemy {
  SDL_Rect rect;
  int speed;
  int direction;
  bool active;
};

// Flag structure
struct Flag {
  SDL_Rect rect;
  bool collected;
};

// Rock structure
struct Rock {
  SDL_Rect rect;
};

// Function to load a texture
SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path) {
  SDL_Surface* loadedSurface = IMG_Load(path.c_str());
  if (loadedSurface == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't load image %s: %s", path.c_str(), IMG_GetError());
    return nullptr;
  }
  SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
  SDL_FreeSurface(loadedSurface);
  return newTexture;
}

// Function to generate random number between min and max (inclusive)
int getRandomNumber(int min, int max) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(min, max);
  return distrib(gen);
}

int main(int argc, char* argv[]) {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
    return 1;
  }

  // Create window
  SDL_Window* window = SDL_CreateWindow("Rally-X Clone", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (window == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window: %s", SDL_GetError());
    return 1;
  }

  // Create renderer
  SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (renderer == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create renderer: %s", SDL_GetError());
    return 1;
  }

  // Load textures
  SDL_Texture* carTexture = loadTexture(renderer, "car.png");
  SDL_Texture* enemyTexture = loadTexture(renderer, "enemy.png");
  SDL_Texture* flagTexture = loadTexture(renderer, "flag.png");
  SDL_Texture* rockTexture = loadTexture(renderer, "rock.png");
  SDL_Texture* smokeTexture = loadTexture(renderer, "smoke.png");

  // Initialize player
  Player player;
  player.rect = { SCREEN_WIDTH / 2 - TILE_SIZE / 2, SCREEN_HEIGHT / 2 - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE };
  player.speed = 3;
  player.direction = 0;
  player.smoke = false;

  // Initialize enemies
  std::vector<Enemy> enemies;
  for (int i = 0; i < 4; ++i) {
    Enemy enemy;
    enemy.rect = { getRandomNumber(0, SCREEN_WIDTH - TILE_SIZE), getRandomNumber(0, SCREEN_HEIGHT - TILE_SIZE), TILE_SIZE, TILE_SIZE };
    enemy.speed = 2;
    enemy.direction = getRandomNumber(0, 3);
    enemy.active = true;
    enemies.push_back(enemy);
  }

  // Initialize flags
  std::vector<Flag> flags;
  for (int i = 0; i < 10; ++i) {
    Flag flag;
    flag.rect = { getRandomNumber(0, SCREEN_WIDTH - TILE_SIZE), getRandomNumber(0, SCREEN_HEIGHT - TILE_SIZE), TILE_SIZE, TILE_SIZE };
    flag.collected = false;
    flags.push_back(flag);
  }

  // Initialize rocks
  std::vector<Rock> rocks;
  for (int i = 0; i < 20; ++i) {
    Rock rock;
    rock.rect = { getRandomNumber(0, SCREEN_WIDTH - TILE_SIZE), getRandomNumber(0, SCREEN_HEIGHT - TILE_SIZE), TILE_SIZE, TILE_SIZE };
    rocks.push_back(rock);
  }

  // Game loop
  bool quit = false;
  SDL_Event e;
  while (!quit) {
    // Handle events
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      } else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
          case SDLK_UP:
            player.direction = 0;
            break;
          case SDLK_RIGHT:
            player.direction = 1;
            break;
          case SDLK_DOWN:
            player.direction = 2;
            break;
          case SDLK_LEFT:
            player.direction = 3;
            break;
          case SDLK_SPACE:
            player.smoke = true;
            break;
        }
      } else if (e.type == SDL_KEYUP) {
        if (e.key.keysym.sym == SDLK_SPACE) {
          player.smoke = false;
        }
      }
    }

    // Move player
    switch (player.direction) {
      case 0:
        player.rect.y -= player.speed;
        break;
      case 1:
        player.rect.x += player.speed;
        break;
      case 2:
        player.rect.y += player.speed;
        break;
      case 3:
        player.rect.x -= player.speed;
        break;
    }

    // Keep player within screen bounds
    if (player.rect.x < 0) {
      player.rect.x = 0;
    } else if (player.rect.x > SCREEN_WIDTH - player.rect.w) {
      player.rect.x = SCREEN_WIDTH - player.rect.w;
    }
    if (player.rect.y < 0) {
      player.rect.y = 0;
    } else if (player.rect.y > SCREEN_HEIGHT - player.rect.h) {
      player.rect.y = SCREEN_HEIGHT - player.rect.h;
    }

    // Move enemies
    for (auto& enemy : enemies) {
      if (enemy.active) {
        switch (enemy.direction) {
          case 0:
            enemy.rect.y -= enemy.speed;
            break;
          case 1:
            enemy.rect.x += enemy.speed;
            break;
          case 2:
            enemy.rect.y += enemy.speed;
            break;
          case 3:
            enemy.rect.x -= enemy.speed;
            break;
        }

        // Keep enemies within screen bounds and change direction if they hit a wall
        if (enemy.rect.x < 0) {
          enemy.rect.x = 0;
          enemy.direction = getRandomNumber(0, 1) ? 1 : 2; // Right or down
        } else if (enemy.rect.x > SCREEN_WIDTH - enemy.rect.w) {
          enemy.rect.x = SCREEN_WIDTH - enemy.rect.w;
          enemy.direction = getRandomNumber(0, 1) ? 3 : 2; // Left or down
        }
        if (enemy.rect.y < 0) {
          enemy.rect.y = 0;
          enemy.direction = getRandomNumber(0, 1) ? 1 : 3; // Right or left
        } else if (enemy.rect.y > SCREEN_HEIGHT - enemy.rect.h) {
          enemy.rect.y = SCREEN_HEIGHT - enemy.rect.h;
          enemy.direction = getRandomNumber(0, 1) ? 1 : 3; // Right or left
        }
      }
    }

    // Check for collisions
    for (auto& flag : flags) {
      if (!flag.collected && SDL_HasIntersection(&player.rect, &flag.rect)) {
        flag.collected = true;
      }
    }

    for (auto& enemy : enemies) {
      if (enemy.active && SDL_HasIntersection(&player.rect, &enemy.rect)) {
        // Game Over!
        quit = true;
      }
    }

    // Clear screen
    SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(renderer);

    // Draw rocks
    for (const auto& rock : rocks) {
      SDL_RenderCopy(renderer, rockTexture, nullptr, &rock.rect);
    }

    // Draw flags
    for (const auto& flag : flags) {
      if (!flag.collected) {
        SDL_RenderCopy(renderer, flagTexture, nullptr, &flag.rect);
      }
    }

    // Draw enemies
    for (const auto& enemy : enemies) {
      if (enemy.active) {
        SDL_RenderCopy(renderer, enemyTexture, nullptr, &enemy.rect);
      }
    }

    // Draw player
    SDL_RenderCopy(renderer, carTexture, nullptr, &player.rect);

    // Draw smoke
    if (player.smoke) {
      SDL_Rect smokeRect = { player.rect.x - TILE_SIZE / 2, player.rect.y - TILE_SIZE / 2, TILE_SIZE, TILE_SIZE };
      SDL_RenderCopy(renderer, smokeTexture, nullptr, &smokeRect);
    }

    // Update screen
    SDL_RenderPresent(renderer);
  }

  // Clean up
  SDL_DestroyTexture(carTexture);
  SDL_DestroyTexture(enemyTexture);
  SDL_DestroyTexture(flagTexture);
  SDL_DestroyTexture(rockTexture);
  SDL_DestroyTexture(smokeTexture);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();

  return 0;
}

