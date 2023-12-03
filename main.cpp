#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "minesweeper.h"

using namespace std;

int main(int argc, char **argv) {
    int height;
    int width;
    int mines;
    int opt;
    int mode;

    while ((opt = getopt(argc, argv, "d:m:")) != -1) {
        switch (opt) {
            case 'd':
                if (optarg == "easy") {
                    printf("Easy difficulty");
                    height = 9;
                    width = 9;
                    mines = 10;
                } else if (optarg == "medium") {
                    height = 16;
                    width = 16;
                    mines = 40;
                } else if (optarg == "hard") {
                    height = 30;
                    width = 16;
                    mines = 99;
                } else {
                    printf("invalid difficulty");
                    return -1;
                }
            case 'm':
                mode = atoi(optarg);
            default:
                printf("invalid command line argument");
                return -1;
        }
    }

    Minesweeper* minesweeper = new Minesweeper(h, w, mines);
    int initialX = rand() % height;
    int initialY = rand() % width;
    minesweeper->boardSetup(initialX, initialY);

    if (mode == 0) {
        printf("Solving using sequential algorithm");
        minesweeper->sequentialSolver();
    }

}