#include <SDL2/SDL.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <algorithm>

// Constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;
const int PLATFORM_WIDTH = 32;
const int PLATFORM_HEIGHT = 32;
const int PLAYER_SIZE = 32;
const int ENEMY_SIZE = 32;
const int PLAYER_SPEED = 5;
const int BULLET_SPEED = 10;

struct GameObject {
    SDL_Rect rect;
    int health;
    bool isEnemy;
};

struct Bullet {
    SDL_Rect rect;
    int dx, dy;
};

SDL_Window* window = nullptr;
SDL_Renderer* renderer = nullptr;

bool init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("2D Platformer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "Failed to create renderer: " << SDL_GetError() << std::endl;
        return false;
    }

    srand(static_cast<unsigned>(time(0)));
    return true;
}

void close() {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void spawnPlatform(std::vector<GameObject>& platforms) {
    GameObject platform = { {rand() % (SCREEN_WIDTH - PLATFORM_WIDTH), 0, PLATFORM_WIDTH, PLATFORM_HEIGHT}, 0, false };
    platforms.push_back(platform);
}

void spawnEnemy(std::vector<GameObject>& enemies) {
    GameObject enemy = { {rand() % (SCREEN_WIDTH - ENEMY_SIZE), rand() % (SCREEN_HEIGHT / 2), ENEMY_SIZE, ENEMY_SIZE}, 3, true };
    enemies.push_back(enemy);
}

void handleInput(SDL_Event& event, bool& running, SDL_Rect& player, std::vector<Bullet>& bullets) {
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            running = false;
        }
        if (event.type == SDL_KEYDOWN) {
            switch (event.key.keysym.sym) {
                case SDLK_SPACE: {
                    // Fire a bullet
                    Bullet bullet = { {player.x + PLAYER_SIZE / 2 - 4, player.y, 8, 8}, 0, -BULLET_SPEED };
                    bullets.push_back(bullet);
                    break;
                }
            }
        }
    }
}

void updateGameObjects(std::vector<GameObject>& platforms, std::vector<GameObject>& enemies, std::vector<Bullet>& bullets, SDL_Rect& player, int& score, bool& gameOver) {
    // Move platforms downward
    for (auto& platform : platforms) {
        platform.rect.y += 2;
    }
    
    // Remove off-screen platforms
    platforms.erase(std::remove_if(platforms.begin(), platforms.end(), [](const GameObject& p) { return p.rect.y > SCREEN_HEIGHT; }), platforms.end());

    // Spawn new platforms
    if (platforms.empty() || platforms.back().rect.y > PLATFORM_HEIGHT) {
        spawnPlatform(platforms);
    }

    // Move enemies and check collisions
    for (auto& enemy : enemies) {
        if (enemy.isEnemy) {
            // Chase player
            if (enemy.rect.x < player.x) enemy.rect.x++;
            if (enemy.rect.x > player.x) enemy.rect.x--;

            if (enemy.rect.y < player.y) enemy.rect.y++;
            if (enemy.rect.y > player.y) enemy.rect.y--;

            // Check collision with player
            if (SDL_HasIntersection(&enemy.rect, &player)) {
                gameOver = true;
            }
        }
    }

    // Remove bullets off-screen
    bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) { return b.rect.y < 0; }), bullets.end());

    // Move bullets and check for enemy collisions
    for (auto& bullet : bullets) {
        bullet.rect.y += bullet.dy;
        for (auto& enemy : enemies) {
            if (SDL_HasIntersection(&bullet.rect, &enemy.rect) && enemy.isEnemy) {
                enemy.health--;
                bullet.rect.y = -1; // Mark bullet for removal
                if (enemy.health <= 0) {
                    enemy.isEnemy = false; // Mark enemy as defeated
                    score += 10;
                }
            }
        }
    }

    // Remove defeated enemies
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const GameObject& e) { return !e.isEnemy; }), enemies.end());

    // Spawn new enemies
    if (enemies.size() < 5) {
        spawnEnemy(enemies);
    }
}

void renderGameObjects(const std::vector<GameObject>& platforms, const std::vector<GameObject>& enemies, const std::vector<Bullet>& bullets, const SDL_Rect& player) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Render platforms
    SDL_SetRenderDrawColor(renderer, 128, 128, 128, 255);
    for (const auto& platform : platforms) {
        SDL_RenderFillRect(renderer, &platform.rect);
    }

    // Render enemies
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (const auto& enemy : enemies) {
        SDL_RenderFillRect(renderer, &enemy.rect);
    }

    // Render bullets
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
    for (const auto& bullet : bullets) {
        SDL_RenderFillRect(renderer, &bullet.rect);
    }

    // Render player
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &player);

    SDL_RenderPresent(renderer);
}

int main(int argc, char* argv[]) {
    if (!init()) {
        return -1;
    }

    bool running = true;
    SDL_Event event;
    
    SDL_Rect player = { SCREEN_WIDTH / 2 - PLAYER_SIZE / 2, SCREEN_HEIGHT - PLAYER_SIZE - 10, PLAYER_SIZE, PLAYER_SIZE };
    std::vector<GameObject> platforms;
    std::vector<GameObject> enemies;
    std::vector<Bullet> bullets;

    int score = 0;
    bool gameOver = false;

    spawnPlatform(platforms);
    spawnEnemy(enemies);

    while (running) {
        handleInput(event, running, player, bullets);

        if (!gameOver) {
            updateGameObjects(platforms, enemies, bullets, player, score, gameOver);
        }

        renderGameObjects(platforms, enemies, bullets, player);

        SDL_Delay(16); // ~60 FPS
    }

    close();
    return 0;
}

