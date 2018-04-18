#pragma once

#include <cstdlib>

#define WIN32_NO_STATUS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>  // Always include this header when develop Win32 GUI application
#include <shellapi.h> // For ShellAbout function
#include <tchar.h>    // If UNICODE defined TCHAR is wchar_t else TCHAR is char

#include "resource.h"
#include "board.h"

// Window Style:
//  WS_OVERLAPPED = has title bar and border
// ~WS_THICKFRAME = disable the sizing border (can't be resized)
// ~WS_MAXIMIZEBOX = disable the maximize button
static constexpr DWORD wndStyle = WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX;

auto CALLBACK TimesDlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)->INT_PTR;