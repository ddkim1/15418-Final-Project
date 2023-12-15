#include "minesweeper.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <omp.h>
#include <unistd.h>

void Minesweeper::openmpDoubleTap(int x, int y) {
    for (int dx = -1; dx < 2; dx++) {
        for (int dy = -1; dy < 2; dy++) {
            //printf("inf loop\n");
            if (isValid(x + dx, y + dy)) {
                //printf("setting position %d %d as %d\n", x+dx, y+dy, board[x+dx][y+dy]);
                solverboard2[x + dx][y + dy] = board[x+dx][y+dy];
            }
        }
    }
}

void Minesweeper::openmpDeduceMines(int x, int y) {
    for (int dx = -1; dx < 2; dx++) {
        for (int dy = -1; dy < 2; dy++) {
            if (isValid(x + dx, y + dy) && solverboard2[x + dx][y + dy] == -10) {
                //printf("setting mine %d\n", mines-minesLeft);

                //printf("mine location: %d %d\n", x+dx, y+dy);
                //printf("current location value: %d\n", solverboard2[x+dx][y+dy]);
                #pragma omp critical
                {
                    if (solverboard2[x+dx][y+dy] == -10) {
                        solverboard2[x + dx][y + dy] = -1;
                        minesLeft--;
                        //printf("left mines: %d\n", minesLeft);
                    }
                }
            }
        }
    }
}

int Minesweeper::openmpNeighboringMines(int x, int y) {
    int neighboringMines = 0;
    for (int dx = -1; dx < 2; dx++) {
        for (int dy = -1; dy < 2; dy++) {
            if (isValid(x + dx, y + dy) && solverboard2[x + dx][y + dy] == -1) {
                neighboringMines++;
            }
        }
    }
    return neighboringMines;
}

int Minesweeper::openmpUnknownTiles(int x, int y) {
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

void Minesweeper::openmpPerformMove(int x, int y) {
    solverboard[x][y] = board[x][y];
    solverboard2[x][y] = board[x][y];
    bool backtrack = false;
    if (board[x][y] == -1) {
        //printf("Mine placed at %d %d\n", x, y);
        wrongGuesses++;
        minesLeft--;
        backtrack = true;
    }
    int neighborMines = neighboringMines(x, y);
    int unknownSquares = unknownTiles(x, y);
    if (neighborMines == solverboard[x][y] || solverboard[x][y] == 0) {
        //printf("doubletapping position %d %d\n", x, y);
        openmpDoubleTap(x, y);
        backtrack = true;
    }
    if (unknownSquares > 0 && unknownSquares == solverboard[x][y] - neighborMines && solverboard[x][y] != -1) {
        //printf("Deducing position %d %d\n", x, y);
        openmpDeduceMines(x, y);
        backtrack = true;
    }
    //printBoard();
    //printf("before backtrack loop\n");
    //printf("backtrack: %d\n", backtrack);
    /*
    if (backtrack) {
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                solverboard[i][j] = solverboard2[i][j];
            }
        }
    }*/

    while (backtrack) {
        //printBoard();
        //printf("after print\n");
        backtrack = false;

        #pragma omp parallel for collapse(2)
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                //printf("i: %d j: %d\n", i, j);
                if (solverboard2[i][j] >= 0) {
                    int localNeighborMines = neighboringMines(i, j);
                    int localUnknownSquares = unknownTiles(i, j);
                    //printf("unknownsquares at %d %d: %d\n", i, j, unknownSquares);
                    if (localUnknownSquares > 0) {
                        if (localNeighborMines == solverboard2[i][j]) {
                            openmpDoubleTap(i, j);
                            //printf("doubletap: setting backtrack true at i: %d j:%d\n", i, j);
                            if (!backtrack) {
                                #pragma omp critical
                                {
                                    backtrack = true;
                                }
                            }
                        } else if (localUnknownSquares > 0 && localUnknownSquares == solverboard2[i][j] - localNeighborMines && solverboard2[i][j] != -1) {
                            //printf("deducemines: setting backtrack true at i: %d j:%d\n", i, j);
                            //printf("unknownSquares: %d\n", localUnknownSquares);
                            //printf("neighborMines: %d\n", localNeighborMines);
                            //printf("number at square: %d\n", solverboard2[i][j]);
                            //printBoard();
                            openmpDeduceMines(i, j);
                            if (!backtrack) {
                                #pragma omp critical
                                {
                                    backtrack = true;
                                }
                            }
                        }
                    }
                }
            }
        }
        #pragma omp barrier
        //printf("after pragma parallel for\n");

        #pragma omp parallel for collapse(2) 
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                solverboard[i][j] = solverboard2[i][j];
            }
        }

        //printBoard();
        //sleep(1);
        //printf("continue backtrack? %d\n", backtrack);
    }
}

int Minesweeper::openmpTotalUnknown() {
    int unknown = 0;
    #pragma omp parallel for reduction(+:unknown)
    for (int i=0; i<height; i++) {
        for (int j=0; j<width; j++) {
            if (solverboard[i][j] == -10) unknown++;
        }
    }
    return unknown;
}

bool Minesweeper::openmpSolver() {

    openmpPerformMove(initialX, initialY);
    while (minesLeft > 0) {

        //printf("start of loop\n");
        int tilesLeft = openmpTotalUnknown();
        //printf("mines left: %d\n", minesLeft);
        //printf("tiles unknown: %d\n", tilesLeft);
        //printBoard();
        int randomIndex = rand() % tilesLeft;
        //printf("generate index\n");
        int unknownSeen = 0;
        int randX = 0;
        int randY = 0;
        bool found = 0;

        for (int i=0; i<height; i++) {
            for (int j=0; j<width; j++) {
                if (solverboard[i][j] == -10 && unknownSeen == randomIndex) {
                    randX = i;
                    randY = j;
                    found = true;
                    break;
                } else if (solverboard[i][j] == -10) {
                    unknownSeen++;
                }
            }
            if (found) break;
        }

        //printf("playing move %d %d\n", randX, randY);
        openmpPerformMove(randX, randY);
        //printf("mines left after move: %d\n", minesLeft);
        //printf("after perform move\n");
        //sleep(2);
    }

    return true;
}