#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <random>
#include <algorithm>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Player constants
const int PLAYER_WIDTH = 60;
const int PLAYER_HEIGHT = 20;
const int PLAYER_SPEED = 5;

// Enemy constants
const int ENEMY_WIDTH = 40;
const int ENEMY_HEIGHT = 30;
const int ENEMY_ROWS = 5;
const int ENEMY_COLS = 11;
const int ENEMY_SPEED = 2;

// Bullet constants
const int BULLET_WIDTH = 5;
const int BULLET_HEIGHT = 10;
const int BULLET_SPEED = 10;

// Structure to hold entity data (player, enemies, bullets)
struct Entity {
  SDL_Rect rect;
  SDL_Texture* texture;
  int speed;
  bool active;
};

// Function declarations
bool init();
bool loadMedia();
void close();
SDL_Texture* loadTexture(const std::string& path);
Mix_Chunk* loadSound(const std::string& path);

// Global variables
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;
Entity gPlayer;
std::vector<Entity> gEnemies;
std::vector<Entity> gPlayerBullets;
std::vector<Entity> gEnemyBullets;
SDL_Texture* gBackgroundTexture = nullptr;
Mix_Music* gMusic = nullptr;
Mix_Chunk* gPlayerFireSound = nullptr;
Mix_Chunk* gEnemyFireSound = nullptr;
Mix_Chunk* gExplosionSound = nullptr;
int gScore = 0;
bool gGameOver = false;

// Initialize SDL
bool init() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  // Create window
  gWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
  if (gWindow == nullptr) {
    std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  // Create renderer
  gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (gRenderer == nullptr) {
    std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  // Initialize SDL_image
  int imgFlags = IMG_INIT_PNG;
  if (!(IMG_Init(imgFlags) & imgFlags)) {
    std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
    return false;
  }

  // Initialize SDL_mixer
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
    return false;
  }

  return true;
}

// Load media (images and sounds)
bool loadMedia() {
  // Load background image
  gBackgroundTexture = loadTexture("800x600png.png");
  if (gBackgroundTexture == nullptr) {
    std::cerr << "Failed to load background texture!" << std::endl;
    return false;
  }

  // Load player texture
  gPlayer.texture = loadTexture("player.png");
  if (gPlayer.texture == nullptr) {
    std::cerr << "Failed to load player texture!" << std::endl;
    return false;
  }

  // Load enemy texture
  SDL_Texture* enemyTexture = loadTexture("enemy.png");
  if (enemyTexture == nullptr) {
    std::cerr << "Failed to load enemy texture!" << std::endl;
    return false;
  }

  // Create player
  gPlayer.rect = {SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT - PLAYER_HEIGHT - 20, PLAYER_WIDTH, PLAYER_HEIGHT};
  gPlayer.speed = PLAYER_SPEED;
  gPlayer.active = true;

  // Create enemies
  for (int i = 0; i < ENEMY_ROWS; ++i) {
    for (int j = 0; j < ENEMY_COLS; ++j) {
      Entity enemy;
      enemy.rect = {j * (ENEMY_WIDTH + 20) + 80, i * (ENEMY_HEIGHT + 20) + 50, ENEMY_WIDTH, ENEMY_HEIGHT};
      enemy.texture = enemyTexture;
      enemy.speed = ENEMY_SPEED;
      enemy.active = true;
      gEnemies.push_back(enemy);
    }
  }

  // Load sounds
  gMusic = Mix_LoadMUS("sound.wav");
  if (gMusic == nullptr) {
    std::cerr << "Failed to load music! SDL_mixer Error: " << Mix_GetError() << std::endl;
    return false;
  }

  gPlayerFireSound = loadSound("sound.wav");
  if (gPlayerFireSound == nullptr) {
    std::cerr << "Failed to load player fire sound!" << std::endl;
    return false;
  }

  gEnemyFireSound = loadSound("sound.wav");
  if (gEnemyFireSound == nullptr) {
    std::cerr << "Failed to load enemy fire sound!" << std::endl;
    return false;
  }

  gExplosionSound = loadSound("explosion.wav");
  if (gExplosionSound == nullptr) {
    std::cerr << "Failed to load explosion sound!" << std::endl;
    return false;
  }

  return true;
}

// Free media and shut down SDL
void close() {
  // Free loaded images
  SDL_DestroyTexture(gBackgroundTexture);
  gBackgroundTexture = nullptr;
  SDL_DestroyTexture(gPlayer.texture);
  gPlayer.texture = nullptr;
  for (auto& enemy : gEnemies) {
    SDL_DestroyTexture(enemy.texture);
    enemy.texture = nullptr;
  }

  // Free sounds
  Mix_FreeMusic(gMusic);
  gMusic = nullptr;
  Mix_FreeChunk(gPlayerFireSound);
  gPlayerFireSound = nullptr;
  Mix_FreeChunk(gEnemyFireSound);
  gEnemyFireSound = nullptr;
  Mix_FreeChunk(gExplosionSound);
  gExplosionSound = nullptr;

  // Destroy window
  SDL_DestroyRenderer(gRenderer);
  SDL_DestroyWindow(gWindow);
  gWindow = nullptr;
  gRenderer = nullptr;

  // Quit SDL subsystems
  Mix_Quit();
  IMG_Quit();
  SDL_Quit();
}

// Load texture from file
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

// Load sound from file
Mix_Chunk* loadSound(const std::string& path) {
  Mix_Chunk* sound = Mix_LoadWAV(path.c_str());
  if (sound == nullptr) {
    std::cerr << "Unable to load sound " << path << "! SDL_mixer Error: " << Mix_GetError() << std::endl;
  }
  return sound;
}

// Main game loop
int main(int argc, char* args[]) {
  // Initialize SDL
  if (!init()) {
    std::cerr << "Failed to initialize!" << std::endl;
    return 1;
  }

  // Load media
  if (!loadMedia()) {
    std::cerr << "Failed to load media!" << std::endl;
    return 1;
  }

  // Play music
  Mix_PlayMusic(gMusic, -1);

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
          case SDLK_LEFT:
            gPlayer.rect.x -= gPlayer.speed;
            break;
          case SDLK_RIGHT:
            gPlayer.rect.x += gPlayer.speed;
            break;
          case SDLK_SPACE: {
            // Create a new bullet
            Entity bullet;
            bullet.rect = {gPlayer.rect.x + PLAYER_WIDTH / 2 - BULLET_WIDTH / 2, gPlayer.rect.y - BULLET_HEIGHT, BULLET_WIDTH, BULLET_HEIGHT};
            bullet.speed = BULLET_SPEED;
            bullet.active = true;
            gPlayerBullets.push_back(bullet);
            Mix_PlayChannel(-1, gPlayerFireSound, 0);
            break;
          }
        }
      }
    }

    // Keep player within screen bounds
    if (gPlayer.rect.x < 0) {
      gPlayer.rect.x = 0;
    } else if (gPlayer.rect.x > SCREEN_WIDTH - PLAYER_WIDTH) {
      gPlayer.rect.x = SCREEN_WIDTH - PLAYER_WIDTH;
    }

    // Move player bullets
    for (auto& bullet : gPlayerBullets) {
      bullet.rect.y -= bullet.speed;
      if (bullet.rect.y < 0) {
        bullet.active = false;
      }
    }

    // Move enemy bullets
    for (auto& bullet : gEnemyBullets) {
      bullet.rect.y += bullet.speed;
      if (bullet.rect.y > SCREEN_HEIGHT) {
        bullet.active = false;
      }
    }

    // Move enemies
    int enemyDirection = 1; // Start moving right
    for (auto& enemy : gEnemies) {
      enemy.rect.x += enemy.speed * enemyDirection;
      if (enemy.rect.x + ENEMY_WIDTH > SCREEN_WIDTH || enemy.rect.x < 0) {
        enemyDirection *= -1; // Change direction
        for (auto& e : gEnemies) {
          e.rect.y += ENEMY_HEIGHT / 2; // Move down
        }
        break;
      }
    }

    // Check for collisions
    for (auto& bullet : gPlayerBullets) {
      if (bullet.active) {
        for (auto& enemy : gEnemies) {
          if (enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
            enemy.active = false;
            bullet.active = false;
            gScore += 10;
            Mix_PlayChannel(-1, gExplosionSound, 0);
          }
        }
      }
    }

    // Check for enemy bullet collisions with player
    for (auto& bullet : gEnemyBullets) {
      if (bullet.active && SDL_HasIntersection(&bullet.rect, &gPlayer.rect)) {
        gPlayer.active = false;
        bullet.active = false;
        Mix_PlayChannel(-1, gExplosionSound, 0);
        gGameOver = true;
      }
    }

    // Check for enemy reaching bottom
    for (auto& enemy : gEnemies) {
      if (enemy.active && enemy.rect.y + ENEMY_HEIGHT > SCREEN_HEIGHT) {
        gGameOver = true;
        break;
      }
    }

    // Remove inactive entities
    gPlayerBullets.erase(std::remove_if(gPlayerBullets.begin(), gPlayerBullets.end(), [](const Entity& b) { return !b.active; }), gPlayerBullets.end());
    gEnemyBullets.erase(std::remove_if(gEnemyBullets.begin(), gEnemyBullets.end(), [](const Entity& b) { return !b.active; }), gEnemyBullets.end());
    gEnemies.erase(std::remove_if(gEnemies.begin(), gEnemies.end(), [](const Entity& e) { return !e.active; }), gEnemies.end());

    // Clear screen
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(gRenderer);

    // Render background
    SDL_RenderCopy(gRenderer, gBackgroundTexture, nullptr, nullptr);

    // Render player
    if (gPlayer.active) {
      SDL_RenderCopy(gRenderer, gPlayer.texture, nullptr, &gPlayer.rect);
    }

    // Render enemies
    for (const auto& enemy : gEnemies) {
      SDL_RenderCopy(gRenderer, enemy.texture, nullptr, &enemy.rect);
    }

    // Render bullets
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0x00, 0xFF);
    for (const auto& bullet : gPlayerBullets) {
      SDL_RenderFillRect(gRenderer, &bullet.rect);
    }
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0x00, 0x00, 0xFF);
    for (const auto& bullet : gEnemyBullets) {
      SDL_RenderFillRect(gRenderer, &bullet.rect);
    }

    // Render score
    // (Implementation for rendering text using SDL_ttf is omitted for brevity)

    // Update screen
    SDL_RenderPresent(gRenderer);

    // Cap frame rate
    SDL_Delay(1000 / 60);
  }

  // Free resources and close SDL
  close();

  return 0;
}

