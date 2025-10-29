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
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Chars.h"
#include "BaseLib/DebugOutput.h"
#include "SupportLib/Rectangle.h"
#include "WindowLib/Window.h"
#include "WinGDIWindowFactory.h"
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
void CWinGDIWindow::Init(CWindow * pcWindow, CNativeWindowFactory * pcWindowFactory, HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow, const char* szWindowClass)
{
    CNativeWindow::Init(pcWindow, pcWindowFactory);
    
    mszWindowClass.Init(szWindowClass);
    mhInstance = hInstance;
    mhPrevInstance = NULL;
    miCmdShow = nCmdShow;
    mhWnd = NULL;
    mhLastDC = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Kill(void)
{
    mpcWindow->DestroyCanvas();

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
void CWinGDIWindow::Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight)
{
    PAINTSTRUCT     sPaintStruct;
    HDC             hDC;
    CWinGDICanvas*  pcGDICanvas;

    pcGDICanvas = (CWinGDICanvas*)pcNativeCanvas;

    hDC = BeginPaint(mhWnd, &sPaintStruct);
    BitBlt(hDC, 0, 0, iWidth, iHeight, pcGDICanvas->GetMemDC(), 0, 0, SRCCOPY);

    EndPaint(mhWnd, &sPaintStruct);
    mhLastDC = hDC;
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
                return ERROR_SUCCESS;
            }
            else
            {
                return ERROR_INVALID_PARAMETER;
            }
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return ERROR_SUCCESS;
        }
        case WM_PAINT:
        {
            pcWindow->CNativeWindow::Present();
            return ERROR_SUCCESS;
        }
        case WM_ERASEBKGND:
        {
            return 1;
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
    WNDCLASSA               sWindowClass;
    bool                    bResult;
    ATOM                    uiAtom;
    CChars                  szError;
    CWinGDIWindowFactory*   pcFactory;

    pcFactory = (CWinGDIWindowFactory*)mpcWindowFactory;

    memset(&sWindowClass, 0, sizeof(WNDCLASSA));

    sWindowClass.lpfnWndProc = WindowProc;
    sWindowClass.hInstance = mhInstance;
    sWindowClass.lpszClassName = mszWindowClass.Text();
    sWindowClass.hCursor = LoadCursorA(NULL, IDC_ARROW);
    sWindowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    uiAtom = RegisterClassA(&sWindowClass);
    ERROR_CHECK(szError, uiAtom == 0, "RegisterClassA");

    mhWnd = CreateWindowExA(0,                             // Optional window styles
                            mszWindowClass.Text(),         // Window class
                            mpcWindow->GetWindowTitle(),   // Window title
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

    pcFactory->SetHWnd(mhWnd);

    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::ExecuteNativeWindow(void)
{
    bool        bResult;
    CChars      szError;
    MSG         sMessage;
    bool        bHasMessage;
    
    memset(&sMessage, 0, sizeof(MSG));
    bHasMessage = PeekMessageA(&sMessage, NULL, 0, 0, PM_REMOVE);
    if (bHasMessage)
    {
        if (sMessage.message == WM_QUIT)
        {
            return false;
        }
        bResult = TranslateMessage(&sMessage);
        ERROR_CHECK(szError, bResult, "TranslateMessage");

        DispatchMessageA(&sMessage);
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::SignalPresent(void)
{
    InvalidateRect(mhWnd, NULL, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::GetRectangle(CRectangle* pcDest)
{
    RECT            cRect;

    GetClientRect(mhWnd, &cRect);
    pcDest->Init(cRect.left, cRect.top, cRect.right, cRect.bottom);
}

