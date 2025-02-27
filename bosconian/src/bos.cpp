#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

// Screen dimensions
const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// Player constants
const int PLAYER_WIDTH = 75;
const int PLAYER_HEIGHT = 75;
const int PLAYER_SPEED = 5;

// Enemy constants
const int ENEMY_WIDTH = 75;
const int ENEMY_HEIGHT = 75;
const int ENEMY_SPEED = 3;

// Bullet constants
const int BULLET_WIDTH = 8;
const int BULLET_HEIGHT = 8;
const int BULLET_SPEED = 10;

// Structure to hold entity data (player, enemies, bullets)
struct Entity {
  SDL_Rect rect;
  SDL_Texture* texture;
  double angle; // For rotation
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
SDL_Texture* gBackgroundTexture = nullptr;
Mix_Music* gMusic = nullptr;
Mix_Chunk* gShootSound = nullptr;
Mix_Chunk* gExplosionSound = nullptr;
int gScore = 0;
bool gGameOver = false;

// Initialize SDL
bool init() {
  // Initialize SDL
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
    return false;
  }

  // Create window
  gWindow = SDL_CreateWindow("Bosconian", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
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
  gBackgroundTexture = loadTexture("background.png");
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

  // Load enemy texture (you can add more enemy types with different textures)
  SDL_Texture* enemyTexture = loadTexture("enemy.png");
  if (enemyTexture == nullptr) {
    std::cerr << "Failed to load enemy texture!" << std::endl;
    return false;
  }

  // Create player
  gPlayer.rect = {SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2, SCREEN_HEIGHT / 2 - PLAYER_HEIGHT / 2, PLAYER_WIDTH, PLAYER_HEIGHT};
  gPlayer.angle = 0.0;
  gPlayer.speed = PLAYER_SPEED;
  gPlayer.active = true;

  // Create initial enemies (example)
  for (int i = 0; i < 10; ++i) {
    Entity enemy;
    enemy.rect = {rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, ENEMY_WIDTH, ENEMY_HEIGHT};
    enemy.texture = enemyTexture;
    enemy.angle = 0.0;
    enemy.speed = ENEMY_SPEED;
    enemy.active = true;
    gEnemies.push_back(enemy);
  }

  // Load sounds
  gMusic = Mix_LoadMUS("music.wav");
  if (gMusic == nullptr) {
    std::cerr << "Failed to load music! SDL_mixer Error: " << Mix_GetError() << std::endl;
    return false;
  }

  gShootSound = loadSound("shoot.wav");
  if (gShootSound == nullptr) {
    std::cerr << "Failed to load shoot sound!" << std::endl;
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
  Mix_FreeChunk(gShootSound);
  gShootSound = nullptr;
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
          case SDLK_SPACE: { // Shoot bullet
            Entity bullet;
            bullet.rect = {gPlayer.rect.x + PLAYER_WIDTH / 2 - BULLET_WIDTH / 2, gPlayer.rect.y + PLAYER_HEIGHT / 2 - BULLET_HEIGHT / 2, BULLET_WIDTH, BULLET_HEIGHT};
            bullet.angle = gPlayer.angle;
            bullet.speed = BULLET_SPEED;
            bullet.active = true;
            gPlayerBullets.push_back(bullet);
            Mix_PlayChannel(-1, gShootSound, 0);
            break;
          }
        }
      }
    }

    // Handle continuous key presses
    const Uint8* currentKeyStates = SDL_GetKeyboardState(NULL);
    if (currentKeyStates[SDL_SCANCODE_UP]) {
      gPlayer.rect.x += (int)(gPlayer.speed * cos(gPlayer.angle * M_PI / 180));
      gPlayer.rect.y -= (int)(gPlayer.speed * sin(gPlayer.angle * M_PI / 180));
    }
    if (currentKeyStates[SDL_SCANCODE_DOWN]) {
      gPlayer.rect.x -= (int)(gPlayer.speed * cos(gPlayer.angle * M_PI / 180));
      gPlayer.rect.y += (int)(gPlayer.speed * sin(gPlayer.angle * M_PI / 180));
    }
    if (currentKeyStates[SDL_SCANCODE_LEFT]) {
      gPlayer.angle -= 5.0; // Adjust rotation speed as needed
    }
    if (currentKeyStates[SDL_SCANCODE_RIGHT]) {
      gPlayer.angle += 5.0;
    }

    // Keep player within screen bounds
    if (gPlayer.rect.x < 0) {
      gPlayer.rect.x = 0;
    } else if (gPlayer.rect.x > SCREEN_WIDTH - PLAYER_WIDTH) {
      gPlayer.rect.x = SCREEN_WIDTH - PLAYER_WIDTH;
    }
    if (gPlayer.rect.y < 0) {
      gPlayer.rect.y = 0;
    } else if (gPlayer.rect.y > SCREEN_HEIGHT - PLAYER_HEIGHT) {
      gPlayer.rect.y = SCREEN_HEIGHT - PLAYER_HEIGHT;
    }

    // Move enemy (example - you'll need more complex enemy AI)
    for (auto& enemy : gEnemies) {
      enemy.rect.x += enemy.speed;
      if (enemy.rect.x + ENEMY_WIDTH > SCREEN_WIDTH || enemy.rect.x < 0) {
        enemy.speed *= -1; // Change direction
      }
    }

    // Move player bullets
    for (auto& bullet : gPlayerBullets) {
      bullet.rect.x += (int)(bullet.speed * cos(bullet.angle * M_PI / 180));
      bullet.rect.y -= (int)(bullet.speed * sin(bullet.angle * M_PI / 180));
      if (bullet.rect.x > SCREEN_WIDTH || bullet.rect.x < 0 || bullet.rect.y > SCREEN_HEIGHT || bullet.rect.y < 0) {
        bullet.active = false;
      }
    }

    // Check for collisions between bullets and enemies
    for (auto& bullet : gPlayerBullets) {
      if (bullet.active) {
        for (auto& enemy : gEnemies) {
          if (enemy.active && SDL_HasIntersection(&bullet.rect, &enemy.rect)) {
            enemy.active = false;
            bullet.active = false;
            gScore += 100;
            Mix_PlayChannel(-1, gExplosionSound, 0);
          }
        }
      }
    }

    // Remove inactive entities
    gEnemies.erase(std::remove_if(gEnemies.begin(), gEnemies.end(), [](const Entity& e) { return !e.active; }), gEnemies.end());
    gPlayerBullets.erase(std::remove_if(gPlayerBullets.begin(), gPlayerBullets.end(), [](const Entity& b) { return !b.active; }), gPlayerBullets.end());

    // Generate new enemies (adjust frequency as needed)
    if (gEnemies.size() < 10) {
      Entity enemy;
      enemy.rect = {rand() % SCREEN_WIDTH, rand() % SCREEN_HEIGHT, ENEMY_WIDTH, ENEMY_HEIGHT};
      enemy.texture = loadTexture("assets/enemy.png"); // Reload texture for new enemies
      enemy.angle = 0.0;
      enemy.speed = ENEMY_SPEED;
      enemy.active = true;
      gEnemies.push_back(enemy);
    }

    // Clear screen
    SDL_SetRenderDrawColor(gRenderer, 0x00, 0x00, 0x00, 0xFF);
    SDL_RenderClear(gRenderer);

    // Render background
    SDL_RenderCopy(gRenderer, gBackgroundTexture, nullptr, nullptr);

    // Render player
    if (gPlayer.active) {
      SDL_RenderCopyEx(gRenderer, gPlayer.texture, nullptr, &gPlayer.rect, gPlayer.angle, nullptr, SDL_FLIP_NONE);
    }

    // Render enemies
    for (const auto& enemy : gEnemies) {
      SDL_RenderCopy(gRenderer, enemy.texture, nullptr, &enemy.rect);
    }

    // Render bullets
    SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0x00, 0xFF); // Yellow color for bullets
    for (const auto& bullet : gPlayerBullets) {
      SDL_RenderFillRect(gRenderer, &bullet.rect);
    }

    // Render score, etc.
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
