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
    int **mineLocations;
<<<<<<< HEAD
    int **solverMineLocations;
=======
    int **solverMineLocations; 
>>>>>>> bb9cefe04a6c98ad8284919044af4390a45dfdd2
    Minesweeper(int height, int width, int mines);
    void boardSetup(int x, int y);
    bool performMove(int x, int y);
    int neighboringMines(int x, int y);
    int unknownTiles(int x, int y);
    bool isValid(int x, int y);
<<<<<<< HEAD
    void doubleTap(int x, int y);
    void deduceMines(int x, int y);

    bool sequentialSolver();
    bool openmpSolver();
=======
    void boardSetup(int x, int y); 
>>>>>>> bb9cefe04a6c98ad8284919044af4390a45dfdd2
};