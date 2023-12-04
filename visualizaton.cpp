#include <stdio.h> 
#include <SFML/Graphics.hpp> 
#include <algorithm> 
#include <iostream> 
#include <string> 

using namespace std; 

int inside_list(vector<sf::Vector2i> listt, int x, int y) {
    for (int i = 0; i < listt.size(); i++) { 
        if (listt[i].x == x && listt[i].y == y) { 
            return i; 
        }
    } 
    return -1; 
} 

void print_list(vector<sf::Vector2i> listt) {
    for (auto p : listt) {
        cout << "(" << p.x << ", " << p.y << ") "; 
    } 
    cout << endl; 
} 

int countAdjacentBombs(int x, int y, int mineMap[10][10]) {
    int count = 0;
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            if (x + i >= 0 && x + i < 10 && y + j >= 0 && y + j < 10) {
                count += mineMap[x + i][y + j];
            }
        }
    }
    return count;
} 

int main() {
    // Create the main window 
    const int canvas_height = 800; 
    const int canvas_width = 600; 
    sf::RenderWindow window(sf::VideoMode(canvas_height, canvas_width), "Minesweeper"); 

    // Set up the grid
    const int height = 10; 
    const int width = 10; 
    float cellSize = 30.f; // size of each cell in the grid 
    sf::RectangleShape grid[height][width]; 
    sf::Color customGray(128, 128, 128); 
    sf::Font font; 
    if (!font.loadFromFile("C:/Users/steve/TheImportantFolderNo.2/CMU_PhD_lecture_resources/15418/open-sans/OpenSans-Regular.ttf")) {
        printf("Cannot load font, terminating the interface now"); 
        exit(-1); 
    } 

    // a static mine map which can be later replaced by the randomly generated mine 
    int minemap[10][10] = {
        {0, 0, 1, 0, 0, 0, 1, 0, 0, 0},
        {1, 1, 1, 0, 0, 0, 0, 0, 1, 1},
        {0, 0, 0, 1, 0, 1, 0, 0, 0, 0},
        {1, 0, 0, 0, 0, 0, 1, 0, 0, 1},
        {0, 0, 0, 1, 1, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 1, 1, 1, 0, 0, 0},
        {0, 0, 1, 1, 1, 0, 1, 0, 0, 1},
        {1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 0, 1, 0, 0, 0, 1},
        {0, 1, 0, 0, 1, 0, 0, 0, 1, 0}
    }; 

    vector<sf::Text> boxtoptext; 

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            // sf::RectangleShape cell(sf::Vector2f(cellSize, cellSize)); 
            // sf::RectangleShape cell = grid[i][j]; 
            grid[i][j].setSize(sf::Vector2f(cellSize, cellSize));
            grid[i][j].setFillColor(customGray); 
            grid[i][j].setOutlineColor(sf::Color::Black);
            grid[i][j].setOutlineThickness(1.f);
            grid[i][j].setPosition(i * cellSize, j * cellSize + 50);
        }
    }

    // Set up the restart button
    sf::RectangleShape restartButton(sf::Vector2f(100.f, 50.f)); 
    restartButton.setFillColor(sf::Color::Yellow); 
    restartButton.setPosition(350.f, 5.f); // Position the button at the bottom 
    restartButton.setOutlineColor(sf::Color::Black); 
    restartButton.setOutlineThickness(1.f); 
    sf::Text restartText; 
    restartText.setCharacterSize(20); 
    restartText.setFont(font); 
    restartText.setFillColor(sf::Color::Black); 
    restartText.setPosition(360.f, 30.f); 
    restartText.setString("Restart"); 

    int dead_count = 0; 
    sf::Text deadCount; 
    deadCount.setCharacterSize(20); 
    deadCount.setFont(font); 
    deadCount.setFillColor(sf::Color::Black); 
    deadCount.setPosition(30.f, 500.f); 
    deadCount.setString("Dead Count: " + to_string(dead_count)); 
    
    int progress_bar = 0;
    sf::Text progressbar;
    progressbar.setCharacterSize(20);
    progressbar.setFont(font);
    progressbar.setFillColor(sf::Color::Black);
    progressbar.setPosition(30.f, 550.f);
    progressbar.setString("Progress: " + to_string(progress_bar) + "/100"); 

    vector<sf::Vector2i> modified_grid_idx; 

    // Start the game loop
    while (window.isOpen()) {
        // Process events
        sf::Event event;
        while (window.pollEvent(event)) {
            // Close window: exit
            if (event.type == sf::Event::Closed) {
                window.close();
            } 
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) { 

                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    std::cout << "On Click" << std::endl; 
                    // Check if the restart button is clicked
                    if (restartButton.getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                        // Restart logic goes here 
                        modified_grid_idx.clear(); 
                        boxtoptext.clear(); 
                        dead_count = 0; 
                        for (int i = 0; i < height; i++) {
                            for (int j = 0; j < width; j++) {
                                grid[i][j].setFillColor(customGray);
                            } 
                        } 
                        break; 
                    }

                    // Check if a cell in the grid is clicked 
                    int i = int(mousePos.x / cellSize); 
                    int j = int((mousePos.y - 50.f) / cellSize); 
                    // auto it = std::find(modified_grid_idx.begin(), modified_grid_idx.end(), sf::Vector2i(i, j)); 
                    if (inside_list(modified_grid_idx, i, j) == -1) { 
                        sf::Text text; 
                        text.setFont(font); 
                        text.setCharacterSize(15); 
                        text.setFillColor(sf::Color::Black); 
                        if (minemap[i][j] == 1) {
                            // bomb 
                            modified_grid_idx.push_back(sf::Vector2i(i, j)); 
                            text.setString("X"); 
                            grid[i][j].setFillColor(sf::Color::Red); 
                            text.setPosition(i * cellSize, j * cellSize + 50); 
                            boxtoptext.push_back(text); 
                            // modified_grid_idx.push_back(sf::Vector2i(i, j)); 
                            dead_count = dead_count + 1; 
                        }
                        else { 
                            for (int ki = -1; ki <= 1; ki++) {
                                for (int kj = -1; kj <= 1; kj++) {
                                    if (i + ki >= 0 && i + ki < 10 && j + kj >= 0 && j + kj < 10) {
                                        int xx = i + ki;
                                        int yy = j + kj; 
                                        if (minemap[xx][yy] == 1)
                                            continue; 
                                        int surrounding_count = countAdjacentBombs(xx, yy, minemap); 
                                        printf("surrounding count: %d\n", surrounding_count);
                                        text.setString(to_string(surrounding_count));
                                        grid[xx][yy].setFillColor(sf::Color::Cyan); 
                                        text.setPosition(xx * cellSize, yy * cellSize + 50); 
                                        boxtoptext.push_back(text); 
                                        modified_grid_idx.push_back(sf::Vector2i(xx, yy)); 
                                    }
                                }
                            } 
                        } 
                        cout << text.getString().getData() << endl; 
                    }
                    else {
                        std::cout << "already found" << std::endl; 
                        print_list(modified_grid_idx); 
                    } 

                    std::cout << i << " " << j << std::endl; 
                    /* 
                    for (int i = 0; i < height; i++) {
                        for (int j = 0; j < height; j++) {
                            if (grid[i][j].getGlobalBounds().contains(mousePos.x, mousePos.y)) {
                                // Logic for when a cell is clicked
                                // For example, change the cell color 
                                modified_grid_idx.push_back(sf::Vector2i(i, j)); 
                                grid[i][j].setFillColor(sf::Color::Red); 
                                std::cout << i << " " << j << std::endl; 
                            }
                        }
                    } 
                    */ 
                } 
                else if (event.mouseButton.button == sf::Mouse::Right) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window); 
                    int i = int(mousePos.x / cellSize);
                    int j = int((mousePos.y - 50.f) / cellSize); 
                    if (inside_list(modified_grid_idx, i, j) == -1) {
                        modified_grid_idx.push_back(sf::Vector2i(i, j));
                        sf::Text text;
                        text.setFont(font);
                        text.setCharacterSize(15);
                        text.setFillColor(sf::Color::Black);
                        text.setString("M"); 
                        text.setPosition(i * cellSize, j * cellSize + 50);
                        cout << text.getString().getData() << endl;
                        boxtoptext.push_back(text);
                    }
                    else {
                        std::cout << "already found" << std::endl;
                        print_list(modified_grid_idx);
                    } 
                    grid[i][j].setFillColor(sf::Color::Yellow); 
                }
            } 
        }

        // Clear screen
        window.clear(sf::Color::White); 
        // std::cout << "We got here" << std::endl; 

        // Draw the grid
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                window.draw(grid[i][j]);
            }
        } 

        for (auto text : boxtoptext) {
            window.draw(text); 
        } 

        // Draw the restart button
        window.draw(restartButton); 

        window.draw(restartText); 

        deadCount.setString("Dead Count: " + to_string(dead_count)); 
        window.draw(deadCount); 

        progress_bar = modified_grid_idx.size(); 
        progressbar.setString("Progress: " + to_string(progress_bar) + "/100"); 
        window.draw(progressbar); 

        // Update the window
        window.display(); 
    }

    return 0;
} 
