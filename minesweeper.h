using namespace std;

class Minesweeper {
public:
    int height;
    int width;
    int mines;
    int minesLeft;
    int initialX;
    int initialY;
    int **board;
    int **solverboard;
    int **solverboard2;
    int *mineLocations;
    int *solverMineLocations;
    int wrongGuesses;

    Minesweeper(int height, int width, int mines);
    void boardSetup(int x, int y);
    void performMove(int x, int y);
    int neighboringMines(int x, int y);
    int unknownTiles(int x, int y);
    bool isValid(int x, int y);
    void doubleTap(int x, int y);
    void deduceMines(int x, int y);
    void newGame();
    bool solverCorrectness();
    int totalUnknown();

    int openmpNeighboringMines(int x, int y);
    int openmpUnknownTiles(int x, int y);
    void openmpDoubleTap(int x, int y);
    void openmpDeduceMines(int x, int y);
    int openmpTotalUnknown();
    void openmpPerformMove(int x, int y);

    void printBoard();

    bool sequentialSolver();
    bool openmpSolver();
};