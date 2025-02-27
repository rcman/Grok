#include <SDL2/SDL.h>
#include <iostream>
#include <vector>
#include <stack>
#include <random>

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int CELL_SIZE = 40;

// Structure to represent a cell in the maze
struct Cell {
    bool walls[4]; // North, East, South, West
    bool visited;

    Cell() : visited(false) {
        for (int i = 0; i < 4; i++) {
            walls[i] = true; // Initially all walls are present
        }
    }
};

// Function to generate the maze using recursive backtracking
void generateMaze(std::vector<std::vector<Cell>>& grid) {
    int rows = grid.size();
    int cols = grid[0].size();
    std::stack<std::pair<int, int>> stack;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 3);

    int startX = 0, startY = 0;
    stack.push({startX, startY});
    grid[startY][startX].visited = true;

    while (!stack.empty()) {
        int x = stack.top().first;
        int y = stack.top().second;
        stack.pop();

        // Get unvisited neighbors
        std::vector<int> neighbors;
        if (x > 0 && !grid[y][x - 1].visited) neighbors.push_back(0); // West
        if (y > 0 && !grid[y - 1][x].visited) neighbors.push_back(1); // North
        if (x < cols - 1 && !grid[y][x + 1].visited) neighbors.push_back(2); // East
        if (y < rows - 1 && !grid[y + 1][x].visited) neighbors.push_back(3); // South

        if (!neighbors.empty()) {
            stack.push({x, y});

            int next = neighbors[distrib(gen)];
            int nx = x, ny = y;
            switch (next) {
                case 0: // West
                    nx = x - 1;
                    grid[y][x].walls[0] = false;
                    grid[ny][nx].walls[2] = false;
                    break;
                case 1: // North
                    ny = y - 1;
                    grid[y][x].walls[1] = false;
                    grid[ny][nx].walls[3] = false;
                    break;
                case 2: // East
                    nx = x + 1;
                    grid[y][x].walls[2] = false;
                    grid[ny][nx].walls[0] = false;
                    break;
                case 3: // South
                    ny = y + 1;
                    grid[y][x].walls[3] = false;
                    grid[ny][nx].walls[1] = false;
                    break;
            }

            stack.push({nx, ny});
            grid[ny][nx].visited = true;
        }
    }
}

// Function to render the maze
void renderMaze(SDL_Renderer* renderer, const std::vector<std::vector<Cell>>& grid) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Set background color to white
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // Set wall color to black

    int rows = grid.size();
    int cols = grid[0].size();

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            int x1 = j * CELL_SIZE;
            int y1 = i * CELL_SIZE;
            int x2 = x1 + CELL_SIZE;
            int y2 = y1 + CELL_SIZE;

            // Draw the walls for each cell
            if (grid[i][j].walls[0]) { // West
                SDL_RenderDrawLine(renderer, x1, y1, x1, y2);
            }
            if (grid[i][j].walls[1]) { // North
                SDL_RenderDrawLine(renderer, x1, y1, x2, y1);
            }
            if (grid[i][j].walls[2]) { // East
                SDL_RenderDrawLine(renderer, x2, y1, x2, y2);
            }
            if (grid[i][j].walls[3]) { // South
                SDL_RenderDrawLine(renderer, x1, y2, x2, y2);
            }
        }
    }

    SDL_RenderPresent(renderer); // Present the rendered frame
}

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create window
    SDL_Window* window = SDL_CreateWindow("Maze Generator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == nullptr) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Create maze grid
    int gridRows = SCREEN_HEIGHT / CELL_SIZE;
    int gridCols = SCREEN_WIDTH / CELL_SIZE;
    std::vector<std::vector<Cell>> grid(gridRows, std::vector<Cell>(gridCols));

    // Generate the maze
    generateMaze(grid);

    // Game loop
    bool quit = false;
    SDL_Event e;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Render the maze
        renderMaze(renderer, grid);
    }

    // Clean up
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
