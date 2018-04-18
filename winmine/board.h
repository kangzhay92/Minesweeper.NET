#pragma once

// Beginner Mode (9x9, 10 Mines)
static constexpr unsigned BEGINNER_MINES = 10;
static constexpr unsigned BEGINNER_ROWS  = 9;
static constexpr unsigned BEGINNER_COLS  = 9;

// Advanced Mode (16x16, 40 Mines)
static constexpr unsigned ADVANCED_MINES = 40;
static constexpr unsigned ADVANCED_COLS  = 16;
static constexpr unsigned ADVANCED_ROWS  = 16;

// Expert Mode (30x16, 99 Mines)
static constexpr unsigned EXPERT_MINES   = 99;
static constexpr unsigned EXPERT_COLS    = 30;
static constexpr unsigned EXPERT_ROWS    = 16;

// Maximum size of the minefield (30x24 boxes)
static constexpr unsigned MAX_COLS = 30;
static constexpr unsigned MAX_ROWS = 24;

// Board margins size
static constexpr unsigned BOTTOM_MARGIN = 20;
static constexpr unsigned BOARD_WMARGIN = 5;
static constexpr unsigned BOARD_HMARGIN = 5;

// Image size (in pixel)
static constexpr unsigned MINE_WIDTH  = 16;
static constexpr unsigned MINE_HEIGHT = 16;
static constexpr unsigned LED_WIDTH   = 12;
static constexpr unsigned LED_HEIGHT  = 23;
static constexpr unsigned FACE_WIDTH  = 24;
static constexpr unsigned FACE_HEIGHT = 24;

// Maximum player name is 31 characters
static constexpr unsigned MAX_PLAYER_NAME_SIZE = 31;

enum GameStatus
{
	WAITING,
	PLAYING,
	GAMEOVER,
	WON
};

enum Difficulty
{
	BEGINNER,
	ADVANCED,
	EXPERT,
	CUSTOM
};

//////////////////////////////////////
// Consecutive based on image strip //
//////////////////////////////////////
enum FaceImage
{
	SPRESS,
	COOL,
	DEAD,
	OOH,
	SMILE
};

enum MineImage
{
	MPRESS,
	ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT,
	BOX, FLAG, QUESTION, EXPLODE, 
	WRONG, MINE, QPRESS
};


// Our main board
class Board
{
public:
	Board() {}
	~Board();

	auto Init(HWND hWnd, HINSTANCE hInst) -> void;
	auto Create() -> void;
	auto Draw(HDC hDC, HDC hMemDC, PAINTSTRUCT* ps) -> void;
	auto Destroy() -> void;

	auto Test(HWND hWnd, int x, int y, int msg) -> void;
	auto SetDifficulty(Difficulty difficulty) -> void;

	// Load/save board properties from registry
	auto LoadProperties() -> void;
	auto SaveProperties() -> void;
	auto CheckLevel() -> void;

	// Native window handles
	HWND hWnd;
	HINSTANCE hInst;

	// Images for use in our board
	HBITMAP hMinesBMP; // Game cell images (mines, flag, numbers, and question)
	HBITMAP hFacesBMP; // For face button
	HBITMAP hLedsBMP;  // For the LED displays in left and right the face button

	// Board properties
	POINT pos;                                   // Window position
	unsigned rows;                               // Number of minefield rows
	unsigned cols;                               // Number of minefield columns
	unsigned mines;                              // Number of mines
	Difficulty difficulty;                       // Game difficulty
	BOOL isMarkQ;                                // Enable/disable mark question mode
	TCHAR bestName[3][MAX_PLAYER_NAME_SIZE + 1]; // Best player name record
	DWORD bestTime[3];                           // Best time record

	POINT press;                                 // Mouse press position
	GameStatus status;                           // Game status

	RECT minesRect;    // Minefield position
	RECT faceRect;     // Face button position
	RECT timerRect;    // Timer display position
	RECT counterRect;  // Counter display position

	unsigned time;     // Game time
	
private:
	// Helper functions
	static auto MoveOnScreen(RECT* rect) -> void;
	static auto ShiftBetween(LONG* x, LONG* y, LONG a, LONG b) -> void;

	// Drawing functions
	auto DrawLeds(HDC hDC, HDC hMemDC, int number, int x, int y) -> void;
	auto DrawFace(HDC hDC, HDC hMemDC) -> void;
	auto DrawMine(HDC hDC, HDC hMemDC, unsigned col, unsigned row, BOOL isPressed) -> void;
	auto DrawMines(HDC hDC, HDC hMemDC) -> void;

	// Game Logic functions
	auto TestFace(POINT pt, int msg) -> void;
	auto TestMines(POINT pt, int msg) -> void;
	auto PlaceMines(int selectedCol, int selectedRow) -> void;
	
	// Minefield related functions
	auto PressBox(unsigned col, unsigned row) -> void;
	auto PressBoxes(unsigned col, unsigned row) -> void;
	auto UnpressBox(unsigned col, unsigned row) -> void;
	auto UnpressBoxes(unsigned col, unsigned row) -> void;
	auto CompleteBox(unsigned col, unsigned row) -> void;
	auto CompleteBoxes(unsigned col, unsigned row) -> void;
	void AddFlag(unsigned col, unsigned row);

	// Minefield boxes
	struct Box
	{
		unsigned isMine    : 1;
		unsigned isPressed : 1;
		unsigned flagType  : 2;
		unsigned numMines  : 4;
	};
	Box box[MAX_COLS + 2][MAX_ROWS + 2];

	// Defines for box[][].flagType
	enum : unsigned { NORMAL, QUESTION, FLAG, COMPLETE };

	unsigned numFlags;  // how many flags we placed?
	unsigned boxesLeft; // how many boxes available for complete the game?
	
	// Minefield size (in pixel)
	unsigned width;
	unsigned height;

	// Face button image
	FaceImage faceImg;
};
