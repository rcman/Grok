#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>


// Screen dimensions
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Player settings
const int PLAYER_WIDTH = 30;
const int PLAYER_HEIGHT = 20;
const int PLAYER_SPEED = 5;
const int HELICOPTER_WIDTH = 40;
const int HELICOPTER_HEIGHT = 20;
const int HELICOPTER_SPEED = 8;

// Enemy settings
const int ENEMY_WIDTH = 30;
const int ENEMY_HEIGHT = 20;
const int ENEMY_SPEED = 3;

// Bullet settings
const int BULLET_SPEED = 10;

// Jeep settings
const int JEEP_WIDTH = 40;
const int JEEP_HEIGHT = 20;
const int JEEP_SPEED = 3;

// Function declarations
bool init();
bool loadMedia();
void close();
bool checkCollision(SDL_Rect a, SDL_Rect b);

// SDL objects
SDL_Window* gWindow = nullptr;
SDL_Renderer* gRenderer = nullptr;

// Texture wrapper class
class LTexture {
public:
    LTexture();
    ~LTexture();

    bool loadFromFile(std::string path);
    void free();
    void render(int x, int y, SDL_Rect* clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);
    int getWidth();
    int getHeight();

private:
    SDL_Texture* mTexture;
    int mWidth;
    int mHeight;
};

// Player class (can be either helicopter or jeep)
class Player {
public:
    // Initializes the variables
    Player(bool isHelicopter);

    // Takes key presses and adjusts the player's velocity
    void handleEvent(SDL_Event& e);

    // Moves the player
    void move();

    // Shows the player on the screen
    void render();

    // Gets the collision box
    SDL_Rect getCollider();

    // Sets the player's type (helicopter or jeep)
    void setIsHelicopter(bool isHelicopter) { this->isHelicopter = isHelicopter; }

    // Gets the player's type (helicopter or jeep)
    bool getIsHelicopter() const { return isHelicopter; }

private:
    // The X and Y offsets of the player
    int mPosX, mPosY;

    // The velocity of the player
    int mVelX, mVelY;

    // Collision box of the player
    SDL_Rect mCollider;

    // Whether the player is a helicopter
    bool isHelicopter;
};

// The bullet that the player fires
class Bullet {
public:
    // The dimensions of the bullet
    static const int BULLET_WIDTH = 5;
    static const int BULLET_HEIGHT = 10;

    // Initializes the variables
    Bullet(int x, int y, bool isHelicopter);

    // Moves the bullet
    void move();

    // Shows the bullet on the screen
    void render();

    // Gets the collision box
    SDL_Rect getCollider();

    // Whether the bullet is active
    bool isActive() const { return active; }

    // Sets the bullet's active state
    void setActive(bool active) { this->active = active; }

private:
    // The X and Y offsets of the bullet
    int mPosX, mPosY;

    // The velocity of the bullet
    int mVelX, mVelY;

    // Collision box of the bullet
    SDL_Rect mCollider;

    // Whether the bullet is active
    bool active;
};

// The enemy
class Enemy {
public:
    // Initializes the variables
    Enemy(int x, int y);

    // Moves the enemy
    void move();

    // Shows the enemy on the screen
    void render();

    // Gets the collision box
    SDL_Rect getCollider();

    // Whether the enemy is alive
    bool isAlive() const { return alive; }

    // Sets the enemy's alive state
    void setAlive(bool alive) { this->alive = alive; }

private:
    // The X and Y offsets of the enemy
    int mPosX, mPosY;

    // The velocity of the enemy
    int mVelX, mVelY;

    // Collision box of the enemy
    SDL_Rect mCollider;

    // Whether the enemy is alive
    bool alive;
};

// Globally used textures
LTexture gHelicopterTexture;
LTexture gJeepTexture;
LTexture gEnemyTexture;
LTexture gBulletTexture;
LTexture gBackgroundTexture;

// Sound effects
Mix_Chunk *gEngineSound = nullptr;
Mix_Chunk *gShootSound = nullptr;
Mix_Chunk *gExplosionSound = nullptr;

LTexture::LTexture() {
    // Initialize
    mTexture = nullptr;
    mWidth = 0;
    mHeight = 0;
}

LTexture::~LTexture() {
    // Deallocate
    free();
}

bool LTexture::loadFromFile(std::string path) {
    // Get rid of preexisting texture
    free();

    // The final texture
    SDL_Texture* newTexture = nullptr;

    // Load image at specified path
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
    } else {
        // Color key image
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));

        // Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == nullptr) {
            std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        } else {
            // Get image dimensions
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }

        // Get rid of old loaded surface
        SDL_FreeSurface(loadedSurface);
    }

    // Return success
    mTexture = newTexture;
    return mTexture != nullptr;
}

void LTexture::free() {
    // Free texture if it exists
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    // Set rendering space and render to screen
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };

    // Set clip rendering dimensions
    if (clip != nullptr) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }

    // Render to screen
    SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth() {
    return mWidth;
}

int LTexture::getHeight() {
    return mHeight;
}

Player::Player(bool isHelicopter) {
    // Initialize the offsets
    mPosX = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
    mPosY = SCREEN_HEIGHT - PLAYER_HEIGHT - 10;

    // Set collision box dimension
    mCollider.w = isHelicopter ? HELICOPTER_WIDTH : PLAYER_WIDTH;
    mCollider.h = isHelicopter ? HELICOPTER_HEIGHT : PLAYER_HEIGHT;

    // Initialize the velocity
    mVelX = 0;
    mVelY = 0;

    // Set player type
    this->isHelicopter = isHelicopter;
}

void Player::handleEvent(SDL_Event& e) {
    // If a key was pressed
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        // Adjust the velocity
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:
                mVelX -= PLAYER_SPEED;
                break;
            case SDLK_RIGHT:
                mVelX += PLAYER_SPEED;
                break;
            case SDLK_UP:
                if (isHelicopter) {
                    mVelY -= PLAYER_SPEED;
                }
                break;
            case SDLK_DOWN:
                if (isHelicopter) {
                    mVelY += PLAYER_SPEED;
                }
                break;
            case SDLK_a:
                if (!isHelicopter) {
                    mVelX -= PLAYER_SPEED;
                }
                break;
            case SDLK_d:
                if (!isHelicopter) {
                    mVelX += PLAYER_SPEED;
                }
                break;
        }
    }
    // If a key was released
    else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        // Adjust the velocity
        switch (e.key.keysym.sym) {
            case SDLK_LEFT:
                mVelX += PLAYER_SPEED;
                break;
            case SDLK_RIGHT:
                mVelX -= PLAYER_SPEED;
                break;
            case SDLK_UP:
                if (isHelicopter) {
                    mVelY += PLAYER_SPEED;
                }
                break;
            case SDLK_DOWN:
                if (isHelicopter) {
                    mVelY -= PLAYER_SPEED;
                }
                break;
            case SDLK_a:
                if (!isHelicopter) {
                    mVelX += PLAYER_SPEED;
                }
                break;
            case SDLK_d:
                if (!isHelicopter) {
                    mVelX -= PLAYER_SPEED;
                }
                break;
        }
    }
}

void Player::move() {
    // Move the player left or right
    mPosX += mVelX;
    mCollider.x = mPosX;

    // Keep the player in bounds
    if ((mPosX < 0) || (mPosX + mCollider.w > SCREEN_WIDTH)) {
        // Move back
        mPosX -= mVelX;
        mCollider.x = mPosX;
    }

    // Move the player up or down (only for helicopter)
    if (isHelicopter) {
        mPosY += mVelY;
        mCollider.y = mPosY;

        // Keep the player in bounds
        if ((mPosY < 0) || (mPosY + mCollider.h > SCREEN_HEIGHT)) {
            // Move back
            mPosY -= mVelY;
            mCollider.y = mPosY;
        }
    }
}

void Player::render() {
    // Show the player
    if (isHelicopter) {
        gHelicopterTexture.render(mPosX, mPosY);
    } else {
        gJeepTexture.render(mPosX, mPosY);
    }
}

SDL_Rect Player::getCollider() {
    return mCollider;
}

Bullet::Bullet(int x, int y, bool isHelicopter) {
    // Initialize the offsets
    mPosX = x;
    mPosY = y;

    // Set collision box dimension
    mCollider.w = BULLET_WIDTH;
    mCollider.h = BULLET_HEIGHT;

    // Initialize the velocity based on player type
    if (isHelicopter) {
        mVelX = 0;
        mVelY = -BULLET_SPEED;
    } else {
        mVelX = BULLET_SPEED;
        mVelY = 0;
    }

    // Initialize the bullet as active
    active = true;
}

void Bullet::move() {
    // Move the bullet
    mPosX += mVelX;
    mPosY += mVelY;
    mCollider.x = mPosX;
    mCollider.y = mPosY;

    // If the bullet went off screen, deactivate it
    if (mPosX > SCREEN_WIDTH || mPosY < 0) {
        active = false;
    }
}

void Bullet::render() {
    // Show the bullet
    gBulletTexture.render(mPosX, mPosY);
}

SDL_Rect Bullet::getCollider() {
    return mCollider;
}

Enemy::Enemy(int x, int y) {
    // Initialize the offsets
    mPosX = x;
    mPosY = y;

    // Set collision box dimension
    mCollider.w = ENEMY_WIDTH;
    mCollider.h = ENEMY_HEIGHT;

    // Initialize the velocity
    mVelX = 0;
    mVelY = ENEMY_SPEED;

    // Initialize the enemy as alive
    alive = true;
}

void Enemy::move() {
    // Move the enemy down
    mPosY += mVelY;
    mCollider.y = mPosY;

    // If the enemy went off screen, deactivate it
    if (mPosY > SCREEN_HEIGHT) {
        alive = false;
    }
}

void Enemy::render() {
    // Show the enemy
    gEnemyTexture.render(mPosX, mPosY);
}

SDL_Rect Enemy::getCollider() {
    return mCollider;
}

bool init() {
    // Initialization flag
    bool success = true;

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        success = false;
    } else {
        // Create window
        gWindow = SDL_CreateWindow("SDL Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == nullptr) {
            std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
            success = false;
        } else {
            // Create renderer
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (gRenderer == nullptr) {
                std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
                success = false;
            } else {
                // Initialize PNG loading
                if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
                    std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
                    success = false;
                }

                // Initialize SDL_mixer
                if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
                    std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
                    success = false;
                }
            }
        }
    }

    return success;
}

bool loadMedia() {
    // Loading success flag
    bool success = true;

    // Load media files
    if (!gHelicopterTexture.loadFromFile("helicopter.png")) {
        std::cerr << "Failed to load helicopter texture!" << std::endl;
        success = false;
    }
    if (!gJeepTexture.loadFromFile("jeep.png")) {
        std::cerr << "Failed to load jeep texture!" << std::endl;
        success = false;
    }
    if (!gEnemyTexture.loadFromFile("enemy.png")) {
        std::cerr << "Failed to load enemy texture!" << std::endl;
        success = false;
    }
    if (!gBulletTexture.loadFromFile("bullet.png")) {
        std::cerr << "Failed to load bullet texture!" << std::endl;
        success = false;
    }
    if (!gBackgroundTexture.loadFromFile("background.png")) {
        std::cerr << "Failed to load background texture!" << std::endl;
        success = false;
    }

    // Load sound effects
    gEngineSound = Mix_LoadWAV("engine.wav");
    if (gEngineSound == nullptr) {
        std::cerr << "Failed to load engine sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        success = false;
    }

    gShootSound = Mix_LoadWAV("shoot.wav");
    if (gShootSound == nullptr) {
        std::cerr << "Failed to load shoot sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        success = false;
    }

    gExplosionSound = Mix_LoadWAV("explosion.wav");
    if (gExplosionSound == nullptr) {
        std::cerr << "Failed to load explosion sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
        success = false;
    }

    return success;
}

void close() {
    // Free loaded images
    gHelicopterTexture.free();
    gJeepTexture.free();
    gEnemyTexture.free();
    gBulletTexture.free();
    gBackgroundTexture.free();

    // Free sound effects
    Mix_FreeChunk(gEngineSound);
    Mix_FreeChunk(gShootSound);
    Mix_FreeChunk(gExplosionSound);

    // Close SDL_mixer
    Mix_Quit();

    // Destroy renderer and window
    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);

    // Quit SDL subsystems
    IMG_Quit();
    SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b) {
    // The sides of the rectangles
    int leftA = a.x;
    int rightA = a.x + a.w;
    int topA = a.y;
    int bottomA = a.y + a.h;

    int leftB = b.x;
    int rightB = b.x + b.w;
    int topB = b.y;
    int bottomB = b.y + b.h;

    // If any sides of the rectangles are outside the bounds of the other, then no collision
    return !(bottomA <= topB || topA >= bottomB || rightA <= leftB || leftA >= rightB);
}

int main(int argc, char* argv[]) {
    // Start up SDL and create a window
    if (!init()) {
        std::cerr << "Failed to initialize!" << std::endl;
    } else {
        // Load media
        if (!loadMedia()) {
            std::cerr << "Failed to load media!" << std::endl;
        } else {
            // Main game loop
            bool quit = false;
            SDL_Event e;

            // Create player (helicopter)
            Player player(true);

            // Create bullets, enemies, etc.
            std::vector<Bullet> bullets;
            std::vector<Enemy> enemies;

            // Game loop
            while (!quit) {
                // Handle events on queue
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }

                    // Handle player input
                    player.handleEvent(e);

                    // Fire bullets
                    if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE) {
                        bullets.push_back(Bullet(player.getCollider().x + player.getCollider().w / 2, player.getCollider().y, player.getIsHelicopter()));
                        Mix_PlayChannel(-1, gShootSound, 0);
                    }
                }

                // Move player and bullets
                player.move();
                for (auto& bullet : bullets) {
                    bullet.move();
                }

                // Add new enemies
                if (rand() % 100 < 2) {
                    enemies.push_back(Enemy(rand() % (SCREEN_WIDTH - ENEMY_WIDTH), 0));
                }

                // Move enemies
                for (auto& enemy : enemies) {
                    enemy.move();
                }

                // Check for collisions
                for (auto& bullet : bullets) {
                    for (auto& enemy : enemies) {
                        if (checkCollision(bullet.getCollider(), enemy.getCollider())) {
                            bullet.setActive(false);
                            enemy.setAlive(false);
                            Mix_PlayChannel(-1, gExplosionSound, 0);
                        }
                    }
                }

                // Render
                SDL_RenderClear(gRenderer);
                gBackgroundTexture.render(0, 0);
                player.render();
                for (auto& bullet : bullets) {
                    if (bullet.isActive()) {
                        bullet.render();
                    }
                }
                for (auto& enemy : enemies) {
                    if (enemy.isAlive()) {
                        enemy.render();
                    }
                }
                SDL_RenderPresent(gRenderer);

                // Remove inactive bullets and enemies
                bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](Bullet& b) { return !b.isActive(); }), bullets.end());
                enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](Enemy& e) { return !e.isAlive(); }), enemies.end());
            }
        }
    }

    // Close and cleanup
    close();

    return 0;
}
