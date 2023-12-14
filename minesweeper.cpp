#include "minesweeper.h"
#include <stdio.h>
#include <stdlib.h>

Minesweeper::Minesweeper(int h, int w, int m) {
    height = h;
    width = w;
    mines = m;
    minesLeft = m;
    board = (int**)malloc(height * sizeof(int*));
    solverboard = (int**)malloc(height * sizeof(int*));
    for (int i = 0; i < height; i++) {
        board[i] = (int*)calloc(width, sizeof(int));
        solverboard[i] = (int*)calloc(width, sizeof(int));
    }
    mineLocations = (int*)malloc(2 * mines * sizeof(int));
    solverMineLocations = (int*)malloc(2 * mines * sizeof(int));
}

bool Minesweeper::isValid(int row, int col) {
    return (row >= 0 && row < height && col >= 0 && col < width);
}

void Minesweeper::newGame() {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            board[i][j] = 0;
            solverboard[i][j] = 0;
        }
    }

    for (int i = 0; i < 2 * mines; i++) {
        mineLocations[i] = -1;
        solverMineLocations[i] = -1;
    }

    minesLeft = mines;
}

void Minesweeper::boardSetup(int x, int y) {
    initialX = x;
    initialY = y;
    for (int i = 0; i < mines; i++) {
        int row = rand() % height;
        int col = rand() % width;

        while (board[row][col] == -1 || (x == row && y == col)) {
            row = rand() % height;
            col = rand() % width;
        }
        //printf("Here?\n");
        mineLocations[2*i] = row;
        mineLocations[2*i+1] = col;
        board[row][col] = -1;
        //printf("there\n");
        for (int dispX = -1; dispX < 2; dispX++) {
            for (int dispY = -1; dispY < 2; dispY++) {
                if (dispX == 0 && dispY == 0) continue;
                if (isValid(row + dispX, col + dispY) && board[row + dispX][col + dispY] != -1) {
                    board[row + dispX][col + dispY]++;
                }
            }
        }
    }
    //printf("got here fine\n");

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
                //printf("Setting mine\n");
                solverboard[x + dx][y + dy] = -1;
                solverMineLocations[2*(mines-minesLeft)] = x + dx;
                solverMineLocations[2*(mines-minesLeft)+1] = y + dy;
                minesLeft--;
            }
        }
    }
}

void Minesweeper::printBoard() {
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            printf("%d ", solverboard[i][j]);
        }
        printf("\n");
    }
    printf("-----------------------------\n");
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }
    printf("------------------------------\n");
}

bool Minesweeper::performMove(int x, int y) {
    solverboard[x][y] = board[x][y];
    if (board[x][y] == -1) {
        return false;
    }
    //printf("before vars\n");
    int neighborMines = neighboringMines(x, y);
    int unknownSquares = unknownTiles(x, y);
    bool backtrack = false;
    if (neighborMines == solverboard[x][y] || solverboard[x][y] == 0) {
        doubleTap(x, y);
        backtrack = true;
    }
    if (unknownSquares == solverboard[x][y] - neighborMines) {
        deduceMines(x, y);
        backtrack = true;
    }
    //printf("before backtrack loop\n");
    //printf("backtrack: %d\n", backtrack);
    while (backtrack) {
        //printBoard();
        //printf("after print\n");
        backtrack = false;
        //printf("before loop\n");
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                if (solverboard[i][j] == -10) continue;
                neighborMines = neighboringMines(i, j);
                unknownSquares = unknownTiles(i, j);
                //printf("Neighboring Mines: %d\n", neighborMines);
                //printf("Unknown Squares: %d\n", unknownSquares);
                if (unknownSquares == 0) continue;
                if (neighborMines == solverboard[i][j] || solverboard[i][j] == 0) {
                    doubleTap(i, j);
                    backtrack = true;
                } else if (unknownSquares == solverboard[i][j] - neighborMines) {
                    deduceMines(i, j);
                    backtrack = true;
                }
            }
        }
    }
    return true;
}

bool Minesweeper::solverCorrectness() {
    if (minesLeft > 0) {
        return false;
    }
    for (int i = 0; i < mines; i++) {
        if (solverboard[solverMineLocations[2*i]][solverMineLocations[2*i+1]] != -1) {
            return false;
        }
    }
    return true;
}

bool Minesweeper::sequentialSolver() {
    //printf("before init\n");
    if (!performMove(initialX, initialY)) {
        for (int i = 0; i < mines; i++) {
            solverboard[mineLocations[2*i]][mineLocations[2*i+1]] = -1;
        }
        return false;
    }
    //printf("after init\n");
    //printBoard();
    //printf("mines left: %d\n", minesLeft);

    while (minesLeft > 0) {
        //printf("mines left: %d\n", minesLeft);
        int randX = 0;
        int randY = 0;
        for (int i=0; i<height; i++) {
            for (int j=0; j<width; j++) {
                if (solverboard[i][j] == -10) {
                    randX = i;
                    randY = j;
                }
            }
        }
        //printf("after random move\n");
        bool moveSuccess = performMove(randX, randY);
        //printf("after move and openings\n");
        //printBoard();
        //printf("success: %d\n", moveSuccess);
        //printf("minesLeft: %d\n", minesLeft);
        if (!moveSuccess) {
            for (int i = 0; i < mines; i++) {
                solverboard[mineLocations[2*i]][mineLocations[2*i+1]] = -1;
            }
            return false;
        }
    }

    return true;
}