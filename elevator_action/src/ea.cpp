#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstdlib> // For rand()

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Tile constants
const int TILE_SIZE = 32;
const int LEVEL_WIDTH = 25;  // Number of tiles horizontally
const int LEVEL_HEIGHT = 19; // Number of tiles vertically

// Player constants
const int PLAYER_WIDTH = 32;
const int PLAYER_HEIGHT = 64;
const int PLAYER_SPEED = 4;

// Enemy constants
const int ENEMY_WIDTH = 32;
const int ENEMY_HEIGHT = 64;
const int ENEMY_SPEED = 2;

// Color definitions
const SDL_Color RED = {255, 0, 0, 255};
const SDL_Color BLUE = {0, 0, 255, 255};

// Structure to hold entity data (player, enemies)
struct Entity {
  SDL_Rect rect;
  SDL_Texture* texture;
  int speed;
  int health;
  bool active;
  int direction; // 0: Up, 1: Down, 2: Left, 3: Right
  bool facingRight;
};

// Structure for elevator
struct Elevator {
  SDL_Rect rect;
  SDL_Color color;
  bool moving;
  int direction; // 0: Up, 1: Down
};

// Function declarations
bool init();
bool loadMedia();
void close();
SDL_Texture* loadTexture(const std::string& path);
Mix_Chunk* loadSound(const std::string& path);
std::vector<std::vector<int>> loadLevel(const std::string& levelFilePath);

// Global variables
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
Entity gPlayer;
std::vector<Entity> gEnemies;
std::vector<Elevator> gElevators;
std::vector<std::vector<int>> gLevelData;
SDL_Texture* gTilesetTexture = nullptr;
Mix_Music* gMusic = nullptr;
Mix_Chunk* gJumpSound = nullptr;
Mix_Chunk* gShootSound = nullptr;
Mix_Chunk* gHitSound = nullptr;
Mix_Chunk* gElevatorSound = nullptr;

// Initialize SDL
bool init() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  gWindow = SDL_CreateWindow("Elevator Action", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (gWindow == nullptr) {
    std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (gRenderer == nullptr) {
    std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  int imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
    std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
    return false;
  }

  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    return false;
  }

  return true;
}

SDL_Texture* loadTexture(const std::string& path) {
  SDL_Texture* newTexture = nullptr;
  SDL_Surface* loadedSurface = IMG_Load(path.c_str());
  if (loadedSurface == nullptr) {
    std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
  } else {
    newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
    if (newTexture == nullptr) {
      std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
    }
    SDL_FreeSurface(loadedSurface);
  }
  return newTexture;
}

Mix_Chunk* loadSound(const std::string& path) {
  Mix_Chunk* sound = Mix_LoadWAV(path.c_str());
  if (sound == nullptr) {
    std::cerr << "Unable to load sound " << path << "! SDL_mixer Error: " << Mix_GetError() << std::endl;
  }
  return sound;
}

bool loadMedia() {
  gTilesetTexture = loadTexture("tileset.png");
  if (gTilesetTexture == nullptr) {
    std::cerr << "Failed to load tileset texture!" << std::endl;
    return false;
  }

  gPlayer.texture = loadTexture("player.png");
  if (gPlayer.texture == nullptr) {
    std::cerr << "Failed to load player texture!" << std::endl;
    return false;
  }

  SDL_Texture* enemyTexture = loadTexture("enemy.png");
  if (enemyTexture == nullptr) {
    std::cerr << "Failed to load enemy texture!" << std::endl;
    return false;
  }

  gPlayer.rect = {SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - PLAYER_HEIGHT - TILE_SIZE, PLAYER_WIDTH, PLAYER_HEIGHT};
  gPlayer.speed = PLAYER_SPEED;
  gPlayer.health = 100;
  gPlayer.active = true;
  gPlayer.direction = 0; // Start facing up
  gPlayer.facingRight = true;

  for (int i = 0; i < 5; ++i) {
    Entity enemy;
    enemy.rect = {rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, ENEMY_WIDTH, ENEMY_HEIGHT};
    enemy.texture = enemyTexture;
    enemy.speed = ENEMY_SPEED;
    enemy.health = 50;
    enemy.active = true;
    enemy.direction = rand() % 4; // Random initial direction
    enemy.facingRight = (rand() % 2 == 0); // Randomly face left or right
    gEnemies.push_back(enemy);
  }

  gLevelData = loadLevel("level1.txt");
  if (gLevelData.empty()) {
    std::cerr << "Failed to load level!" << std::endl;
    return false;
  }

  for (int i = 0; i < LEVEL_HEIGHT; ++i) {
    for (int j = 0; j < LEVEL_WIDTH; ++j) {
      if (gLevelData[i][j] == 3) {
        Elevator elevator;
        elevator.rect = {j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE};
        elevator.color = (rand() % 2 == 0) ? RED : BLUE; // Randomly assign color
        elevator.moving = false;
        elevator.direction = 0; // Start stationary
        gElevators.push_back(elevator);
      }
    }
  }

  gMusic = Mix_LoadMUS("music.wav");
  if (gMusic == nullptr) {
    std::cerr << "Failed to load music! SDL_mixer Error: " << Mix_GetError() << std::endl;
    return false;
  }

  gJumpSound = loadSound("jump.wav");
  gShootSound = loadSound("shoot.wav");
  gHitSound = loadSound("hit.wav");
  gElevatorSound = loadSound("elevator.wav");

  return true;
}

void close() {
  SDL_DestroyTexture(gTilesetTexture);
  gTilesetTexture = nullptr;
  SDL_DestroyTexture(gPlayer.texture);
  gPlayer.texture = nullptr;
  for (auto& enemy : gEnemies) {
    SDL_DestroyTexture(enemy.texture);
    enemy.texture = nullptr;
  }

  Mix_FreeMusic(gMusic);
  gMusic = nullptr;
  Mix_FreeChunk(gJumpSound);
  gJumpSound = nullptr;
  Mix_FreeChunk(gShootSound);
  gShootSound = nullptr;
  Mix_FreeChunk(gHitSound);
  gHitSound = nullptr;
  Mix_FreeChunk(gElevatorSound);
  gElevatorSound = nullptr;

  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  gWindow = nullptr;
  gRenderer = nullptr;

  Mix_Quit();
  IMG_Quit();
  SDL_Quit();
}

std::vector<std::vector<int>> loadLevel(const std::string& levelFilePath) {
  std::vector<std::vector<int>> level;
  std::ifstream levelFile(levelFilePath);
  if (levelFile.is_open()) {
    std::string line;
    while (std::getline(levelFile, line)) {
      std::vector<int> row;
      std::stringstream ss(line);
      std::string tile;
      while (std::getline(ss, tile, ',')) {
        row.push_back(std::stoi(tile));
      }
      level.push_back(row);
    }
    levelFile.close();
  } else {
    std::cerr << "Unable to open level file: " << levelFilePath << std::endl;
  }
  return level;
}

int main(int argc, char* args[]) {
  if (!init()) {
    std::cerr << "Failed to initialize!" << std::endl;
    return 1;
  }

  if (!loadMedia()) {
    std::cerr << "Failed to load media!" << std::endl;
    return 1;
  }

  if (Mix_PlayMusic(gMusic, -1) == -1) {
    std::cerr << "Failed to play music! SDL_mixer Error: " << Mix_GetError() << std::endl;
    return 1;
  }

  bool quit = false;
  SDL_Event e;
  while (!quit) {
    while (SDL_PollEvent(&e) != 0) {
      if (e.type == SDL_QUIT) {
        quit = true;
      } else if (e.type == SDL_KEYDOWN) {
        switch (e.key.keysym.sym) {
          case SDLK_LEFT:
            gPlayer.rect.x -= gPlayer.speed;
            gPlayer.facingRight = false;
            break;
          case SDLK_RIGHT:
            gPlayer.rect.x += gPlayer.speed;
            gPlayer.facingRight = true;
            break;
          case SDLK_UP:
            for (auto& elevator : gElevators) {
              if (SDL_HasIntersection(&gPlayer.rect, &elevator.rect)) {
                elevator.moving = true;
                elevator.direction = 0; // Move up
                Mix_PlayChannel(-1, gElevatorSound, 0);
              }
            }
            break;
          case SDLK_DOWN:
            for (auto& elevator : gElevators) {
              if (SDL_HasIntersection(&gPlayer.rect, &elevator.rect)) {
                elevator.moving = true;
                elevator.direction = 1; // Move down
                Mix_PlayChannel(-1, gElevatorSound, 0);
              }
            }
            break;
        }
      }
    }

    SDL_RenderClear(gRenderer);

    // Render Level Tiles (background)
    for (int i = 0; i < LEVEL_HEIGHT; ++i) {
      for (int j = 0; j < LEVEL_WIDTH; ++j) {
        // If it's a valid tile (not 0)
        if (gLevelData[i][j] != 0) {
          SDL_Rect destRect = {j * TILE_SIZE, i * TILE_SIZE, TILE_SIZE, TILE_SIZE};

          if (gLevelData[i][j] == 3) { // Elevator
            for (auto& elevator : gElevators) {
              if (SDL_HasIntersection(&destRect, &elevator.rect)) {
                SDL_SetRenderDrawColor(gRenderer, elevator.color.r, elevator.color.g, elevator.color.b, 255);
                SDL_RenderFillRect(gRenderer, &destRect);  // Fill with color
              }
            }
          }
        }
      }
    }

    // Render Player
    SDL_Rect playerRect = gPlayer.rect;
    SDL_RenderCopy(gRenderer, gPlayer.texture, nullptr, &playerRect);

    // Render Enemies
    for (const auto& enemy : gEnemies) {
      if (enemy.active) {
        SDL_Rect enemyRect = enemy.rect;
        SDL_RenderCopy(gRenderer, enemy.texture, nullptr, &enemyRect);
      }
    }

    // Render Elevators
    for (const auto& elevator : gElevators) {
      if (elevator.moving) {
        // You can update the elevator's position based on its movement here.
      }
      SDL_SetRenderDrawColor(gRenderer, elevator.color.r, elevator.color.g, elevator.color.b, 255);
      SDL_RenderFillRect(gRenderer, &elevator.rect); // Fill with the elevator's color
    }

    SDL_RenderPresent(gRenderer);  // Update screen
  }

  close(); // Clean up
  return 0;
}
