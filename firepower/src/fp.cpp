#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>
#include <vector>
#include <cmath>

// Screen dimensions
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Player tank settings
const int TANK_WIDTH = 40;
const int TANK_HEIGHT = 30;
const int TANK_SPEED = 5;

// Enemy tank settings
const int ENEMY_TANK_WIDTH = 30;
const int ENEMY_TANK_HEIGHT = 20;
const int ENEMY_TANK_SPEED = 3;

// Bullet settings
const int BULLET_SPEED = 10;

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

// The player-controlled tank
class Tank {
public:
    // Initializes the variables
    Tank();

    // Takes key presses and adjusts the tank's velocity
    void handleEvent(SDL_Event& e);

    // Moves the tank
    void move(std::vector<SDL_Rect>& colliders);

    // Shows the tank on the screen
    void render();

    // Gets the collision box
    SDL_Rect getCollider() const;  // Marked as const

private:
    // The X and Y offsets of the tank
    int mPosX, mPosY;

    // The velocity of the tank
    int mVelX, mVelY;

    // The angle of the tank
    double mAngle;

    // Collision box of the tank
    SDL_Rect mCollider;
};

// The bullet that the tank fires
class Bullet {
public:
    // The dimensions of the bullet
    static const int BULLET_WIDTH = 5;
    static const int BULLET_HEIGHT = 10;

    // Initializes the variables
    Bullet(int x, int y, double angle);

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

// The enemy tank
class EnemyTank {
public:
    // Initializes the variables
    EnemyTank(int x, int y);

    // Moves the enemy tank
    void move(const Tank& player);

    // Shows the enemy tank on the screen
    void render();

    // Gets the collision box
    SDL_Rect getCollider();

    // Whether the enemy tank is alive
    bool isAlive() const { return alive; }

    // Sets the enemy tank's alive state
    void setAlive(bool alive) { this->alive = alive; }

private:
    // The X and Y offsets of the enemy tank
    int mPosX, mPosY;

    // The velocity of the enemy tank
    int mVelX, mVelY;

    // The angle of the enemy tank
    double mAngle;

    // Collision box of the enemy tank
    SDL_Rect mCollider;

    // Whether the enemy tank is alive
    bool alive;
};

// Globally used textures
LTexture gTankTexture;
LTexture gEnemyTankTexture;
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

Tank::Tank() {
    // Initialize the offsets
    mPosX = SCREEN_WIDTH / 2 - TANK_WIDTH / 2;
    mPosY = SCREEN_HEIGHT - TANK_HEIGHT - 10;

    // Set collision box dimension
    mCollider.w = TANK_WIDTH;
    mCollider.h = TANK_HEIGHT;

    // Initialize the velocity
    mVelX = 0;
    mVelY = 0;

    // Initialize angle
    mAngle = 0.0;
}

void Tank::handleEvent(SDL_Event& e) {
    // If a key was pressed
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        // Adjust the velocity
        switch (e.key.keysym.sym) {
        case SDLK_UP:
            mVelY -= TANK_SPEED;
            break;
        case SDLK_DOWN:
            mVelY += TANK_SPEED;
            break;
        case SDLK_LEFT:
            mAngle -= 5.0;
            break;
        case SDLK_RIGHT:
            mAngle += 5.0;
            break;
        }
    }
    // If a key was released
    else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        // Adjust the velocity
        switch (e.key.keysym.sym) {
        case SDLK_UP:
            mVelY += TANK_SPEED;
            break;
        case SDLK_DOWN:
            mVelY -= TANK_SPEED;
            break;
        }
    }
}

void Tank::move(std::vector<SDL_Rect>& colliders) {
    // Move the tank up or down
    mPosY += mVelY;
    mCollider.y = mPosY;

    // Keep the tank in bounds
    if ((mPosY < 0) || (mPosY + TANK_HEIGHT > SCREEN_HEIGHT)) {
        // Move back
        mPosY -= mVelY;
        mCollider.y = mPosY;
    }

    // Move the tank left or right
    mPosX += mVelX;
    mCollider.x = mPosX;

    // Keep the tank in bounds
    if ((mPosX < 0) || (mPosX + TANK_WIDTH > SCREEN_WIDTH)) {
        // Move back
        mPosX -= mVelX;
        mCollider.x = mPosX;
    }

    // Check for collisions with colliders
    for (const auto& collider : colliders) {
        if (checkCollision(mCollider, collider)) {
            // Move back
            mPosX -= mVelX;
            mPosY -= mVelY;
            mCollider.x = mPosX;
            mCollider.y = mPosY;
        }
    }
}

void Tank::render() {
    // Show the tank
    gTankTexture.render(mPosX, mPosY, nullptr, mAngle);
}

SDL_Rect Tank::getCollider() const {
    return mCollider;
}

Bullet::Bullet(int x, int y, double angle) {
    // Initialize the offsets
    mPosX = x;
    mPosY = y;

    // Set collision box dimension
    mCollider.w = BULLET_WIDTH;
    mCollider.h = BULLET_HEIGHT;

    // Calculate velocity
    mVelX = BULLET_SPEED * cos(angle * M_PI / 180.0);
    mVelY = BULLET_SPEED * sin(angle * M_PI / 180.0);

    // Set the active state
    active = true;
}

void Bullet::move() {
    // Move the bullet
    mPosX += mVelX;
    mPosY += mVelY;

    // Update the collider
    mCollider.x = mPosX;
    mCollider.y = mPosY;
}

void Bullet::render() {
    // Show the bullet
    gBulletTexture.render(mPosX, mPosY);
}

SDL_Rect Bullet::getCollider() {
    return mCollider;
}

EnemyTank::EnemyTank(int x, int y) {
    mPosX = x;
    mPosY = y;
    mAngle = 0.0;
    mVelX = ENEMY_TANK_SPEED;
    mVelY = 0;
    alive = true;

    // Set collision box dimension
    mCollider.w = ENEMY_TANK_WIDTH;
    mCollider.h = ENEMY_TANK_HEIGHT;
}

void EnemyTank::move(const Tank& player) {
    // Move the enemy tank towards the player tank
    int playerX = player.getCollider().x + player.getCollider().w / 2;
    int playerY = player.getCollider().y + player.getCollider().h / 2;

    // Calculate angle to player
    double angle = atan2(playerY - mPosY, playerX - mPosX) * 180.0 / M_PI;

    // Move based on the angle
    mPosX += mVelX * cos(angle * M_PI / 180.0);
    mPosY += mVelY * sin(angle * M_PI / 180.0);

    // Update collider
    mCollider.x = mPosX;
    mCollider.y = mPosY;
}

void EnemyTank::render() {
    // Show the enemy tank
    gEnemyTankTexture.render(mPosX, mPosY);
}

SDL_Rect EnemyTank::getCollider() {
    return mCollider;
}

// Check collision between two rectangles
bool checkCollision(SDL_Rect a, SDL_Rect b) {
    // Calculate the sides of each rectangle
    int leftA = a.x;
    int rightA = a.x + a.w;
    int topA = a.y;
    int bottomA = a.y + a.h;

    int leftB = b.x;
    int rightB = b.x + b.w;
    int topB = b.y;
    int bottomB = b.y + b.h;

    // Check if there is a collision
    return !(bottomA <= topB || topA >= bottomB || rightA <= leftB || leftA >= rightB);
}

bool init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create window
    gWindow = SDL_CreateWindow("Tank Game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (gWindow == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create renderer for window
    gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (gRenderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_image
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
        return false;
    }

    // Initialize SDL_mixer
    if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1) {
        std::cerr << "SDL_mixer could not initialize! Mix_Error: " << Mix_GetError() << std::endl;
        return false;
    }

    return true;
}

bool loadMedia() {
    // Load textures
    if (!gTankTexture.loadFromFile("tank.png") ||
        !gEnemyTankTexture.loadFromFile("enemy_tank.png") ||
        !gBulletTexture.loadFromFile("bullet.png") ||
        !gBackgroundTexture.loadFromFile("background.png")) {
        std::cerr << "Failed to load textures!" << std::endl;
        return false;
    }

    // Load sound effects
    gEngineSound = Mix_LoadWAV("engine.wav");
    gShootSound = Mix_LoadWAV("shoot.wav");
    gExplosionSound = Mix_LoadWAV("explosion.wav");

    if (gEngineSound == nullptr || gShootSound == nullptr || gExplosionSound == nullptr) {
        std::cerr << "Failed to load sound effects!" << std::endl;
        return false;
    }

    return true;
}

void close() {
    // Free loaded textures
    gTankTexture.free();
    gEnemyTankTexture.free();
    gBulletTexture.free();
    gBackgroundTexture.free();

    // Free sound effects
    Mix_FreeChunk(gEngineSound);
    Mix_FreeChunk(gShootSound);
    Mix_FreeChunk(gExplosionSound);

    // Quit SDL subsystems
    Mix_Quit();
    IMG_Quit();
    SDL_Quit();
}

int main() {
    // Start up SDL and create window
    if (!init()) {
        std::cerr << "Failed to initialize!" << std::endl;
    } else {
        // Load media
        if (!loadMedia()) {
            std::cerr << "Failed to load media!" << std::endl;
        } else {
            // Create player tank
            Tank player;
            std::vector<SDL_Rect> colliders; // List of colliders in the game

            // Create enemy tank
            EnemyTank enemy(100, 100);
            bool quit = false;
            SDL_Event e;

            // Game loop
            while (!quit) {
                // Handle events
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }

                    // Handle player input
                    player.handleEvent(e);
                }

                // Move player tank
                player.move(colliders);

                // Move enemy tank
                enemy.move(player);

                // Clear screen
                SDL_RenderClear(gRenderer);

                // Render background
                gBackgroundTexture.render(0, 0);

                // Render the tanks
                player.render();
                enemy.render();

                // Update screen
                SDL_RenderPresent(gRenderer);
            }
        }
    }

    // Close everything and clean up
    close();

    return 0;
}
