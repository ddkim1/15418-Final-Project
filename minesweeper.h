using namespace std;

class Minesweeper {
public:
    int height;
    int width;
    int mines;
    int minesLeft;
    int **board;
    int **solverboard;
    int **mineLocations;
    int **solverMineLocations; 
    Minesweeper(int height, int width, int mines);
    void placeMines(int x, int y);
    void sequentialSolver();
    bool isValid(int x, int y);
    void boardSetup(int x, int y); 
};