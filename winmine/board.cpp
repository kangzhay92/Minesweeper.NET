#include "main.h"
#include <random>
#include <chrono>

// Initialize the pseudo-random generator
// we use mersenne twister instead of stdlib rand(),
// because it give us better random number.
static auto seed = std::chrono::system_clock::now().time_since_epoch().count();
static std::mt19937_64 gen_rand{static_cast<unsigned long long>(seed)}; // like srand(time(nullptr))

Board::~Board()
{
	Destroy();
}

auto Board::Init(HWND hWnd, HINSTANCE hInst) -> void
{
	this->hWnd = hWnd;
	this->hInst = hInst;

	// Load images from resource file
	hMinesBMP = LoadBitmap(this->hInst, MAKEINTRESOURCE(IDI_MINES));
	hFacesBMP = LoadBitmap(this->hInst, MAKEINTRESOURCE(IDI_FACES));
	hLedsBMP = LoadBitmap(this->hInst, MAKEINTRESOURCE(IDI_LEDS));

	// Load properties from registry
	LoadProperties();

	// Update main menu item
	HMENU hMenu = GetMenu(this->hWnd);
	CheckMenuItem(hMenu, IDM_BEGINNER + difficulty, MF_CHECKED);
	if (isMarkQ) {
		CheckMenuItem(hMenu, IDM_MARKQ, MF_CHECKED);
	} else {
		CheckMenuItem(hMenu, IDM_MARKQ, MF_UNCHECKED);
	}

	// Check the board properties, not greater/less than
	// the maximum/minimum value
	CheckLevel();
}

auto Board::ShiftBetween(LONG* x, LONG* y, LONG a, LONG b) -> void
{
	if (*x < a) {
		*y += a - *x;
		*x = a;
	} else if (*y > b) {
		*x -= *y - b;
		*y = b;
	}
}

auto Board::MoveOnScreen(RECT* rect) -> void
{
	// Find nearest monitor
	HMONITOR hMonitor = MonitorFromRect(rect, MONITOR_DEFAULTTONEAREST);

	// ... and move it into working area (excluding taskbar)
	MONITORINFO mi;
	mi.cbSize = sizeof(mi);
	GetMonitorInfo(hMonitor, &mi);

	ShiftBetween(&rect->left, &rect->right, mi.rcWork.left, mi.rcWork.right);
	ShiftBetween(&rect->top, &rect->bottom, mi.rcWork.top, mi.rcWork.bottom);
}

auto Board::Create() -> void
{
	// Reset game parameters
	boxesLeft = cols * rows - mines;
	numFlags = 0;

	// Create the boxes...
	// Actually create them with an empty border,
	// so special care doesn't have to be taken on the edges
	for (unsigned col = 0; col <= cols + 1; col++) {
		for (unsigned row = 0; row <= rows + 1; row++) {
			box[col][row].isPressed = FALSE;
			box[col][row].isMine = FALSE;
			box[col][row].flagType = NORMAL;
			box[col][row].numMines = 0;
		}
	}

	// Set the minefield size
	width = cols * MINE_WIDTH + BOARD_WMARGIN * 2;
	height = rows * MINE_HEIGHT + LED_HEIGHT + BOARD_HMARGIN * 3;

	// Set the minefield position
	int left = BOARD_WMARGIN;
	int top = BOARD_HMARGIN * 2 + LED_HEIGHT;
	int right = left + cols * MINE_WIDTH;
	int bottom = top + rows * MINE_HEIGHT;
	minesRect = {left, top, right, bottom};

	// Set the face button position
	left = width / 2 - FACE_WIDTH / 2;
	top = BOARD_HMARGIN;
	right = left + FACE_WIDTH;
	bottom = top + FACE_HEIGHT;
	faceRect = {left, top, right, bottom};

	// Set the timer display position
	left = BOARD_WMARGIN;
	top = BOARD_HMARGIN;
	right = left + LED_WIDTH * 3;
	bottom = top + LED_HEIGHT;
	timerRect = {left, top, right, bottom};

	// Set the counter display position
	left = width - BOARD_WMARGIN - LED_WIDTH * 3;
	top = BOARD_HMARGIN;
	right = width - BOARD_WMARGIN;
	bottom = top + LED_HEIGHT;
	counterRect = {left, top, right, bottom};

	// Wait until action from brainware
	status = GameStatus::WAITING;
	faceImg = FaceImage::SMILE;
	time = 0;

	// Set window size and position
	RECT wndRect{pos.x, pos.y, pos.x + static_cast<LONG>(width), pos.y + static_cast<LONG>(height)};
	AdjustWindowRect(&wndRect, wndStyle, TRUE);

	// Make sure the window is completely on the screen
	MoveOnScreen(&wndRect);
	MoveWindow(hWnd, wndRect.left, wndRect.top, wndRect.right - wndRect.left, wndRect.bottom - wndRect.top, TRUE);
	RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
}

auto Board::Destroy() -> void
{
	// Destroy the drawing objects before exit the program
	DeleteObject(hFacesBMP);
	DeleteObject(hLedsBMP);
	DeleteObject(hMinesBMP);
}

auto Board::DrawLeds(HDC hDC, HDC hMemDC, int number, int x, int y) -> void
{
	// LED display index
	//
	// +-----------+
	// | 0 | 1 | 2 |
	// +-----------+
	unsigned led[3];

	// if led[2] > 9 reset led[2] to 0
	// then increase value of led[1],
	// and so on.
	if (number < 1000) {
		if (number >= 0) {
			led[0] = number / 100;
			number -= led[0] * 100;
		} else {
			led[0] = 10; // negative sign
			number = -number;
		}
		led[1] = number / 10;
		number -= led[1] * 10;
		led[2] = number;
	} else {
		for (int i = 0; i < 3; i++) {
			led[i] = 10;
		}
	}

	// Select the LED image
	HGDIOBJ hOldObj = SelectObject(hMemDC, hLedsBMP);

	// Draw the numbers based on image strip
	for (int i = 0; i < 3; i++) {
		BitBlt(hDC, i * LED_WIDTH + x, y, LED_WIDTH, LED_HEIGHT, hMemDC, 0, led[i] * LED_HEIGHT, SRCCOPY);
	}

	// End draw
	SelectObject(hMemDC, hOldObj);
}

auto Board::DrawFace(HDC hDC, HDC hMemDC) -> void
{
	// Select the face image
	HGDIOBJ hOldObj = SelectObject(hMemDC, hFacesBMP);

	// Draw face image based on image strip index
	BitBlt(hDC, faceRect.left, faceRect.top, FACE_WIDTH, FACE_HEIGHT, hMemDC, 0, faceImg * FACE_HEIGHT, SRCCOPY);

	// End draw
	SelectObject(hMemDC, hOldObj);
}

auto Board::DrawMine(HDC hDC, HDC hMemDC, unsigned col, unsigned row, BOOL isPressed) -> void
{
	MineImage offset = MineImage::BOX;

	// Sanity check
	if (col == 0 || col > cols || row == 0 || row > rows) {
		return;
	}

	if (status == GAMEOVER) {  // hit a mine?
		if (box[col][row].isMine) {
			switch (box[col][row].flagType) {
				case FLAG:
					offset = MineImage::FLAG;
					break;
				case COMPLETE:
					offset = MineImage::EXPLODE;
					break;
				case QUESTION:
				case NORMAL:
					offset = MineImage::MINE;
					break;
			}
		} else {
			switch (box[col][row].flagType) {
				case QUESTION:
					offset = MineImage::QUESTION;
					break;
				case FLAG:
					offset = MineImage::WRONG;
					break;
				case NORMAL:
					offset = MineImage::BOX;
					break;
				case COMPLETE:
				default:
					break;
			}
		}
	} else { // WAITING/PLAYING
		switch (box[col][row].flagType) {
			case QUESTION:
				if (!isPressed) {
					offset = MineImage::QUESTION;
				} else {
					offset = MineImage::QPRESS;
				}
				break;
			case FLAG:
				offset = MineImage::FLAG;
				break;
			case NORMAL:
				if (!isPressed) {
					offset = MineImage::BOX;
				} else {
					offset = MineImage::MPRESS;
				}
				break;
			case COMPLETE:
			default:
				break;
		}
	}

	if (box[col][row].flagType == COMPLETE && !box[col][row].isMine) {
		offset = static_cast<MineImage>(box[col][row].numMines);
	}

	// Draw the images based on image index
	BitBlt(hDC, (col - 1) * MINE_WIDTH + minesRect.left, (row - 1) * MINE_HEIGHT + minesRect.top, MINE_WIDTH, MINE_HEIGHT, hMemDC, 0, offset * MINE_HEIGHT, SRCCOPY);
}

auto Board::DrawMines(HDC hDC, HDC hMemDC) -> void
{
	// Select the mines image
	HGDIOBJ hOldObj = SelectObject(hMemDC, hMinesBMP);

	// Iterate over minefield and draw boxes inside it
	for (unsigned row = 1; row <= rows; row++) {
		for (unsigned col = 1; col <= cols; col++) {
			DrawMine(hDC, hMemDC, col, row, FALSE);
		}
	}

	// End draw
	SelectObject(hMemDC, hOldObj);
}

auto Board::Draw(HDC hDC, HDC hMemDC, PAINTSTRUCT* ps) -> void
{
	RECT temp;

	// Draw counter display
	if (IntersectRect(&temp, &ps->rcPaint, &counterRect)) {
		DrawLeds(hDC, hMemDC, mines - numFlags, counterRect.left, counterRect.top);
	}

	// Draw timer display
	if (IntersectRect(&temp, &ps->rcPaint, &timerRect)) {
		DrawLeds(hDC, hMemDC, time, timerRect.left, timerRect.top);
	}

	// Draw face button
	if (IntersectRect(&temp, &ps->rcPaint, &faceRect)) {
		DrawFace(hDC, hMemDC);
	}

	// Draw the minefield
	if (IntersectRect(&temp, &ps->rcPaint, &minesRect)) {
		DrawMines(hDC, hMemDC);
	}
}

auto Board::UnpressBox(unsigned col, unsigned row) -> void
{
	// Get drawing objects
	HDC hDC = GetDC(hWnd);
	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOldObj = SelectObject(hMemDC, hMinesBMP);

	// Draw the image at the selected box
	DrawMine(hDC, hMemDC, col, row, FALSE);

	// Release drawing objects
	SelectObject(hMemDC, hOldObj);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);
}

auto Board::UnpressBoxes(unsigned col, unsigned row) -> void
{
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			UnpressBox(col + i, row + j);
		}
	}
}

auto Board::PressBox(unsigned col, unsigned row) -> void
{
	// Get drawing objects
	HDC hDC = GetDC(hWnd);
	HDC hMemDC = CreateCompatibleDC(hDC);
	HGDIOBJ hOldObj = SelectObject(hMemDC, hMinesBMP);

	// Draw the pressed image at the selected box
	DrawMine(hDC, hMemDC, col, row, TRUE);

	// Release drawing objects
	SelectObject(hMemDC, hOldObj);
	DeleteDC(hMemDC);
	ReleaseDC(hWnd, hDC);
}

auto Board::PressBoxes(unsigned col, unsigned row) -> void
{
	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			box[col + i][row + j].isPressed = TRUE;
			PressBox(col + i, row + j);
		}
	}

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			if (!box[press.x + i][press.y + j].isPressed) {
				UnpressBox(press.x + i, press.y + j);
			}
		}
	}

	for (int i = -1; i <= 1; i++) {
		for (int j = -1; j <= 1; j++) {
			box[col + i][row + j].isPressed = FALSE;
			PressBox(col + i, row + j);
		}
	}

	press = {static_cast<LONG>(col), static_cast<LONG>(row)};
}

auto Board::CompleteBox(unsigned col, unsigned row) -> void
{
	if (box[col][row].flagType != COMPLETE && box[col][row].flagType != FLAG && col > 0 && col < cols + 1 && row > 0 && row < rows + 1) {
		box[col][row].flagType = COMPLETE;

		// Clicking box contain a mine, game over...
		if (box[col][row].isMine) {
			faceImg = FaceImage::DEAD;
			status = GameStatus::GAMEOVER;
		} else if (status != GameStatus::GAMEOVER) {
			boxesLeft--;
		}

		// Recursively complete the box if this is an empty box
		if (box[col][row].numMines == 0) {
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					CompleteBox(col + i, row + j);
				}
			}
		}
	}
}

auto Board::CompleteBoxes(unsigned col, unsigned row) -> void
{
	unsigned flagCount = 0;

	if (box[col][row].flagType == COMPLETE) {
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				if (box[col + i][row + j].flagType == FLAG) {
					flagCount++;
				}
			}
		}
		if (flagCount == box[col][row].numMines) {
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if (box[col + i][row + j].flagType != FLAG) {
						CompleteBox(col + i, row + j);
					}
				}
			}
		}
	}
}

auto Board::AddFlag(unsigned col, unsigned row) -> void
{
	if (box[col][row].flagType != COMPLETE) {
		switch (box[col][row].flagType) {
			// If mark question is enabled, change box into question
			// else switch to normal
			case FLAG:
				if (isMarkQ) {
					box[col][row].flagType = QUESTION;
				} else {
					box[col][row].flagType = NORMAL;
				}
				numFlags--;
				break;

			// Fallback into normal box
			case QUESTION:
				box[col][row].flagType = NORMAL;
				break;

			// Else change box into flagged
			default:
				box[col][row].flagType = FLAG;
				numFlags++;
		}
	}
}

auto Board::PlaceMines(int selectedCol, int selectedRow) -> void
{
	// Temporary place a mine at the selected box until all the other
	// mines are placed. This avoids checking in the mine creation loop.
	box[selectedCol][selectedRow].isMine = TRUE;

	// Spread the mines using our random number generator
	int i = 0, j;
	unsigned col, row;

	while (static_cast<unsigned>(i) < mines) {
		col = gen_rand() % cols + 1;  // get random value between 0...cols+1
		row = gen_rand() % rows + 1;  // get random value between 0...rows+1

		// Place mine into box position 
		if (!box[col][row].isMine) {
			i++;
			box[col][row].isMine = TRUE;
		}
	}

	// Remove temporarily placed mine for selected box
	box[selectedCol][selectedRow].isMine = FALSE;

	// Now label the remaining boxes with the
	// number of mines surrounding them
	for (col = 1; col < cols + 1; col++) {
		for (row = 1; row < rows + 1; row++) {
			for (i = -1; i <= 1; i++) {
				for (j = -1; j <= 1; j++) {
					if (box[col + i][row + j].isMine) {
						box[col][row].numMines++;
					}
				}
			}
		}
	}
}

auto Board::TestFace(POINT pt, int msg) -> void
{
	// Update the button image based
	// on game state
	if (status == GameStatus::PLAYING || status == GameStatus::WAITING) {
		if (msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN) {
			faceImg = FaceImage::OOH;
		} else {
			faceImg = FaceImage::SMILE;
		}
	} else if (status == GameStatus::GAMEOVER) {
		faceImg = FaceImage::DEAD;
	} else if (status == GameStatus::WON) {
		faceImg = FaceImage::COOL;
	}

	// If we press the button, change image to pressed button
	// and trigger new game when release button.
	if (PtInRect(&faceRect, pt)) {
		if (msg == WM_LBUTTONDOWN) {
			faceImg = FaceImage::SPRESS;
		}
		if (msg == WM_LBUTTONUP) {
			Create();
		}
	}

	// Don't forget to redraw the face button
	RedrawWindow(hWnd, &faceRect, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
}

auto Board::TestMines(POINT pt, int msg) -> void
{
	int col = (pt.x - minesRect.left) / MINE_WIDTH + 1;
	int row = (pt.y - minesRect.top) / MINE_HEIGHT + 1;
	BOOL draw = TRUE;

	switch (msg) {
		///////////////////////////////////
		// Left Button = Open single box //
		///////////////////////////////////
		case WM_LBUTTONDOWN:
			if (press.x != col || press.y != row) {
				UnpressBox(press.x, press.y);
				press = {static_cast<LONG>(col), static_cast<LONG>(row)};
				PressBox(col, row);
			}
			draw = FALSE;
			break;

		case WM_LBUTTONUP:
			if (press.x != col || press.y != row) {
				UnpressBox(press.x, press.y);
			}
			press = {0, 0};
			if (box[col][row].flagType != FLAG && status != GameStatus::PLAYING) {
				status = GameStatus::PLAYING;
				PlaceMines(col, row);
			}
			CompleteBox(col, row);
			break;

		////////////////////////////////////////////////////////////////////
		// Middle Button + Left Button = Open multiple boxes (Game Trick) //
		////////////////////////////////////////////////////////////////////
		case WM_MBUTTONDOWN:
			PressBoxes(col, row);
			draw = FALSE;
			break;

		case WM_MBUTTONUP:
			if (press.x != col || press.y != row) {
				UnpressBoxes(press.x, press.y);
			}
			press = {0, 0};
			CompleteBoxes(col, row);
			break;

		///////////////////////////////////////////
		// Right Button = Add Flag/Question Mark //
		///////////////////////////////////////////
		case WM_RBUTTONDOWN:
			AddFlag(col, row);
			break;

		default:
			break;
	}

	if (draw) {
		// Redraw entire window
		RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
	}
}

// Congratulations dialog procedure
auto CALLBACK CongratsDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) -> INT_PTR
{
	static Board* pBoard;

	switch (uMsg) {
		case WM_INITDIALOG:
			pBoard = reinterpret_cast<Board*>(lParam);
			SetDlgItemText(hDlg, IDC_EDITNAME, pBoard->bestName[pBoard->difficulty]);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					GetDlgItemText(hDlg, IDC_EDITNAME, pBoard->bestName[pBoard->difficulty], sizeof(pBoard->bestName[pBoard->difficulty]) / sizeof(TCHAR));
					EndDialog(hDlg, 0);
					return TRUE;

				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
			break;
	}

	return FALSE;
}

auto Board::Test(HWND hWnd, int x, int y, int msg) -> void
{
	POINT pt{x, y};  // Point for testing

	// Update the minefield
	if (PtInRect(&minesRect, pt) && status != GameStatus::GAMEOVER && status != GameStatus::WON) {
		TestMines(pt, msg);
	} else {
		UnpressBoxes(press.x, press.y);
		press = {0, 0};
	}

	// We beat the game, congratulations!!!
	if (boxesLeft == 0) {
		status = GameStatus::WON;

		// Change all remaining boxes into flag
		if (numFlags < mines) {
			for (unsigned row = 1; row <= rows; row++) {
				for (unsigned col = 1; col <= cols; col++) {
					if (box[col][row].isMine && box[col][row].flagType != FLAG) {
						box[col][row].flagType = FLAG;
					}
				}
			}
			numFlags = mines;
			RedrawWindow(hWnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
		}

		// If in beginner/advanced/expert mode, and
		// beat the best time. Show congratulations
		// and best times dialog. And replace the old one.
		if (difficulty != Difficulty::CUSTOM && time < bestTime[difficulty]) {
			bestTime[difficulty] = time;
			DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_CONGRATS), hWnd, CongratsDlgProc, reinterpret_cast<LPARAM>(this));
			DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_TIMES), hWnd, TimesDlgProc, reinterpret_cast<LPARAM>(this));
		}
	}

	// Update face button
	TestFace(pt, msg);
}

// Custom dialog procedure
static auto CALLBACK CustomDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) -> INT_PTR
{
	BOOL isRet;
	static Board* pBoard;

	switch (uMsg) {
		case WM_INITDIALOG:
			// Get current board properties
			pBoard = reinterpret_cast<Board*>(lParam);
			SetDlgItemInt(hDlg, IDC_EDITROWS, pBoard->rows, FALSE);
			SetDlgItemInt(hDlg, IDC_EDITCOLS, pBoard->cols, FALSE);
			SetDlgItemInt(hDlg, IDC_EDITMINES, pBoard->mines, FALSE);
			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
					// Update board size
					pBoard->rows = GetDlgItemInt(hDlg, IDC_EDITROWS, &isRet, FALSE);
					pBoard->cols = GetDlgItemInt(hDlg, IDC_EDITCOLS, &isRet, FALSE);
					pBoard->mines = GetDlgItemInt(hDlg, IDC_EDITMINES, &isRet, FALSE);
					pBoard->CheckLevel();
					EndDialog(hDlg, 0);
					return TRUE;
				case IDCANCEL:
					EndDialog(hDlg, 1);
					return TRUE;
			}
			break;
	}

	return FALSE;
}

auto Board::SetDifficulty(Difficulty difficulty) -> void
{
	// Show the Custom Dialog if in custom mode
	if (difficulty == Difficulty::CUSTOM) {
		if (DialogBoxParam(hInst, MAKEINTRESOURCE(DLG_CUSTOM), hWnd, CustomDlgProc, reinterpret_cast<LPARAM>(this)) != 0) {
			return;
		}
	}

	// Update menu item
	HMENU hMenu = GetMenu(hWnd);
	CheckMenuItem(hMenu, IDM_BEGINNER + this->difficulty, MF_UNCHECKED);
	this->difficulty = difficulty;
	CheckMenuItem(hMenu, IDM_BEGINNER + difficulty, MF_CHECKED);

	// Update board size
	switch (difficulty) {
		case Difficulty::BEGINNER:
			cols = BEGINNER_COLS;
			rows = BEGINNER_ROWS;
			mines = BEGINNER_MINES;
			break;
		case Difficulty::ADVANCED:
			cols = ADVANCED_COLS;
			rows = ADVANCED_ROWS;
			mines = ADVANCED_MINES;
			break;
		case Difficulty::EXPERT:
			cols = EXPERT_COLS;
			rows = EXPERT_ROWS;
			mines = EXPERT_MINES;
			break;
		case Difficulty::CUSTOM:
			break;
	}
}

// WinReg Stuff
// If you search in regedit, you will find it
// in HKEY_CURRENT_USER/Software/Microsoft/WinMine
static constexpr TCHAR regKey[] = {'S','o','f','t','w','a','r','e','\\','M','i','c','r','o','s','o','f','t','\\','W','i','n','M','i','n','e',0};
static constexpr TCHAR xPosStr[] = {'X','p','o','s',0};
static constexpr TCHAR yPosStr[] = {'Y','p','o','s',0};
static constexpr TCHAR heightStr[] = {'H','e','i','g','h','t',0};
static constexpr TCHAR widthStr[] = {'W','i','d','t','h',0};
static constexpr TCHAR minesStr[] = {'M','i','n','e','s',0};
static constexpr TCHAR difficultyStr[] = {'D','i','f','f','i','c','u','l','t','y',0};
static constexpr TCHAR markStr[] = {'M','a','r','k',0};
static constexpr TCHAR nameStr[] = {'N','a','m','e','%','u',0};
static constexpr TCHAR timeStr[] = {'T','i','m','e','%','u',0};

auto Board::LoadProperties() -> void
{
	HKEY hKey;
	DWORD type;

	// Open registry entry
	RegOpenKeyEx(HKEY_CURRENT_USER, regKey, 0, KEY_QUERY_VALUE, &hKey);

	///////////////////////////////
	// Load values from registry //
	///////////////////////////////

	DWORD size = sizeof(pos.x);
	if (RegQueryValueEx(hKey, xPosStr, nullptr, &type, reinterpret_cast<BYTE*>(&pos.x), &size)) {
		pos.x = 0;
	}

	size = sizeof(pos.y);
	if (RegQueryValueEx(hKey, yPosStr, nullptr, &type, reinterpret_cast<BYTE*>(&pos.y), &size)) {
		pos.y = 0;
	}

	size = sizeof(rows);
	if (RegQueryValueEx(hKey, heightStr, nullptr, &type, reinterpret_cast<BYTE*>(&rows), &size)) {
		rows = BEGINNER_ROWS;
	}

	size = sizeof(cols);
	if (RegQueryValueEx(hKey, widthStr, nullptr, &type, reinterpret_cast<BYTE*>(&cols), &size)) {
		cols = BEGINNER_COLS;
	}

	size = sizeof(mines);
	if (RegQueryValueEx(hKey, minesStr, nullptr, &type, reinterpret_cast<BYTE*>(&mines), &size)) {
		mines = BEGINNER_MINES;
	}

	size = sizeof(difficulty);
	if (RegQueryValueEx(hKey, difficultyStr, nullptr, &type, reinterpret_cast<BYTE*>(&difficulty), &size)) {
		difficulty = BEGINNER;
	}

	size = sizeof(isMarkQ);
	if (RegQueryValueEx(hKey, markStr, nullptr, &type, reinterpret_cast<BYTE*>(&isMarkQ), &size)) {
		isMarkQ = TRUE;
	}

	TCHAR keyName[8];
	TCHAR data[MAX_PLAYER_NAME_SIZE + 1];

	for (unsigned i = 0; i < 3; i++) {
		wsprintf(keyName, nameStr, i + 1);
		size = sizeof(data);
		if (RegQueryValueEx(hKey, keyName, nullptr, &type, reinterpret_cast<BYTE*>(data), &size) == ERROR_SUCCESS) {
			lstrcpyn(bestName[i], data, sizeof(bestName[i]) / sizeof(TCHAR));
		} else {
			LoadString(hInst, IDS_NOBODY, bestName[i], MAX_PLAYER_NAME_SIZE + 1);
		}
	}

	for (unsigned i = 0; i < 3; i++) {
		wsprintf(keyName, timeStr, i + 1);
		size = sizeof(bestTime[i]);
		if (RegQueryValueEx(hKey, keyName, nullptr, &type, reinterpret_cast<BYTE*>(&bestTime[i]), &size)) {
			bestTime[i] = 999;
		}
	}

	// Close registry handle
	RegCloseKey(hKey);
}

auto Board::SaveProperties() -> void
{
	HKEY hKey;

	// Create registry entry
	if (RegCreateKeyEx(HKEY_CURRENT_USER, regKey, 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr) != ERROR_SUCCESS) {
		return;
	}

	////////////////////////////////
	// Write values into registry //
	////////////////////////////////

	RegSetValueEx(hKey, xPosStr, 0, REG_DWORD, reinterpret_cast<BYTE*>(&pos.x), sizeof(pos.x));
	RegSetValueEx(hKey, yPosStr, 0, REG_DWORD, reinterpret_cast<BYTE*>(&pos.y), sizeof(pos.y));
	RegSetValueEx(hKey, difficultyStr, 0, REG_DWORD, reinterpret_cast<BYTE*>(&difficulty), sizeof(difficulty));
	RegSetValueEx(hKey, heightStr, 0, REG_DWORD, reinterpret_cast<BYTE*>(&rows), sizeof(rows));
	RegSetValueEx(hKey, widthStr, 0, REG_DWORD, reinterpret_cast<BYTE*>(&cols), sizeof(cols));
	RegSetValueEx(hKey, minesStr, 0, REG_DWORD, reinterpret_cast<BYTE*>(&mines), sizeof(mines));
	RegSetValueEx(hKey, markStr, 0, REG_DWORD, reinterpret_cast<BYTE*>(&isMarkQ), sizeof(isMarkQ));

	TCHAR keyName[8];
	TCHAR data[MAX_PLAYER_NAME_SIZE + 1];

	for (unsigned i = 0; i < 3; i++) {
		wsprintf(keyName, nameStr, i + 1);
		lstrcpyn(data, bestName[i], sizeof(data) / sizeof(TCHAR));
		RegSetValueEx(hKey, keyName, 0, REG_SZ, reinterpret_cast<BYTE*>(data), (lstrlen(data) + 1) * sizeof(TCHAR));
	}

	for (unsigned i = 0; i < 3; i++) {
		wsprintf(keyName, timeStr, i + 1);
		RegSetValueEx(hKey, keyName, 0, REG_DWORD, reinterpret_cast<BYTE*>(&bestTime[i]), sizeof(bestTime[i]));
	}

	// Close registry handle
	RegCloseKey(hKey);
}

auto Board::CheckLevel() -> void
{
	if (rows < BEGINNER_ROWS) {
		rows = BEGINNER_ROWS;
	}
	if (rows > MAX_ROWS) {
		rows = MAX_ROWS;
	}
	if (cols < BEGINNER_COLS) {
		cols = BEGINNER_COLS;
	}
	if (cols > MAX_COLS) {
		cols = MAX_COLS;
	}
	if (mines < BEGINNER_MINES) {
		mines = BEGINNER_MINES;
	}
	if (mines > (cols - 1) * (rows - 1)) {
		mines = (cols - 1) * (rows - 1);
	}
}
