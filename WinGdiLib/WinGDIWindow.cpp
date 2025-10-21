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
#include "SupportLib/Rectangle.h"
#include "WinGDIWindow.h"


#define ERROR_CHECK(p, q, r) \
if (q) \
{ \
    p = ErrorToString(); \
    if (!p.Empty()) \
    { \
        gcLogger.Error2(__METHOD__, " "##r##" failed [", p.Text(), "].", NULL); \
        p.Kill(); \
        return false; \
    } \
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Init(HINSTANCE hInstance, HINSTANCE hLastInstance, int nCmdShow, const char* szWindowClass, const char* szWindowTitle)
{
    CNativeWindow::Init(szWindowTitle);
    
    mszWindowClass.Init(szWindowClass);
    mhInstance = hInstance;
    mhLastInstance = NULL;
    miCmdShow = nCmdShow;
    mhWnd = NULL;
    mhLastDC = NULL;
    mcLastRectangle.Init(-1, -1, -1, -1);
    mhMemDC = NULL;
    mhMemBitmap = NULL;
    mhOldBitmap = NULL;
    mbPainting = false;

    miX = 0;
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
CChars ErrorToString(void)
{
    DWORD       uiError;
    char        szError[256];
    CChars      sz;

    uiError = GetLastError();
    if (uiError != ERROR_SUCCESS)
    {
        FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, uiError, 0, szError, 256, NULL);
        sz.Init(szError);
        sz.Replace("\r\n", "");
    }
    else
    {
        sz.Init();
    }
    return sz;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Tick(void)
{
    miX += 2;
    if (miX > 350)
    {
        miX -= 350;
    }

    InvalidateRect(mhWnd, NULL, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Draw(void)
{
    PAINTSTRUCT     sPaintStruct;
    HDC             hDC;
    RECT            cRect;
    CRectangle      cRectangle;
    int32           iWidth;
    int32           iHeight;

    if (!mbPainting)
    {
        mbPainting = true;
        hDC = BeginPaint(mhWnd, &sPaintStruct);

        GetClientRect(mhWnd, &cRect);
        cRectangle.Init(cRect.left, cRect.top, cRect.right, cRect.bottom);
        iWidth = cRectangle.GetWidth();
        iHeight = cRectangle.GetHeight();

        if (hDC != mhLastDC)
        {
            if (!mcLastRectangle.Equals(&cRectangle))
            {
                if (mhMemDC)
                {
                    // Clean up
                    SelectObject(mhMemDC, mhOldBitmap);
                    DeleteObject(mhMemBitmap);
                    DeleteDC(mhMemDC);
                }

                // Create double-buffering resources
                mhMemDC = CreateCompatibleDC(hDC);
                mhMemBitmap = CreateCompatibleBitmap(hDC, iWidth, iHeight);
                mhOldBitmap = (HBITMAP)SelectObject(mhMemDC, mhMemBitmap);
            }
        }

        // Fill background in memory DC
        FillRect(mhMemDC, &cRect, (HBRUSH)(COLOR_WINDOW + 1));

        // Draw a 10x10 grid of colored pixels in memory DC
        for (int x = 50; x < 150; x += 10)
        {
            for (int y = 50; y < 150; y += 10)
            {
                SetPixel(mhMemDC, x + miX, y, RGB(255, 0, 0)); // Red pixels
            }
        }

        // Copy memory DC to window DC
        BitBlt(hDC, 0, 0, iWidth, iHeight, mhMemDC, 0, 0, SRCCOPY);

        EndPaint(mhWnd, &sPaintStruct);
        mhLastDC = hDC;
        mcLastRectangle.Init(&cRectangle);

        mbPainting = false;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMessage, WPARAM wParam, LPARAM lParam)
{
    CWinGDIWindow*  pcWindow;
    CREATESTRUCTA*  pCreateStruct = (CREATESTRUCTA*)lParam;

    pCreateStruct = (CREATESTRUCTA*)lParam;

    pcWindow = (CWinGDIWindow*)GetWindowLongPtrA(hWnd, GWLP_USERDATA);

    switch (uiMessage)
    {
    case WM_CREATE:
    {
        pCreateStruct = (CREATESTRUCTA*)lParam;
        pcWindow = (CWinGDIWindow*)pCreateStruct->lpCreateParams;
        if (pcWindow)
        {
            SetWindowLongPtrA(hWnd, GWLP_USERDATA, (LONG_PTR)pcWindow);
            SetTimer(hWnd, 1, 1000 / 60, NULL);
            return ERROR_SUCCESS;
        }
        else
        {
            return ERROR_INVALID_PARAMETER;
        }
    }
    case WM_TIMER: 
    {
        pcWindow->Tick();
        return 0;
    }
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        return 0;
    }
    case WM_PAINT:
    {
        pcWindow->Draw();
        return 0;
    }
    }
    return DefWindowProcA(hWnd, uiMessage, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::CreateNativeWindow(void)
{
    WNDCLASSA   sWindowClass;
    bool        bResult;
    ATOM        uiAtom;
    CChars      szError;

    memset(&sWindowClass, 0, sizeof(WNDCLASSA));

    sWindowClass.lpfnWndProc = WindowProc;
    sWindowClass.hInstance = mhInstance;
    sWindowClass.lpszClassName = mszWindowClass.Text();;
    sWindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    sWindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    uiAtom = RegisterClassA(&sWindowClass);
    ERROR_CHECK(szError, uiAtom == 0, "RegisterClassA");

    mhWnd = CreateWindowExA(0,                             // Optional window styles
                            mszWindowClass.Text(),         // Window class
                            mszWindowTitle.Text(),         // Window title
                            WS_OVERLAPPEDWINDOW,           // Window style
                            CW_USEDEFAULT, CW_USEDEFAULT,  // Position
                            800, 600,                      // Size
                            NULL,                          // Parent window
                            NULL,                          // Menu
                            mhInstance,                    // Instance handle
                            this);
    ERROR_CHECK(szError, mhWnd == NULL, "CreateWindowExA");

    bResult = ShowWindow(mhWnd, miCmdShow);
    ERROR_CHECK(szError, bResult, "ShowWindow");

    bResult = UpdateWindow(mhWnd);
    ERROR_CHECK(szError, !bResult, "UpdateWindow");

    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::ExecuteNativeWindow(void)
{
    bool        bResult;
    LRESULT     lResult;
    CChars      szError;
    MSG         sMessage;
    
    memset(&sMessage, 0, sizeof(MSG));
    while (GetMessageA(&sMessage, NULL, 0, 0))
    {
        bResult = TranslateMessage(&sMessage);
        ERROR_CHECK(szError, bResult, "TranslateMessage");

        lResult = DispatchMessageA(&sMessage);

    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::Show(void)
{
    bool   bResult;

    bResult = CreateNativeWindow();
    if (!bResult)
    {
        return false;
    }

    bResult = ExecuteNativeWindow();
    return bResult;
}

