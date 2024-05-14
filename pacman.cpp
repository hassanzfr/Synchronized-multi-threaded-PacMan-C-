#include "functions.cpp"
#include <semaphore.h>
#include <ctime>
#include <iostream>
#include <unistd.h>

pthread_mutex_t gameBoardMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ghostBoxEntrancesMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t scoreMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t livesMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t renderDataMutex = PTHREAD_MUTEX_INITIALIZER;

const int SPEED_BOOST_DURATION = 10; 

bool powerPelletActive = false;

sem_t ghostGate;
sem_t keySemaphore;
sem_t permitSemaphore;
sem_t speedBoostSemaphore;

sf::Clock pacmanClock;

const int GHOST_GATE_X = 10;
const int GHOST_GATE_Y = 9;


bool checkPelletsRemaining();
void initializeSemaphores();
void initializePacman();
void initializeGhosts();
void initializeGameBoard();
void keyPress();
void drawMap(sf::RenderWindow& window, const std::array<std::array<int, MAP_WIDTH>, MAP_HEIGHT>& gameBoard);

void initializeSemaphores() {
    sem_init(&ghostGate, 0, 1);
    sem_init(&keySemaphore, 0, 2);
    sem_init(&permitSemaphore, 0, 2);
    sem_init(&speedBoostSemaphore, 0, 2); 
}

bool checkPelletsRemaining() {
    bool pelletsRemaining = false;
    pthread_mutex_lock(&gameBoardMutex);
    for (int y = 0; y < MAP_HEIGHT; ++y) {
        for (int x = 0; x < MAP_WIDTH; ++x) {
            if (gameBoard[y][x] == 0 || gameBoard[y][x] == 3) {
                pelletsRemaining = true;
                break;
            }
        }
        if (pelletsRemaining) break;
    }
    pthread_mutex_unlock(&gameBoardMutex);
    return pelletsRemaining;
}
void* gameEngine(void* arg) {
    initializeGameBoard();
    initializePacman();

    sf::RenderWindow window(sf::VideoMode(MAP_WIDTH * CELL_SIZE, MAP_HEIGHT * CELL_SIZE), "Pacman Game");

    sf::CircleShape pacmanShape(CELL_SIZE / 2);
    pacmanShape.setFillColor(sf::Color::Yellow);

    while (window.isOpen() && lives > 0) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        keyPress();

        if (pacmanClock.getElapsedTime().asMilliseconds() >= 100) {
            pthread_mutex_lock(&gameBoardMutex);

            if (gameBoard[pacman.y + pacman.desiredDy][pacman.x + pacman.desiredDx] != 1) {
                pacman.dx = pacman.desiredDx;
                pacman.dy = pacman.desiredDy;
            }

            if (gameBoard[pacman.y + pacman.dy][pacman.x + pacman.dx] != 1) {
                pacman.x += pacman.dx;
                pacman.y += pacman.dy;

                if (gameBoard[pacman.y][pacman.x] == 0) {
                    pthread_mutex_lock(&scoreMutex);
                    gameBoard[pacman.y][pacman.x] = -1;
                    score++;
                    pthread_mutex_unlock(&scoreMutex);
                } else if (gameBoard[pacman.y][pacman.x] == 3 && !powerPelletActive) {
                    pthread_mutex_lock(&scoreMutex);
                    gameBoard[pacman.y][pacman.x] = -1;
                    score++;
                    powerPelletActive = 1;
                    for (int i = 0; i < NUM_GHOSTS; ++i) {
                        ghosts[i].isBlue = true;
                        ghosts[i].blueClock.restart();
                    }
                    pthread_mutex_unlock(&scoreMutex);
                }
            }
            pthread_mutex_unlock(&gameBoardMutex);

            pthread_mutex_lock(&livesMutex);
            for (int i = 0; i < NUM_GHOSTS; ++i) {
                if (pacman.x == ghosts[i].x && pacman.y == ghosts[i].y) {
                    if (ghosts[i].isBlue) {
                        ghosts[i].x = 8 + i;
                        ghosts[i].y = 10;
                        ghosts[i].isBlue = false;
                        std::cout << "Pacman ate ghost " << ghosts[i].name << std::endl;
                    } else {
                        lives--;
                        if (lives == 0) {
                            window.close();
                        } else {
                            initializePacman();
                        }
                        break;
                    }
                }
            }
            pthread_mutex_unlock(&livesMutex);

            if (!checkPelletsRemaining()) {
                window.close();
            }

            pacmanClock.restart();
        }

        for (int i = 0; i < NUM_GHOSTS; ++i) {
            if (ghosts[i].isBlue && ghosts[i].blueClock.getElapsedTime().asSeconds() > 10) {
                ghosts[i].isBlue = false;
                powerPelletActive = 0;
                std::cout << "Ghost " << ghosts[i].name << " turned back to normal" << std::endl;
            }
        }

        window.clear(sf::Color::Black);

        drawMap(window, gameBoard);

        pacmanShape.setPosition(pacman.x * CELL_SIZE, pacman.y * CELL_SIZE);
        window.draw(pacmanShape);

        pthread_mutex_lock(&renderDataMutex);
        window.draw(renderData.scoreText);
        window.draw(renderData.livesText);
        window.draw(renderData.boostedGhostsText); 
        pthread_mutex_unlock(&renderDataMutex);

        window.display();

        usleep(100000);
    }

    if (!window.isOpen()) {
        std::system("stty sane");
    }

    return nullptr;
}
void* uIThread(void* arg) {
    sf::Font font;
    if (!font.loadFromFile("emulogic.ttf")) {
        std::cerr << "Could not load font!\n";
        return nullptr;
    }

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(14);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition(20, 410);

    sf::Text livesText;
    livesText.setFont(font);
    livesText.setCharacterSize(14);
    livesText.setFillColor(sf::Color::White);
    livesText.setPosition(20, 440);

    sf::Text boostedGhostsText; 
    boostedGhostsText.setFont(font);
    boostedGhostsText.setCharacterSize(14);
    boostedGhostsText.setFillColor(sf::Color::White);
    boostedGhostsText.setPosition(230, 410); 

    while (true) {
        pthread_mutex_lock(&renderDataMutex);
        scoreText.setString("Score: " + std::to_string(score));
        renderData.scoreText = scoreText;

        livesText.setString("Lives: " + std::to_string(lives));
        renderData.livesText = livesText;

        std::string boostedGhosts;
        for (int i = 0; i < NUM_GHOSTS; ++i) {
            if (ghosts[i].hasSpeedBoost) {
                boostedGhosts += ghosts[i].name;
                boostedGhosts += " ";
            }
        }
        boostedGhostsText.setString("Boosted: " + boostedGhosts);
        renderData.boostedGhostsText = boostedGhostsText;
        pthread_mutex_unlock(&renderDataMutex);

        usleep(100000);
    }
    return nullptr;
}

void* ghostController(void* arg) {
    int id = *((int*)arg);
    sf::Clock ghostClock;

    while (true) {
        if (ghosts[id].y == 10) {
            std::cout << "Ghost " << ghosts[id].name << " is waiting for a key and permit" << std::endl;
            sem_wait(&keySemaphore);
            sem_wait(&permitSemaphore);

            std::cout << "Ghost " << ghosts[id].name << " acquired a key and permit" << std::endl;
            sem_wait(&ghostGate);

            ghosts[id].y = GHOST_GATE_Y;
            std::cout << "Ghost " << ghosts[id].name << " passed through the gate" << std::endl;

            sem_post(&ghostGate);
            sem_post(&keySemaphore);
            sem_post(&permitSemaphore);
        }

        if (ghostClock.getElapsedTime().asMilliseconds() >= (ghosts[id].hasSpeedBoost ? 100 : 200)) {
            pthread_mutex_lock(&gameBoardMutex);

            int newX = ghosts[id].x + ghosts[id].dx;
            int newY = ghosts[id].y + ghosts[id].dy;

            if (gameBoard[newY][newX] == 1 || rand() % 5 == 0) {
                int direction = rand() % 4;
                switch (direction) {
                    case 0: ghosts[id].dx = -1; ghosts[id].dy = 0; break;
                    case 1: ghosts[id].dx = 1; ghosts[id].dy = 0; break;
                    case 2: ghosts[id].dx = 0; ghosts[id].dy = -1; break;
                    case 3: ghosts[id].dx = 0; ghosts[id].dy = 1; break;
                }
            }

            newX = ghosts[id].x + ghosts[id].dx;
            newY = ghosts[id].y + ghosts[id].dy;

            if (gameBoard[newY][newX] != 1) {
                ghosts[id].x = newX;
                ghosts[id].y = newY;
            }

            pthread_mutex_unlock(&gameBoardMutex);
            ghostClock.restart();
        }

        if (ghosts[id].isFast && !ghosts[id].hasSpeedBoost && sem_trywait(&speedBoostSemaphore) == 0) {
            ghosts[id].hasSpeedBoost = true;
            ghosts[id].speedBoostClock.restart();
            std::cout << "Ghost " << ghosts[id].name << " acquired a speed boost" << std::endl;
        }

        if (ghosts[id].hasSpeedBoost && ghosts[id].speedBoostClock.getElapsedTime().asSeconds() >= SPEED_BOOST_DURATION) {
            ghosts[id].hasSpeedBoost = false;
            sem_post(&speedBoostSemaphore);
            std::cout << "Ghost " << ghosts[id].name << " released the speed boost" << std::endl;
        }

        usleep(100000); 
    }

    return nullptr;
}

int main() {
    initializeSemaphores();
    initializePacman();
    initializeGhosts();

    pthread_t gameEngineThread;
    pthread_t uiThread;
    pthread_t ghostThreads[NUM_GHOSTS];
    int ghostIds[NUM_GHOSTS] = {0, 1, 2, 3};

    pthread_create(&gameEngineThread, nullptr, gameEngine, nullptr);
    pthread_create(&uiThread, nullptr, uIThread, nullptr);
    for (int i = 0; i < NUM_GHOSTS; ++i) {
        pthread_create(&ghostThreads[i], nullptr, ghostController, &ghostIds[i]);
    }

    pthread_exit(nullptr);
}
