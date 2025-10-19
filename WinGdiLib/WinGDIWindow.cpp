/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include <windows.h>
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Chars.h"
#include "WinGDIWindow.h"


CChars ErrorToString(void)
{
    DWORD       uiError;
    char        szError[256];
    CChars      sz;

    uiError = GetLastError();
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, uiError, 0, szError, 256, NULL);
    sz.Init(szError);
    sz.Replace("\r\n", "");
    return sz;
}


LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    switch (uiMessage)
    {
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_PAINT: 
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // Basic GDI drawing: Fill a rectangle with a color
            HBRUSH brush = CreateSolidBrush(RGB(0, 128, 255)); // Blue color
            FillRect(hdc, &ps.rcPaint, brush);
            DeleteObject(brush);
            EndPaint(hWnd, &ps);
            return 0;
        }
    }
    return DefWindowProcA(hWnd, uiMessage, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::Init(HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow, const char* szWindowClass, const char* szWindowTitle)
{
    WNDCLASSA   sWindowClass;
    bool        bResult;
    ATOM        uiAtom;
    CChars      szError;

    CNativeWindow::Init(szWindowTitle);
    
    mszWindowClass.Init(szWindowClass);
    mhInstance = hInstance;
    mhPrevInstance = mhPrevInstance;

    memset(&sWindowClass, 0, sizeof(WNDCLASSA));

    sWindowClass.lpfnWndProc = WindowProc;
    sWindowClass.hInstance = hInstance;
    sWindowClass.lpszClassName = szWindowClass;
    sWindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    sWindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    uiAtom = RegisterClassA(&sWindowClass);
    if (uiAtom == 0)
    {
        szError = ErrorToString();
        gcLogger.Error2(__METHOD__, " RegisterClassA failed [", szError.Text(), "].", NULL);
        szError.Kill();
        return false;
    }

    // Create the window
    HWND hWnd = CreateWindowExA(0,                             // Optional window styles
                                mszWindowClass.Text(),         // Window class
                                mszWindowTitle.Text(),            // Window title
                                WS_OVERLAPPEDWINDOW,           // Window style
                                CW_USEDEFAULT, CW_USEDEFAULT,  // Position
                                800, 600,                      // Size
                                NULL,                          // Parent window
                                NULL,                          // Menu
                                hInstance,                     // Instance handle
                                NULL                           // Additional application data
    );

    if (hWnd == NULL) 
    {
        szError = ErrorToString();
        gcLogger.Error2(__METHOD__, " CreateWindowExA failed [", szError.Text(), "].", NULL);
        szError.Kill();
        return false;
    }

    bResult = ShowWindow(hWnd, nCmdShow);
    if (bResult)
    {
        szError = ErrorToString();
        gcLogger.Error2(__METHOD__, " ShowWindow failed [", szError.Text(), "].", NULL);
        szError.Kill();
        return false;
    }

    bResult = UpdateWindow(hWnd);
    if (!bResult)
    {
        szError = ErrorToString();
        gcLogger.Error2(__METHOD__, " UpdateWindow failed [", szError.Text(), "].", NULL);
        szError.Kill();
        return false;
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Kill(void)
{
    UnregisterClassA(mszWindowClass.Text(), mhInstance);
    mszWindowClass.Kill();
    CNativeWindow::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::Run(void)
{
    bool        bResult;
    LRESULT     lResult;
    CChars      szError;

    // Message loop
    MSG sMessage = { 0 };
    while (GetMessageA(&sMessage, NULL, 0, 0))
    {
        bResult = TranslateMessage(&sMessage);
        lResult = DispatchMessageA(&sMessage);

        if (bResult)
        {
            szError = ErrorToString();
            gcLogger.Error2(__METHOD__, " TranslateMessage failed [", szError.Text(), "].", NULL);
            szError.Kill();
            return false;
        }
    }
    return true;
}

