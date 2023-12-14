#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "minesweeper.h"
#include <string.h>

using namespace std;

int main(int argc, char **argv) {
    int height;
    int width;
    int mines;
    int opt;
    int mode = 0;

    while ((opt = getopt(argc, argv, "d:m:")) != -1) {
        switch (opt) {
            case 'd':
                if (!strcmp(optarg, "easy")) {
                    printf("Easy difficulty\n");
                    height = 9;
                    width = 9;
                    mines = 10;
                } else if (!strcmp(optarg, "medium")) {
                    printf("Medium difficulty\n");
                    height = 16;
                    width = 16;
                    mines = 40;
                } else if (!strcmp(optarg, "hard")) {
                    printf("Hard difficulty\n");
                    height = 16;
                    width = 30;
                    mines = 99;
                } else {
                    printf("Invalid difficulty\n");
                    return -1;
                }
                break;
            case 'm':
                mode = atoi(optarg);
                break;
            default:
                printf("invalid command line argument");
                return -1;
        }
    }

    Minesweeper* minesweeper = new Minesweeper(height, width, mines); 

    if (mode == 0) {
        printf("Solving using sequential algorithm\n");
        int correct = 0;
        for (int i=0; i<100; i++) {
            //printf("Iteration %d\n", i);
            minesweeper->newGame();
            int initialX = rand() % height;
            int initialY = rand() % width;
            minesweeper->boardSetup(initialX, initialY);
            //minesweeper->printBoard();
            //printf("before solver\n");
            bool success = minesweeper->sequentialSolver();
            //printf("Success? %d\n", success);
            //minesweeper->printBoard();
            if (success && minesweeper->solverCorrectness()) correct++;
        }
        printf("Number of correct times: %d\n", correct);
    
    } else if (mode == 1){
        printf("Solving using parallel OpenMP algorithm");
    }

    return 0;
}