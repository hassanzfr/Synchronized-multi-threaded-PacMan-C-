#ifndef VAR_H
#define VAR_H

#include <SFML/System.hpp>
#include <SFML/Graphics.hpp>
#include <pthread.h>
#include <array>
#include <string>
#include <unistd.h>
#include <iostream>

int totalPellets = 0;
int totalPowerPellets = 3;

struct RenderData {
    sf::Text scoreText;
    sf::Text livesText;
    sf::Text boostedGhostsText; 
} renderData;


struct Pacman {
    int x, y;
    int dx, dy;
    int desiredDx, desiredDy; 
} pacman;

struct Ghost {
    int x, y;
    int dx, dy;
    char name;
    bool isBlue;
    bool isFast;
    sf::Clock blueClock;
    bool hasSpeedBoost; 
    sf::Clock speedBoostClock; 
};

int score = 0;
int lives = 3;
const int MAP_WIDTH = 21;
const int MAP_HEIGHT = 24;
const int CELL_SIZE = 20;

std::array<std::array<int, MAP_WIDTH>, MAP_HEIGHT> gameBoard{};

const int NUM_GHOSTS = 4;
Ghost ghosts[NUM_GHOSTS];
std::array<std::string, NUM_GHOSTS> ghostNames = {"Blinky", "Pinky", "Inky", "Clyde"};
std::array<sf::Color, NUM_GHOSTS> ghostColors = {sf::Color::Red, sf::Color::Magenta, sf::Color::Cyan, sf::Color::Green};

void initializeGhosts() {
    for (int i = 0; i < NUM_GHOSTS; ++i) {
        ghosts[i].y = 10;
        ghosts[i].x = 10;
        ghosts[i].dx = 0;
        ghosts[i].dy = 0;
        ghosts[i].name = ghostNames[i][0];
        ghosts[i].isBlue = false;
        ghosts[i].isFast = (i == 0 || i == 2); 
    }
}

void initializePacman() {
    pacman.x = 10;
    pacman.y = 2;
    pacman.dx = 0;
    pacman.dy = 0;
    pacman.desiredDx = 0;
    pacman.desiredDy = 0;
}

void initializeGameBoard() {
    std::array<std::string, MAP_HEIGHT> map_sketch = {
        "222222222222222222222",
        "211111111111111111112",
        "21                 12",
        "21 1111 11111 1111 12",
        "21 1221 12221 1221 12",
        "21 1111 11111 1111 12",
        "21                 12",
        "2111 1111   1111 1112",
        "2221 1         1 1222",
        "1111 1 111 111 1 1111",
        "2      1     1      2",
        "1111 1 1111111 1 1111",
        "2221 1         1 1222",
        "2111 1111   1111 1112",
        "21                 12",
        "21 1111 11111 1111 12",
        "21 1221 12221 1221 12",
        "21 1111 11111 1111 12",
        "21                 12",
        "211111111111111111112",
        "222222222222222222222",
        "222222222222222222222",
        "222222222222222222222",
        "222222222222222222222",
    };

    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            char cell = map_sketch[y][x];
            if (cell == ' ') {
                gameBoard[y][x] = 0;
                totalPellets++;
            } else if (cell == '1') {
                gameBoard[y][x] = 1;
            } else if (cell == '2') {
                gameBoard[y][x] = 2;
            }
        }
    }

    srand(time(NULL));
    for (int i = 0; i < totalPowerPellets; ++i) {
        int x, y;
        do {
            x = rand() % MAP_WIDTH;
            y = rand() % MAP_HEIGHT;
        } while (gameBoard[y][x] != 0);
        gameBoard[y][x] = 3;
        totalPellets--; 
    }
}

#endif
