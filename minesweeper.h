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
    int *mineLocations;
    int *solverMineLocations;

    Minesweeper(int height, int width, int mines);
    void boardSetup(int x, int y);
    bool performMove(int x, int y);
    int neighboringMines(int x, int y);
    int unknownTiles(int x, int y);
    bool isValid(int x, int y);
    void doubleTap(int x, int y);
    void deduceMines(int x, int y);
    void newGame();
    bool solverCorrectness();

    void printBoard();

    bool sequentialSolver();
    bool openmpSolver();
};