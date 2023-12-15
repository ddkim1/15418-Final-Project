#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "minesweeper.h"
#include <string.h>
#include <time.h>
#include "cycleTImer.h"

using namespace std;

int main(int argc, char **argv) {
    int height;
    int width;
    int mines;
    int opt;
    int mode = 0;

    while ((opt = getopt(argc, argv, "d:v:w:h:m:")) != -1) {
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
            case 'v':
                mode = atoi(optarg);
                break;
            case 'h':
                height = atoi(optarg);
                break;
            case 'w':
                width = atoi(optarg);
                break;
            case 'm':
                mines = atoi(optarg);
                break;
            default:
                printf("invalid command line argument");
                return -1;
        }
    }

    Minesweeper* minesweeper = new Minesweeper(height, width, mines);
    //printf("after making\n"); 
    //printf("mode: %d\n", mode);

    if (mode == 0) {
        printf("Solving using sequential algorithm\n");
        int correct = 0;
        srand(time(0));
        int incorrectGuesses = 0;
        double totalTime = 0.;
        for (int i=0; i<50; i++) {
            minesweeper->newGame();
            int initialX = rand() % height;
            int initialY = rand() % width;
            minesweeper->boardSetup(initialX, initialY);
            double startTime = CycleTimer::currentSeconds();
            bool success = minesweeper->sequentialSolver();
            double endTime = CycleTimer::currentSeconds();
            double runTime = endTime - startTime;
            totalTime += runTime;
            if (success && minesweeper->solverCorrectness()) correct++;
            incorrectGuesses += minesweeper->wrongGuesses;
        }
        printf("Number of correct times: %d\n", correct);
        printf("Average incorrect guesses: %.2f\n", incorrectGuesses / 100.f);
        printf("Average run time: %.2f ms\n", 1000.f * (totalTime / 50));
    } else if (mode == 1){
        printf("Solving using parallel OpenMP algorithm\n");
        int correct = 0;
        srand(time(0));
        int incorrectGuesses = 0;
        double totalTime = 0.;
        for (int i=0; i<50; i++) {
            minesweeper->newGame();
            int initialX = rand() % height;
            int initialY = rand() % width;
            minesweeper->boardSetup(initialX, initialY);
            double startTime = CycleTimer::currentSeconds();
            bool success = minesweeper->openmpSolver();
            double endTime = CycleTimer::currentSeconds();
            double runTime = endTime - startTime;
            totalTime += runTime;
            if (success && minesweeper->solverCorrectness()) correct++;
            if (!minesweeper->solverCorrectness()) {
                minesweeper->printBoard();
                printf("mines left: %d\n", minesweeper->minesLeft);
            }
            incorrectGuesses += minesweeper->wrongGuesses;
        }
        printf("Number of correct times: %d\n", correct);
        printf("Average incorrect guesses: %.2f\n", incorrectGuesses / 100.f);
        printf("Average run time: %.2f ms\n", 1000.f * (totalTime / 50));
    }

    return 0;
}