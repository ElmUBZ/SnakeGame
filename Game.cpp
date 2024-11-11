#include <iostream>
#include <windows.h>
#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string> 

using namespace std; 

struct HighScore {
	string name;
	int score;
};

// Colors
const int COLOR_DEFAULT = 7;
const int COLOR_FRUIT = 12;
const int COLOR_SNAKE = 10;
const int COLOR_NEW_HS = 6;

// Global Variables
bool gameOver;
const int height = 20;
const int width = 20;
int x, y, fruitX, fruitY, score;
int tailX[100], tailY[100];
int nTail;
enum direction { STOP = 0, LEFT, RIGHT, UP, DOWN };
direction dir;
int sleepTime = 100;
int highScore = 0; 
bool babyMode; 

// Function Prototypes
void newGame();
void drawInitialBoard();
void drawElement(int x, int y, char element);
void eraseSnake();
void drawSnakeAndFruit();
void updateScore();
void input();
void brain();
void selectDifficulty();
void manageHighScores(const string& playerName);
void displayHighScore();

// Initialize the game
void newGame() {
	srand(time(0));
	gameOver = false;
	dir = STOP;
	x = width / 2;
	y = height / 2;
	fruitX = rand() % width;
	fruitY = rand() % height;
	score = 0;
	nTail = 0;
}

// High Score Management Functions
void readHighScores(vector<HighScore>& highScores) {
	ifstream file("./highscores.txt");
	if (!file.is_open()) return;

	HighScore hs;
	while (file >> hs.name >> hs.score) {
		highScores.push_back(hs);
	}
	file.close();
}

void writeHighScores(const vector<HighScore>& highScores) {
	ofstream file("highscores.txt");
	for (const auto& hs : highScores) {
		file << hs.name << " " << hs.score << endl;
	}
	file.close();
}

void manageHighScores(const string& playerName) {
	vector<HighScore> highScores;
	readHighScores(highScores);

	// Add the new score
	highScores.push_back({ playerName, score });

	// Sort high scores in descending order
	sort(highScores.begin(), highScores.end(), [](const HighScore& a, const HighScore& b) {
		return a.score > b.score;
		});

	// Keep only the top 10 scores
	if (highScores.size() > 10) {
		highScores.resize(10);
	}

	// Write the updated high scores back to file
	writeHighScores(highScores);

	// Update the global highScore
	if (!highScores.empty()) {
		highScore = highScores[0].score;
	}
}

void displayHighScore() {
	COORD scorePos = { 0, height + 2 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), scorePos);
	cout << "High Score: " << highScore << "    "; 
}

void drawInitialBoard() {
	for (int i = 0; i < width + 2; i++) cout << "#";
	cout << endl;
	for (int i = 0; i < height; i++) {
		cout << "#";
		for (int j = 0; j < width; j++) cout << " ";
		cout << "#" << endl;
	}
	for (int i = 0; i < width + 2; i++) cout << "#";
	cout << endl;
}

void drawElement(int x, int y, char element) {
	COORD coord;
	coord.X = x + 1;  // Adjust for left border
	coord.Y = y + 1;  // Adjust for top border
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Set color based on the element type
	if (element == 'F') {
		SetConsoleTextAttribute(hConsole, COLOR_FRUIT);  // Set color to red for fruit
	}
	else if (element == 'O' || element == 'o') {
		SetConsoleTextAttribute(hConsole, COLOR_SNAKE);  // Set color to green for snake
	}
	else {
		SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);  // Default color for other elements
	}

	SetConsoleCursorPosition(hConsole, coord);
	cout << element;

	// Reset to default color after drawing
	SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);
}

void eraseSnake() {
	drawElement(x, y, ' ');
	for (int i = 0; i < nTail; i++) {
		drawElement(tailX[i], tailY[i], ' ');
	}
}

void drawSnakeAndFruit() {
	drawElement(x, y, 'O');
	for (int i = 0; i < nTail; i++) {
		drawElement(tailX[i], tailY[i], 'o');
	}
	drawElement(fruitX, fruitY, 'F');
}

void updateScore() {
	COORD scorePos = { 0, height + 3 };  
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), scorePos);
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	if( score > highScore)
		SetConsoleTextAttribute(hConsole, COLOR_NEW_HS); 

	cout << "Score: " << score << "    ";
	SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);

}

void input() {
	if (_kbhit()) {
		switch (_getch()) {
		case 'a': case 75: dir = LEFT; break;
		case 'w': case 72: dir = UP; break;
		case 's': case 80: dir = DOWN; break;
		case 'd': case 77: dir = RIGHT; break;
		case 'x': gameOver = true; break;
		}
	}
}

void brain() {
	int prevX = tailX[0];
	int prevY = tailY[0];
	int prevvX, prevvY;
	tailX[0] = x;
	tailY[0] = y;
	for (int i = 1; i < nTail; i++) {
		prevvX = tailX[i];
		prevvY = tailY[i];
		tailX[i] = prevX;
		tailY[i] = prevY;
		prevX = prevvX;
		prevY = prevvY;
	}
	switch (dir) {
	case LEFT:  x--; break;
	case RIGHT: x++; break;
	case UP:    y--; break;
	case DOWN:  y++; break;
	default:    break;
	}

	if (babyMode){
		if (x >= width) x = 0; else if (x < 0) x = width - 1;
		if (y >= height) y = 0; else if (y < 0) y = height - 1;
	}
	if (!babyMode) {
		if (x < 0 || x > width || y > height || y < 0)
			gameOver = true; 
	}
	for (int i = 0; i < nTail; i++) {
		if (tailX[i] == x && tailY[i] == y) gameOver = true;
	}
	if (x == fruitX && y == fruitY) {
		score += 10;
		fruitX = rand() % width;
		fruitY = rand() % height;
		nTail++;
	}
}

void selectDifficulty() {
	int choice;
	cout << " __        __   _                            _          _   _            \n";
	cout << " \\ \\      / /__| | ___ ___  _ __ ___   ___  | |_ ___   | |_| |__   ___   \n";
	cout << "  \\ \\ /\\ / / _ \\ |/ __/ _ \\| '_ ` _ \\ / _ \\ | __/ _ \\  | __| '_ \\ / _ \\  \n";
	cout << "   \\ V  V /  __/ | (_| (_) | | | | | |  __/ | || (_) | | |_| | | |  __/  \n";
	cout << " ___\\_/\\_/ \\___|_|\\___\\___/|_| |_| |_|\\___|  \\__\\___/   \\__|_| |_|\\___|  \n";
	cout << "/ ___| _ __   __ _| | _____   / ___| __ _ _ __ ___   ___| |              \n";
	cout << "\\___ \\| '_ \\ / _` | |/ / _ \\ | |  _ / _` | '_ ` _ \\ / _ \\ |              \n";
	cout << " ___) | | | | (_| |   <  __/ | |_| | (_| | | | | | |  __/_|              \n";
	cout << "|____/|_| |_|\\__,_|_|\\_\\___|  \\____|\\__,_|_| |_| |_|\\___(_)             \n";
	cout << endl << endl;

	cout << "#####################################################" << endl;
	cout << "##                      To Play:                   ##" << endl;
	cout << "## - - - - - - - - - - - - - - - - - - - - - - - - ##" << endl;
	cout << "##                                                 ##" << endl;
	cout << "##   Use AWSD or the arrows to control the snake   ##" << endl;
	cout << "##                                                 ##" << endl;
	cout << "##       Walls can't kill you in easy mode         ##" << endl;
	cout << "##                                                 ##" << endl;
	cout << "##               Eat Fruit to Grow!!               ##" << endl;
	cout << "##                                                 ##" << endl;
	cout << "#####################################################" << endl << endl;

	cout << "Select Difficulty Level (1- Easy, 2- Medium, 3- Hard): ";
	cin >> choice;
	switch (choice) {
	case 1: sleepTime = 150; babyMode = true;  break;
	case 2: sleepTime = 100;  babyMode = false;  break;
	case 3: sleepTime = 75; babyMode = false; break;
	default: sleepTime = 130; babyMode = true;  break;
	}
	system("cls");
	drawInitialBoard();
}

int main() {
	string playerName;
	cout << "Enter your name: ";
	cin >> playerName;

	selectDifficulty();
	newGame();

	// Read the current high score from the file
	vector<HighScore> highScores;
	readHighScores(highScores);
	if (!highScores.empty()) {
		highScore = highScores[0].score; 
	}

	while (!gameOver) {
		eraseSnake();
		input();
		brain();
		drawSnakeAndFruit();
		updateScore();
		displayHighScore(); 
		Sleep(sleepTime);
	}
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	for (int i = 0; i < 4; i++) {
		// Move cursor to the bottom of the screen
		SetConsoleCursorPosition(hConsole, { 0, height + 5 });
		SetConsoleTextAttribute(hConsole, 12);  // Red color
		cout << "GAME OVER";
		Sleep(500);
		// Clear the "GAME OVER" message
		SetConsoleCursorPosition(hConsole, { 0, height + 5 });
		cout << "          ";  // Clear text
		Sleep(500);
	}

	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), { 0, height + 6 });

	if (score > highScore) {
		SetConsoleTextAttribute(hConsole, COLOR_NEW_HS);
		cout << "Final Score: " << score << endl;
		Sleep(500); 
		cout << "NEW HIGH SCORE!" << endl; 
	}
	else
		cout << "Final Score: " << score << endl;

	manageHighScores(playerName); // Update the high scores
	SetConsoleTextAttribute(hConsole, COLOR_DEFAULT);

	cout << "\nScore board updated!" << endl << endl;
	Sleep(500);

	if (score >= highScore) {
		cout << "High Score: " << score << " | " << playerName << endl << endl;
	}
	else {
		cout << "High Score: " << highScore << " | " << highScores[0].name << endl << endl;
	}
	return 0;
}
