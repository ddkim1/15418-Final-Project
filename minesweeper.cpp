#include "minesweeper.h"
#include <stdio.h>
#include <stdlib.h>

Minesweeper::Minesweeper(int h, int w, int m) {
    height = h;
    width = w;
    mines = m;
    minesLeft = m;
    board = (int**)malloc(height * sizeof(int));
    solverboard = (int**)malloc(height * sizeof(int));
    for (int i = 0; i < height; i++) {
        board[i] = (int*)calloc(width, sizeof(int));
        solverboard[i] = (int*)calloc(width, sizeof(int));
    }
    mineLocations = (int**)malloc(mines * sizeof(int));
    solverMineLocations = (int**)malloc(mines * sizeof(int));
}

bool Minesweeper::isValid(int x, int y) {
    return (x >= 0 && x < width && y >= 0 && y < height);
}

void Minesweeper::boardSetup(int x, int y) {
    initialX = x;
    initialY = y;
    for (int i = 0; i < mines; i++) {
        int mineX = rand() % height;
        int mineY = rand() % width;

        while (board[mineX][mineY] == -1 || (x == mineX && y == mineY)) {
            mineX = rand() % height;
            mineY = rand() % width;
        }

        mineLocations[i][0] = mineX;
        mineLocations[i][1] = mineY;
        board[mineX][mineY] = -1;

        for (int dispX = -1; dispX < 2; dispX++) {
            for (int dispY = -1; dispY < 2; dispY++) {
                if (dispX == 0 && dispY == 0) continue;
                if (isValid(mineX + dispX, mineY + dispY) && board[mineX + dispX][mineY + dispY] != -1) {
                    board[mineX + dispX][mineY + dispY]++;
                }
            }
        }
    }

    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            solverboard[i][j] = -10;
        }
    }
}

int Minesweeper::neighboringMines(int x, int y) {
    int neighboringMines = 0;
    for (int dx = -1; dx < 2; dx++) {
        for (int dy = -1; dy < 2; dy++) {
            if (isValid(x + dx, y + dy) && solverboard[x + dx][y + dy] == -1) {
                neighboringMines++;
            }
        }
    }
    return neighboringMines;
}

int Minesweeper::unknownTiles(int x, int y) {
    int unknownTiles = 0;
    for (int dx = -1; dx < 2; dx++) {
        for (int dy = -1; dy < 2; dy++) {
            if (isValid(x + dx, y + dy) && solverboard[x + dx][y + dy] == -10) {
                unknownTiles++;
            }
        }
    }
    return unknownTiles;
}

void Minesweeper::doubleTap(int x, int y) {
    for (int dx = -1; dx < 2; dx++) {
        for (int dy = -1; dy < 2; dy++) {
            if (isValid(x + dx, y + dy)) {
                solverboard[x + dx][y + dy] = board[x + dx][y + dy];
            }
        }
    }
}

void Minesweeper::deduceMines(int x, int y) {
    for (int dx = -1; dx < 2; dx++) {
        for (int dy = -1; dy < 2; dy++) {
            if (isValid(x + dx, y + dy) && solverboard[x + dx][y + dy] == -10) {
                solverboard[x + dx][y + dy] = -1;
                solverMineLocations[mines-minesLeft][0] = x + dx;
                solverMineLocations[mines-minesLeft][1] = y + dy;
                minesLeft--;
            }
        }
    }
}

bool Minesweeper::performMove(int x, int y) {
    solverboard[x][y] = board[x][y];
    if (board[x][y] == -1) {
        return false;
    }
    int neighborMines = neighboringMines(x, y);
    int unknownSquares = unknownTiles(x, y);
    bool backtrack = false;
    if (neighborMines == solverboard[x][y]) {
        doubleTap(x, y);
        backtrack = true;
    }
    if (unknownSquares == solverboard[x][y] - neighborMines) {
        deduceMines(x, y);
        backtrack = true;
    }
    while (backtrack) {
        backtrack = false;
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                neighborMines = neighboringMines(i, j);
                unknownSquares = unknownTiles(i, j);
                if (neighborMines == solverboard[i][j] || unknownSquares == solverboard[i][j] - neighborMines) {
                    doubleTap(i, j);
                    backtrack = true;
                }
                if (unknownSquares == solverboard[i][j] - neighborMines) {
                    deduceMines(i, j);
                    backtrack = true;
                }
            }
        }
    }
}

bool Minesweeper::sequentialSolver() {
    if (!performMove(initialX, initialY)) {
        for (int i = 0; i < mines; i++) {
            solverboard[mineLocations[i][0]][mineLocations[i][1]] = -1;
        }
        return false;
    }

    while (minesLeft > 0) {
        int randX = rand() % height;
        int randY = rand() % width;
        while (solverboard[randX][randY] != -10) {
            int randX = rand() % height;
            int randY = rand() % width;
        }
        if (!performMove(initialX, initialY)) {
            for (int i = 0; i < mines; i++) {
            solverboard[mineLocations[i][0]][mineLocations[i][1]] = -1;
            }
        return false;
        }
    }

    return true;
}