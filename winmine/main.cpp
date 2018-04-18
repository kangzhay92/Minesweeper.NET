#include "main.h"

// Our game board
static Board board;

// Fastest Time dialog procedure
auto CALLBACK TimesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) -> INT_PTR
{
	static Board* pBoard;

	switch (uMsg) {
		case WM_INITDIALOG:
			pBoard = reinterpret_cast<Board*>(lParam);

			// Set best names
			for (unsigned i = 0; i < 3; i++) {
				SetDlgItemText(hDlg, (IDC_NAME1) + i, pBoard->bestName[i]);
			}

			// Set best times
			for (unsigned i = 0; i < 3; i++) {
				SetDlgItemInt(hDlg, (IDC_TIME1) + i, pBoard->bestTime[i], FALSE);
			}

			return TRUE;

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hDlg, 0);
					return TRUE;
			}
			break;
	}
	return FALSE;
}

// Main window procedure
static auto CALLBACK MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT
{
	// GDI Objects
	HDC hDC;
	PAINTSTRUCT ps;

	// Menu handle
	HMENU hMenu;

	switch (msg) {
		// Called when a window be created by calling CreateWindow
		case WM_CREATE:
			// Initialize the board and start new game
			board.Init(hWnd, reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance);
			board.Create();
			return 0;

		// Called when program makes a request to paint application window
		case WM_PAINT: {
			// Redraw the board
			hDC = BeginPaint(hWnd, &ps);
			HDC hMemDC = CreateCompatibleDC(hDC);
			
			board.Draw(hDC, hMemDC, &ps);  
			
			DeleteDC(hMemDC);
			EndPaint(hWnd, &ps);

			return 0;
		}

		// Called when window has been moved
		case WM_MOVE:
			// Set the board position
			board.pos.x = static_cast<short>(LOWORD(lParam));
			board.pos.y = static_cast<short>(HIWORD(lParam));
			return 0;

		// Called when window is being destroyed
		case WM_DESTROY:
			// Release all board drawing objects, 
			// save the board properties, and safely 
			// close the program
			board.SaveProperties();
			board.Destroy();
			PostQuitMessage(EXIT_SUCCESS);
			return 0;

		// Called when application timer expires
		case WM_TIMER:
			// Update time display when game is starting
			if (board.status == GameStatus::PLAYING) {
				board.time++;
				RedrawWindow(hWnd, &board.timerRect, nullptr, RDW_INVALIDATE | RDW_UPDATENOW);
			}
			return 0;

		//////////////////
		// Mouse Events //
		//////////////////

		case WM_LBUTTONDOWN:  // Left mouse button pressed
			if (wParam & (MK_RBUTTON | MK_SHIFT)) {
				msg = WM_MBUTTONDOWN;
			}
			board.Test(hWnd, static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)), msg);
			SetCapture(hWnd);
			return 0;

		case WM_LBUTTONUP:  // Left mouse button released
			if (wParam & (MK_RBUTTON | MK_SHIFT)) {
				msg = WM_MBUTTONUP;
			}
			board.Test(hWnd, static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)), msg);
			ReleaseCapture();
			return 0;

		case WM_RBUTTONDOWN:  // Right mouse button pressed
			if (wParam & MK_LBUTTON) {
				board.press.x = 0;
				board.press.y = 0;
				msg = WM_MBUTTONDOWN;
			}
			board.Test(hWnd, static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)), msg);
			return 0;

		case WM_RBUTTONUP:  // Right mouse button released
			if (wParam & MK_LBUTTON) {
				msg = WM_MBUTTONUP;
			}
			board.Test(hWnd, static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)), msg);
			return 0;

		case WM_MBUTTONDOWN: // Middle mouse button pressed
			board.Test(hWnd, static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)), msg);
			return 0;

		case WM_MBUTTONUP: // Middle mouse button released
			board.Test(hWnd, static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)), msg);
			return 0;

		case WM_MOUSEMOVE: { // Called when mouse pointer moves
			if ((wParam & MK_MBUTTON) || ((wParam & MK_LBUTTON) && (wParam & MK_RBUTTON))) {
				msg = WM_MBUTTONDOWN;
			} else if (wParam & MK_LBUTTON) {
				msg = WM_LBUTTONDOWN;
			} else {
				return 0;
			}
			board.Test(hWnd, static_cast<short>(LOWORD(lParam)), static_cast<short>(HIWORD(lParam)), msg);
			return 0;
		}


		//////////////////////////
		// Handle menu commands //
		//////////////////////////

		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDM_NEW:  // Options -> New
					board.Create();
					return 0;

				case IDM_MARKQ: // Options -> Mark Question
					hMenu = GetMenu(hWnd);                             // Get main menu from main window
					board.isMarkQ = !board.isMarkQ;                    // Toggle on/off
					if (board.isMarkQ) {
						CheckMenuItem(hMenu, IDM_MARKQ, MF_CHECKED);   // Check menu item
					} else {
						CheckMenuItem(hMenu, IDM_MARKQ, MF_UNCHECKED); // Uncheck menu item
					}
					return 0;

				case IDM_BEGINNER: // Options -> Beginner
					board.SetDifficulty(Difficulty::BEGINNER);
					board.Create();
					return 0;

				case IDM_ADVANCED: // Options -> Advanced
					board.SetDifficulty(Difficulty::ADVANCED);
					board.Create();
					return 0;

				case IDM_EXPERT: // Options -> Expert
					board.SetDifficulty(Difficulty::EXPERT);
					board.Create();
					return 0;

				case IDM_CUSTOM: // Options -> Custom...
					board.SetDifficulty(Difficulty::CUSTOM);
					board.Create();
					return 0;

				case IDM_EXIT: // Options -> Exit
					SendMessage(hWnd, WM_CLOSE, 0, 0);
					return 0;

				case IDM_TIMES: // Info -> Fastest Times
					// Show Fastest Times dialog
					DialogBoxParam(board.hInst, MAKEINTRESOURCE(DLG_TIMES), hWnd, TimesDlgProc, reinterpret_cast<LPARAM>(&board));
					return 0;

				case IDM_ABOUT: { // Info -> About
					// Show the About your OS dialog, and our
					// program stuff in .rc file.
					// I don't create about dialog in this application,
					// because I'm a lazy programmer... :D
					TCHAR appName[256], aboutText[256];
					LoadString(board.hInst, IDS_APPNAME, appName, sizeof(appName) / sizeof(TCHAR));
					LoadString(board.hInst, IDS_ABOUT, aboutText, sizeof(aboutText) / sizeof(TCHAR));
					ShellAbout(hWnd, appName, aboutText, static_cast<HICON>(LoadImage(board.hInst, MAKEINTRESOURCE(IDI_WINMINE), IMAGE_ICON, 48, 48, LR_SHARED)));
					return 0;
				}

				default:
					break;
			}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Windows GUI application entry point, 
// main() is entry point for console application.
auto WINAPI _tWinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPWSTR cmdLine, int cmdShow) -> int
{
	// Load string from resource file (winmine.rc)
	TCHAR appName[20];
	LoadString(hInst, IDS_APPNAME, appName, sizeof(appName) / sizeof(TCHAR));
	
	// Initialize the window class
	WNDCLASSEX wc;
	wc.cbSize = sizeof(wc);
	wc.style = 0;
	wc.lpfnWndProc = MainProc;  // main window procedure, for handle window events.
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_WINMINE));               // Set application title bar icon, load from .rc file
	wc.hCursor = LoadCursor(nullptr, static_cast<LPWSTR>(IDI_APPLICATION)); // Set the application cursor, use default cursor
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wc.lpszMenuName = MAKEINTRESOURCE(IDM_WINMINE);                         // Set application main menu, load from .rc file
	wc.lpszClassName = appName;
	wc.hIconSm = static_cast<HICON>(LoadImage(hInst, MAKEINTRESOURCE(IDI_WINMINE), IMAGE_ICON, GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), LR_SHARED));

	// Register the window class for use in CreateWindow
	if (!RegisterClassEx(&wc)) {
		ExitProcess(EXIT_FAILURE);
	}

	// Creates the main window
	HWND hWnd = CreateWindow(appName, appName, wndStyle, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInst, nullptr);
	if (!hWnd) {
		ExitProcess(EXIT_FAILURE);
	}

	// Show and update the main window
	ShowWindow(hWnd, cmdShow);
	UpdateWindow(hWnd);

	// Initialize game timer
	SetTimer(hWnd, ID_TIMER, 1000, nullptr);

	// Initialize program hotkey, load accelerators from .rc file
	// This program use F2 for new game, and Alt+X for terminate program.
	MSG msg;
	HACCEL hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(IDA_WINMINE));

	// Process the window events that defined in MainProc function
	while (GetMessage(&msg, nullptr, 0, 0)) {
		if (!TranslateAccelerator(hWnd, hAccel, &msg)) { // Process the hotkeys
			TranslateMessage(&msg);
		}
		DispatchMessage(&msg);
	}

	// Safely exit the program
	return msg.wParam;
}
