#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>

// Screen dimensions
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Player settings
const int PLAYER_SPEED = 5;
const int PLAYER_LIVES = 3;

// Enemy settings
const int ENEMY_ROWS = 5;
const int ENEMY_COLS = 10;
const int ENEMY_SPEED = 2;
const int ENEMY_VERTICAL_SPEED = 10;

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
TTF_Font* gFont = nullptr;

// Texture wrapper class
class LTexture {
public:
    LTexture();
    ~LTexture();

    bool loadFromFile(std::string path);
    bool loadFromRenderedText(std::string textureText, SDL_Color textColor);
    void free();
    void render(int x, int y, SDL_Rect* clip = nullptr, double angle = 0.0, SDL_Point* center = nullptr, SDL_RendererFlip flip = SDL_FLIP_NONE);
    int getWidth();
    int getHeight();

private:
    SDL_Texture* mTexture;
    int mWidth;
    int mHeight;
};

// The player-controlled dot
class Player {
public:
    static const int PLAYER_WIDTH = 20;
    static const int PLAYER_HEIGHT = 20;

    static const int PLAYER_VEL = 10;

    Player();
    void handleEvent(SDL_Event& e);
    void move();
    void render();
    SDL_Rect getCollider();
    void loseLife() { lives--; }
    int getLives() const { return lives; }

private:
    int mPosX, mPosY;
    int mVelX;
    SDL_Rect mCollider;
    int lives;
};

// The bullet that the player fires
class Bullet {
public:
    static const int BULLET_WIDTH = 5;
    static const int BULLET_HEIGHT = 10;

    Bullet(int x, int y);
    void move();
    void render();
    SDL_Rect getCollider();
    bool isActive() const { return active; }
    void setActive(bool active) { this->active = active; }

private:
    int mPosX, mPosY;
    int mVelY;
    SDL_Rect mCollider;
    bool active;
};

// The enemy that moves across the screen
class Enemy {
public:
    static const int ENEMY_WIDTH = 30;
    static const int ENEMY_HEIGHT = 20;

    Enemy(int x, int y);
    void move(int direction);
    void render();
    SDL_Rect getCollider();
    bool isAlive() const { return alive; }
    void setAlive(bool alive) { this->alive = alive; }

private:
    int mPosX, mPosY;
    int mVelX;
    SDL_Rect mCollider;
    bool alive;
};

// Globally used textures
LTexture gPlayerTexture;
LTexture gEnemyTexture;
LTexture gBulletTexture;
LTexture gTextTexture;

// Sound effects
Mix_Chunk *gShootSound = nullptr;
Mix_Chunk *gExplosionSound = nullptr;

LTexture::LTexture() {
    mTexture = nullptr;
    mWidth = 0;
    mHeight = 0;
}

LTexture::~LTexture() {
    free();
}

bool LTexture::loadFromFile(std::string path) {
    free();
    SDL_Texture* newTexture = nullptr;
    SDL_Surface* loadedSurface = IMG_Load(path.c_str());
    if (loadedSurface == nullptr) {
        std::cerr << "Unable to load image " << path << "! SDL_image Error: " << IMG_GetError() << std::endl;
    } else {
        SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
        newTexture = SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
        if (newTexture == nullptr) {
            std::cerr << "Unable to create texture from " << path << "! SDL Error: " << SDL_GetError() << std::endl;
        } else {
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }
        SDL_FreeSurface(loadedSurface);
    }
    mTexture = newTexture;
    return mTexture != nullptr;
}

bool LTexture::loadFromRenderedText(std::string textureText, SDL_Color textColor) {
    free();
    SDL_Surface* textSurface = TTF_RenderText_Solid(gFont, textureText.c_str(), textColor);
    if (textSurface == nullptr) {
        std::cerr << "Unable to render text surface! SDL_ttf Error: " << TTF_GetError() << std::endl;
    } else {
        mTexture = SDL_CreateTextureFromSurface(gRenderer, textSurface);
        if (mTexture == nullptr) {
            std::cerr << "Unable to create texture from rendered text! SDL Error: " << SDL_GetError() << std::endl;
        } else {
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }
        SDL_FreeSurface(textSurface);
    }
    return mTexture != nullptr;
}

void LTexture::free() {
    if (mTexture != nullptr) {
        SDL_DestroyTexture(mTexture);
        mTexture = nullptr;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::render(int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip) {
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };
    if (clip != nullptr) {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    SDL_RenderCopyEx(gRenderer, mTexture, clip, &renderQuad, angle, center, flip);
}

int LTexture::getWidth() {
    return mWidth;
}

int LTexture::getHeight() {
    return mHeight;
}

Player::Player() {
    mPosX = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2;
    mPosY = SCREEN_HEIGHT - PLAYER_HEIGHT - 10;
    mCollider.w = PLAYER_WIDTH;
    mCollider.h = PLAYER_HEIGHT;
    mVelX = 0;
    lives = PLAYER_LIVES;
}

void Player::handleEvent(SDL_Event& e) {
    if (e.type == SDL_KEYDOWN && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
        case SDLK_LEFT: mVelX -= PLAYER_VEL; break;
        case SDLK_RIGHT: mVelX += PLAYER_VEL; break;
        }
    } else if (e.type == SDL_KEYUP && e.key.repeat == 0) {
        switch (e.key.keysym.sym) {
        case SDLK_LEFT: mVelX += PLAYER_VEL; break;
        case SDLK_RIGHT: mVelX -= PLAYER_VEL; break;
        }
    }
}

void Player::move() {
    mPosX += mVelX;
    mCollider.x = mPosX;
    if ((mPosX < 0) || (mPosX + PLAYER_WIDTH > SCREEN_WIDTH)) {
        mPosX -= mVelX;
        mCollider.x = mPosX;
    }
}

void Player::render() {
    gPlayerTexture.render(mPosX, mPosY);
}

SDL_Rect Player::getCollider() {
    return mCollider;
}

Bullet::Bullet(int x, int y) {
    mPosX = x;
    mPosY = y;
    mCollider.w = BULLET_WIDTH;
    mCollider.h = BULLET_HEIGHT;
    mVelY = -BULLET_SPEED;
    active = true;
}

void Bullet::move() {
    mPosY += mVelY;
    mCollider.y = mPosY;
    if (mPosY < 0) {
        active = false;
    }
}

void Bullet::render() {
    gBulletTexture.render(mPosX, mPosY);
}

SDL_Rect Bullet::getCollider() {
    return mCollider;
}

Enemy::Enemy(int x, int y) {
    mPosX = x;
    mPosY = y;
    mCollider.w = ENEMY_WIDTH;
    mCollider.h = ENEMY_HEIGHT;
    mVelX = ENEMY_SPEED;
    alive = true;
}

void Enemy::move(int direction) {
    mPosX += mVelX * direction;
    mCollider.x = mPosX;
}

void Enemy::render() {
    gEnemyTexture.render(mPosX, mPosY);
}

SDL_Rect Enemy::getCollider() {
    return mCollider;
}

bool init() {
    bool success = true;
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL Error: " << SDL_GetError() << std::endl;
        success = false;
    } else {
        if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1")) {
            std::cerr << "Warning: Linear texture filtering not enabled!" << std::endl;
        }
        gWindow = SDL_CreateWindow("Galaxy Attack", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
        if (gWindow == nullptr) {
            std::cerr << "Window could not be created! SDL Error: " << SDL_GetError() << std::endl;
            success = false;
        } else {
            gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
            if (gRenderer == nullptr) {
                std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
                success = false;
            } else {
                int imgFlags = IMG_INIT_PNG;
                if (!(IMG_Init(imgFlags) & imgFlags)) {
                    std::cerr << "SDL_image could not initialize! SDL_image Error: " << IMG_GetError() << std::endl;
                    success = false;
                }
                if (Mix_Init(MIX_INIT_MP3) != MIX_INIT_MP3) {
                    std::cerr << "Mix_Init failed!" << std::endl;
                    success = false;
                }
                if (TTF_Init() == -1) {
                    std::cerr << "TTF_Init failed!" << std::endl;
                    success = false;
                }
            }
        }
    }
    return success;
}

bool loadMedia() {
    bool success = true;

    if (!gPlayerTexture.loadFromFile("player.bmp")) {
        std::cerr << "Failed to load player texture!" << std::endl;
        success = false;
    }

    if (!gEnemyTexture.loadFromFile("enemy.bmp")) {
        std::cerr << "Failed to load enemy texture!" << std::endl;
        success = false;
    }

    if (!gBulletTexture.loadFromFile("bullet.bmp")) {
        std::cerr << "Failed to load bullet texture!" << std::endl;
        success = false;
    }

    //gShootSound = Mix_LoadWAV("shoot.wav");
    //if (gShootSound == nullptr) {
    //    std::cerr << "Failed to load shoot sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
    //    success = false;
   // }

    //gExplosionSound = Mix_LoadWAV("explosion.wav");
    //if (gExplosionSound == nullptr) {
    //    std::cerr << "Failed to load explosion sound effect! SDL_mixer Error: " << Mix_GetError() << std::endl;
    //    success = false;
   // }

    gFont = TTF_OpenFont("lazy.ttf", 28);
    if (gFont == nullptr) {
        std::cerr << "Failed to load font! SDL_ttf Error: " << TTF_GetError() << std::endl;
        success = false;
    }

    return success;
}

void close() {
    gPlayerTexture.free();
    gEnemyTexture.free();
    gBulletTexture.free();
    gTextTexture.free();

    Mix_FreeChunk(gShootSound);
    Mix_FreeChunk(gExplosionSound);
    gShootSound = nullptr;
    gExplosionSound = nullptr;

    TTF_CloseFont(gFont);
    gFont = nullptr;

    SDL_DestroyRenderer(gRenderer);
    SDL_DestroyWindow(gWindow);
    gWindow = nullptr;
    gRenderer = nullptr;

    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

bool checkCollision(SDL_Rect a, SDL_Rect b) {
    return (a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y);
}

int main(int argc, char* args[]) {
    if (!init()) {
        std::cerr << "Failed to initialize!" << std::endl;
    } else {
        if (!loadMedia()) {
            std::cerr << "Failed to load media!" << std::endl;
        } else {
            bool quit = false;
            SDL_Event e;

            Player player;
            std::vector<Bullet> bullets;
            std::vector<Enemy> enemies;
            int enemyDirection = 1; // 1 = right, -1 = left

            // Initialize enemies
            for (int i = 0; i < ENEMY_ROWS; ++i) {
                for (int j = 0; j < ENEMY_COLS; ++j) {
                    enemies.push_back(Enemy(j * 60 + 10, i * 30 + 10));
                }
            }

            while (!quit) {
                while (SDL_PollEvent(&e) != 0) {
                    if (e.type == SDL_QUIT) {
                        quit = true;
                    }
                    player.handleEvent(e);
                }

                player.move();

                // Move bullets
                for (int i = 0; i < bullets.size(); ++i) {
                    if (bullets[i].isActive()) {
                        bullets[i].move();
                    } else {
                        bullets.erase(bullets.begin() + i);
                        --i;
                    }
                }

                // Move enemies
                for (auto& enemy : enemies) {
                    if (enemy.isAlive()) {
                        enemy.move(enemyDirection);
                    }
                }

                // Check collision with bullets and enemies
                for (auto& enemy : enemies) {
                    for (auto& bullet : bullets) {
                        if (checkCollision(bullet.getCollider(), enemy.getCollider())) {
                            enemy.setAlive(false);
                            bullet.setActive(false);
                            Mix_PlayChannel(-1, gExplosionSound, 0);
                        }
                    }
                }

                // If enemies hit the edge, change direction
                for (auto& enemy : enemies) {
                    if (enemy.isAlive() && (enemy.getCollider().x + enemy.getCollider().w >= SCREEN_WIDTH || enemy.getCollider().x <= 0)) {
                        enemyDirection = -enemyDirection;
                        break;
                    }
                }

                SDL_SetRenderDrawColor(gRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
                SDL_RenderClear(gRenderer);

                // Render player
                player.render();

                // Render bullets
                for (auto& bullet : bullets) {
                    bullet.render();
                }

                // Render enemies
                for (auto& enemy : enemies) {
                    if (enemy.isAlive()) {
                        enemy.render();
                    }
                }

                // Update screen
                SDL_RenderPresent(gRenderer);
            }
        }
    }

    close();
    return 0;
}

