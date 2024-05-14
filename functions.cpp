
#include "variables.h"

void drawMap(sf::RenderWindow& window, const std::array<std::array<int, MAP_WIDTH>, MAP_HEIGHT>& gameBoard) {
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (gameBoard[y][x] == 1) {
                sf::RectangleShape wall(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                wall.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                wall.setFillColor(sf::Color::Blue);
                window.draw(wall);
            } else if (gameBoard[y][x] == 2 || gameBoard[y][x] == -1) {
                sf::RectangleShape blackWall(sf::Vector2f(CELL_SIZE, CELL_SIZE));
                blackWall.setPosition(x * CELL_SIZE, y * CELL_SIZE);
                blackWall.setFillColor(sf::Color::Black);
                window.draw(blackWall);
            } else if (gameBoard[y][x] == 3) {
                sf::CircleShape powerPellet(CELL_SIZE / 4);
                powerPellet.setPosition(x * CELL_SIZE + CELL_SIZE / 2 - powerPellet.getRadius(), y * CELL_SIZE + CELL_SIZE / 2 - powerPellet.getRadius());
                powerPellet.setFillColor(sf::Color::Yellow);
                window.draw(powerPellet);
            } else {
                sf::CircleShape pellet(CELL_SIZE / 8);
                pellet.setPosition(x * CELL_SIZE + CELL_SIZE / 2 - pellet.getRadius(), y * CELL_SIZE + CELL_SIZE / 2 - pellet.getRadius());
                pellet.setFillColor(sf::Color::White);
                window.draw(pellet);
            }
        }
    }

    for (int i = 0; i < NUM_GHOSTS; ++i) {
        sf::Color color = ghosts[i].isBlue ? sf::Color::White : ghostColors[i];
 

        sf::CircleShape ghostHead(CELL_SIZE / 2);
        ghostHead.setFillColor(color);
        ghostHead.setPosition(ghosts[i].x * CELL_SIZE, ghosts[i].y * CELL_SIZE);
        window.draw(ghostHead);

        sf::RectangleShape ghostBody(sf::Vector2f(CELL_SIZE, CELL_SIZE / 2));
        ghostBody.setFillColor(color);
        ghostBody.setPosition(ghosts[i].x * CELL_SIZE, (ghosts[i].y * CELL_SIZE) + (CELL_SIZE / 2));
        window.draw(ghostBody);
    }
}

void keyPress() {
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        pacman.desiredDx = 0;
        pacman.desiredDy = -1;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        pacman.desiredDx = 0;
        pacman.desiredDy = 1;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        pacman.desiredDx = -1;
        pacman.desiredDy = 0;
    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        pacman.desiredDx = 1;
        pacman.desiredDy = 0;
    }
}
