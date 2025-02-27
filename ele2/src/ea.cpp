#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <vector>
#include <iostream>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int TILE_SIZE = 32;  // Platform size (32x32)
const int PLAYER_SIZE = 32;
const int ENEMY_SIZE = 32;

struct Vec2 {
    int x, y;
    Vec2(int x=0, int y=0) : x(x), y(y) {}
};

struct Player {
    Vec2 position;
    int velocity;
    SDL_Rect rect;
    SDL_Texture* texture;
    
    Player(int x, int y, SDL_Texture* tex) {
        position = Vec2(x, y);
        velocity = 5;
        rect = {position.x, position.y, PLAYER_SIZE, PLAYER_SIZE};
        texture = tex;
    }

    void move(int dx, int dy) {
        position.x += dx * velocity;
        position.y += dy * velocity;
        rect.x = position.x;
        rect.y = position.y;
    }
};

struct Enemy {
    Vec2 position;
    SDL_Rect rect;
    SDL_Texture* texture;

    Enemy(int x, int y, SDL_Texture* tex) {
        position = Vec2(x, y);
        rect = {position.x, position.y, ENEMY_SIZE, ENEMY_SIZE};
        texture = tex;
    }
};

struct Platform {
    SDL_Rect rect;
    SDL_Texture* texture;

    Platform(int x, int y, SDL_Texture* tex) {
        rect = {x, y, TILE_SIZE, TILE_SIZE};
        texture = tex;
    }
};

class ElevatorAction {
public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    Player* player;
    std::vector<Enemy> enemies;
    std::vector<Platform> platforms;
    SDL_Texture* playerTexture;
    SDL_Texture* enemyTexture;
    SDL_Texture* platformTexture;

    bool running;

    ElevatorAction() {
        SDL_Init(SDL_INIT_VIDEO);
        IMG_Init(IMG_INIT_PNG);  // Initialize SDL_image (PNG support)
        
        window = SDL_CreateWindow("Elevator Action Clone", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

        // Load textures
        playerTexture = loadTexture("player.png");
        enemyTexture = loadTexture("enemy.png");
        platformTexture = loadTexture("platform.png");

        player = new Player(100, 100, playerTexture);

        // Add some enemies
        enemies.push_back(Enemy(500, 100, enemyTexture));
        enemies.push_back(Enemy(600, 200, enemyTexture));

        // Add some platforms
        platforms.push_back(Platform(300, 400, platformTexture));
        platforms.push_back(Platform(400, 500, platformTexture));

        running = true;
    }

    ~ElevatorAction() {
        delete player;
        for (auto& enemy : enemies) {
            SDL_DestroyTexture(enemy.texture);
        }
        for (auto& platform : platforms) {
            SDL_DestroyTexture(platform.texture);
        }
        SDL_DestroyTexture(playerTexture);
        SDL_DestroyTexture(enemyTexture);
        SDL_DestroyTexture(platformTexture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
    }

    SDL_Texture* loadTexture(const std::string& path) {
        SDL_Surface* loadedSurface = IMG_Load(path.c_str());
        if (loadedSurface == nullptr) {
            std::cout << "Unable to load image " << path << " SDL_image Error: " << IMG_GetError() << std::endl;
            return nullptr;
        }
        SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderer, loadedSurface);
        SDL_FreeSurface(loadedSurface);
        return newTexture;
    }

    void processEvents() {
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                running = false;
            }
        }
    }

    void update() {
        const Uint8* state = SDL_GetKeyboardState(NULL);

        if (state[SDL_SCANCODE_LEFT]) {
            player->move(-1, 0);
        }
        if (state[SDL_SCANCODE_RIGHT]) {
            player->move(1, 0);
        }
        if (state[SDL_SCANCODE_UP]) {
            player->move(0, -1);
        }
        if (state[SDL_SCANCODE_DOWN]) {
            player->move(0, 1);
        }

        // Handle player collision with platforms (simplified)
        for (Platform& platform : platforms) {
            if (SDL_HasIntersection(&player->rect, &platform.rect)) {
                // Prevent player from falling through the platform
                player->position.y = platform.rect.y - PLAYER_SIZE;
                player->rect.y = player->position.y;
            }
        }

        // Check collisions with enemies
        for (Enemy& enemy : enemies) {
            if (SDL_HasIntersection(&player->rect, &enemy.rect)) {
                std::cout << "Player collided with enemy!" << std::endl;
            }
        }
    }

    void render() {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Clear screen
        SDL_RenderClear(renderer);

        // Draw platforms
        for (Platform& platform : platforms) {
            SDL_RenderCopy(renderer, platform.texture, NULL, &platform.rect);
        }

        // Draw the player
        SDL_RenderCopy(renderer, player->texture, NULL, &player->rect);

        // Draw enemies
        for (const Enemy& enemy : enemies) {
            SDL_RenderCopy(renderer, enemy.texture, NULL, &enemy.rect);
        }

        SDL_RenderPresent(renderer);  // Update screen
    }

    void run() {
        while (running) {
            processEvents();
            update();
            render();
            SDL_Delay(16);  // ~60 FPS
        }
    }
};

int main(int argc, char* argv[]) {
    ElevatorAction game;
    game.run();
    return 0;
}

