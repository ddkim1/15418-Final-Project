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
}

void Minesweeper::sequentialSolver() {
    
}