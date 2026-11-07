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
#include <windowsx.h>
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Chars.h"
#include "BaseLib/WindowsError.h"
#include "BaseLib/DebugOutput.h"
#include "SupportLib/Int32Rect.h"
#include "WindowLib/Window.h"
#include "WindowLib/ComponentInput.h"
#include "WinGDIWindowFactory.h"
#include "WinGDIWindow.h"


#define ERROR_CHECK(p, q, r) \
if (q) \
{ \
    p = ErrorToString(); \
    if (!p.Empty()) \
    { \
        gcLogger.Error2(__METHOD__, " " r " failed [", p.Text(), "].", NULL); \
        p.Kill(); \
        return false; \
    } \
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Init(CWindow* pcWindow, CNativeWindowFactory * pcWindowFactory, HINSTANCE hInstance, HINSTANCE hPrevInstance, int nCmdShow, const char* szWindowClass)
{
    CNativeWindow::Init(pcWindow, pcWindowFactory);
    
    mszWindowClass.Init(szWindowClass);
    mhInstance = hInstance;
    mhPrevInstance = NULL;
    miCmdShow = nCmdShow;
    mhWnd = NULL;
    mhLastDC = NULL;
    mbTrackingLeave = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A window still open (its loop was stopped, not closed) is destroyed,
// let go of first: a message it is sent now, say activation as another
// program's window goes, must not reach this object.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Kill(void)
{
    if (mhWnd)
    {
        SetWindowLongPtrA(mhWnd, GWLP_USERDATA, 0);
        DestroyWindow(mhWnd);
        mhWnd = NULL;
    }
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
    char*       szError;
    CChars      sz;

    uiError = GetLastError();
    if (uiError != ERROR_SUCCESS)
    {
        szError = WindowsErrorCodeToString(uiError);
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
    pcGDICanvas->Paint(hDC, iWidth, iHeight);
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
        case WM_CLOSE:
        {
            //A tool window, a floating pane, is closed by its owner's loop seeing it has gone.
            if (pcWindow && pcWindow->IsTool())
            {
                pcWindow->DestroyNativeWindow();
                return 0;
            }
            //A dialog's close button only stops it.  CWindow::ShowModal enables the owner before it destroys the dialog,
            //otherwise Windows would activate another application's window.
            if (pcWindow && pcWindow->GetOwner())
            {
                pcWindow->Stop();
                return 0;
            }
            //The application can keep the main window open, to ask to save its changes say.
            if (pcWindow && !pcWindow->ApproveClose())
            {
                return 0;
            }
            break;
        }
        case WM_DESTROY:
        {
            //Only the main window ends the message loop, a dialog closing just stops its own.  A window Kill
            //destroys has already let go of its object.
            if (pcWindow)
            {
                pcWindow->Destroyed();
                pcWindow->Stop();
                if (!pcWindow->GetOwner())
                {
                    PostQuitMessage(0);
                }
            }
            return ERROR_SUCCESS;
        }
        case WM_PAINT:
        {
            //Until the window is first painted there is nothing to show, but the paint must still be validated or
            //Windows sends WM_PAINT again for ever.  Painting signals another present.
            if (!pcWindow || !pcWindow->Present())
            {
                ValidateRect(hWnd, NULL);
            }
            return ERROR_SUCCESS;
        }
        case WM_ERASEBKGND:
        {
            return 1;
        }
        case WM_SETCURSOR:
        {
            //The class's arrow would be put back on every move.
            if (pcWindow && (LOWORD(lParam) == HTCLIENT) && pcWindow->ShowCursor())
            {
                return TRUE;
            }
            break;
        }
        case WM_SIZE:
        {
            if (pcWindow)
            {
                pcWindow->Resized();
            }
            break;
        }
        case WM_GETMINMAXINFO:
        {
            //A resizable dialog is no smaller than its minimum client size plus its frame.
            if (pcWindow && pcWindow->GetOwner() && !pcWindow->IsTool() && pcWindow->IsResizable())
            {
                pcWindow->GetMinimumTrackSize(&((MINMAXINFO*)lParam)->ptMinTrackSize);
                return 0;
            }
            break;
        }
        case WM_INPUT:
        {
            //DefWindowProc must still be called to clean up after the raw input.
            if (pcWindow)
            {
                pcWindow->RawInput((HRAWINPUT)lParam);
            }
            break;
        }
        case WM_MOUSEMOVE:
        {
            if (pcWindow)
            {
                pcWindow->MouseMoved(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
            }
            return 0;
        }
        case WM_MOUSELEAVE:
        {
            if (pcWindow)
            {
                pcWindow->MouseLeft();
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_MBUTTONDOWN:
        {
            if (pcWindow)
            {
                pcWindow->MouseButton(hWnd, wParam, true);
            }
            return 0;
        }
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        {
            if (pcWindow)
            {
                pcWindow->MouseButton(hWnd, wParam, false);
            }
            return 0;
        }
        case WM_ACTIVATE:
        {
            //DefWindowProc must still be called to set the keyboard focus.
            if (pcWindow)
            {
                if (LOWORD(wParam) == WA_INACTIVE)
                {
                    pcWindow->Deactivated();
                }
                else
                {
                    pcWindow->Activated();
                }
            }
            break;
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
    WNDCLASSA               sExistingClass;
    RECT                    sRect;
    RECT                    sOwnerRect;
    DWORD                   dwStyle;
    HWND                    hOwner;
    int                     iX;
    int                     iY;
    int                     iWidth;
    int                     iHeight;
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

    //Every window, a dialog's too, is of the same class so it is only registered once.
    if (!GetClassInfoA(mhInstance, mszWindowClass.Text(), &sExistingClass))
    {
        uiAtom = RegisterClassA(&sWindowClass);
        ERROR_CHECK(szError, uiAtom == 0, "RegisterClassA");
    }

    //The requested size is the client area's, the window is that plus its frame.
    if (mpcOwner && mbTool)
    {
        //No frame: WindowLib draws the tool window's title and edges.
        dwStyle = WS_POPUP;
    }
    else
    {
        dwStyle = mpcOwner ? (WS_POPUP | WS_CAPTION | WS_SYSMENU) : WS_OVERLAPPEDWINDOW;
        if (mpcOwner && IsResizable())
        {
            //A dialog with a minimum size has a frame to resize it by (WM_GETMINMAXINFO keeps it from going smaller).
            dwStyle |= WS_THICKFRAME;
        }
    }
    sRect.left = 0;
    sRect.top = 0;
    sRect.right = (msRequestedSize.x > 0) ? msRequestedSize.x : 800;
    sRect.bottom = (msRequestedSize.y > 0) ? msRequestedSize.y : 600;
    AdjustWindowRect(&sRect, dwStyle, FALSE);
    iWidth = sRect.right - sRect.left;
    iHeight = sRect.bottom - sRect.top;

    //A dialog is owned by, and centred on, its owner.
    hOwner = NULL;
    iX = CW_USEDEFAULT;
    iY = CW_USEDEFAULT;
    if (mpcOwner)
    {
        hOwner = ((CWinGDIWindow*)mpcOwner)->mhWnd;
        GetWindowRect(hOwner, &sOwnerRect);
        iX = sOwnerRect.left + (sOwnerRect.right - sOwnerRect.left - iWidth) / 2;
        iY = sOwnerRect.top + (sOwnerRect.bottom - sOwnerRect.top - iHeight) / 2;
    }
    if (mbScreenRectangle)
    {
        iX = mcScreenRectangle.GetLeft();
        iY = mcScreenRectangle.GetTop();
    }

    mhWnd = CreateWindowExA(mpcOwner ? (mbTool ? (WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE) : WS_EX_DLGMODALFRAME) : 0,  // Optional window styles
                            mszWindowClass.Text(),         // Window class
                            mpcWindow->GetWindowTitle(),   // Window title
                            dwStyle,                       // Window style
                            iX, iY,                        // Position
                            iWidth, iHeight,               // Size
                            hOwner,                        // Owner window
                            NULL,                          // Menu
                            mhInstance,                    // Instance handle
                            this);
    ERROR_CHECK(szError, mhWnd == NULL, "CreateWindowExA");

    //Only the main window is shown as the program was asked to start, maximised say.
    //A tool window appears without taking the keys, dragged out of its owner say.  WS_EX_NOACTIVATE is only for
    //showing it; it can be activated by clicking it after.
    bResult = ShowWindow(mhWnd, mpcOwner ? (mbTool ? SW_SHOWNOACTIVATE : SW_SHOWNORMAL) : miCmdShow);
    ERROR_CHECK(szError, bResult, "ShowWindow");
    if (mbTool)
    {
        SetWindowLongPtrA(mhWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
    }

    bResult = UpdateWindow(mhWnd);
    ERROR_CHECK(szError, !bResult, "UpdateWindow");

    //Raw input and the canvases' compatible DCs stay the main window's, a tool window doesn't take them.
    if (!mbTool)
    {
        pcFactory->SetHWnd(mhWnd);
    }

    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::ExecuteNativeWindow(void)
{
    MSG         sMessage;
    bool        bHasMessage;
    
    //Every waiting message is handled each tick.  A mouse sends raw input far faster than the window ticks.
    memset(&sMessage, 0, sizeof(MSG));
    bHasMessage = PeekMessageA(&sMessage, NULL, 0, 0, PM_REMOVE);
    while (bHasMessage)
    {
        if (sMessage.message == WM_QUIT)
        {
            //Seen by a dialog's loop when its owner closed, so the owner's loop must see it too.
            if (mpcOwner)
            {
                PostQuitMessage(0);
            }
            return false;
        }

        //TranslateMessage returns whether it made a WM_CHAR, it doesn't fail.
        TranslateMessage(&sMessage);
        DispatchMessageA(&sMessage);
        bHasMessage = PeekMessageA(&sMessage, NULL, 0, 0, PM_REMOVE);
    }
    return mhWnd != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::CanWaitForEvents(void)
{
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Any message wakes it, raw input and WM_PAINT included, so a paint
// signalled before waiting is presented straight away.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::WaitForEvents(int64 iMilliseconds)
{
    DWORD   dwTimeout;

    if (iMilliseconds < 0)
    {
        dwTimeout = INFINITE;
    }
    else if (iMilliseconds >= (int64)INFINITE)
    {
        dwTimeout = INFINITE - 1;
    }
    else
    {
        dwTimeout = (DWORD)iMilliseconds;
    }
    MsgWaitForMultipleObjectsEx(0, NULL, dwTimeout, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
}


//////////////////////////////////////////////////////////////////////////
//
// The owner of a modal dialog is disabled so it gets no keys or clicks.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::SetEnabled(bool bEnabled)
{
    CNativeWindow::SetEnabled(bEnabled);
    if (mhWnd)
    {
        EnableWindow(mhWnd, bEnabled ? TRUE : FALSE);
    }
}


//////////////////////////////////////////////////////////////////////////
//
// WM_DESTROY calls Destroyed.  Canvases are made compatible with the
// owner's window again.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::DestroyNativeWindow(void)
{
    CWinGDIWindowFactory*   pcFactory;

    if (mhWnd)
    {
        DestroyWindow(mhWnd);
    }
    mhWnd = NULL;

    pcFactory = (CWinGDIWindowFactory*)mpcWindowFactory;
    if (mpcOwner && !mbTool)
    {
        pcFactory->SetHWnd(((CWinGDIWindow*)mpcOwner)->mhWnd);
    }
}


//////////////////////////////////////////////////////////////////////////
//
// The native window has gone, closed by its user say.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Destroyed(void)
{
    mhWnd = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::SignalPresent(void)
{
    if (mhWnd)
    {
        InvalidateRect(mhWnd, NULL, TRUE);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::GetRectangle(CInt32Rect* pcDest)
{
    RECT            cRect;

    if (mhWnd && GetClientRect(mhWnd, &cRect))
    {
        pcDest->Init(cRect.left, cRect.top, cRect.right, cRect.bottom);
    }
    else
    {
        pcDest->Init(0, 0, 0, 0);
    }
}


//////////////////////////////////////////////////////////////////////////
//
// Keyboard and mouse buttons come as raw input and go through CInput to
// the window's CComponentInput when it updates.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::RawInput(HRAWINPUT hRawInput)
{
    CWinGDIWindowFactory*   pcFactory;
    UINT                    uiSize;
    UINT                    uiRead;
    union
    {
        RAWINPUT            sRawInput;
        uint8               aucData[256];
    } uBuffer;

    uiSize = 0;
    GetRawInputData(hRawInput, RID_INPUT, NULL, &uiSize, sizeof(RAWINPUTHEADER));
    if ((uiSize == 0) || (uiSize > sizeof(uBuffer)))
    {
        //Only HID devices other than keyboards and mice send more.
        return;
    }

    uiRead = GetRawInputData(hRawInput, RID_INPUT, &uBuffer, &uiSize, sizeof(RAWINPUTHEADER));
    if (uiRead == uiSize)
    {
        pcFactory = (CWinGDIWindowFactory*)mpcWindowFactory;
        pcFactory->RawInput(&uBuffer.sRawInput);
    }
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are in client space.  They can be outside the window while a
// button is held, the mouse is captured then.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::MouseMoved(int x, int y)
{
    TRACKMOUSEEVENT     sTrack;

    if (!mbTrackingLeave)
    {
        memset(&sTrack, 0, sizeof(TRACKMOUSEEVENT));
        sTrack.cbSize = sizeof(TRACKMOUSEEVENT);
        sTrack.dwFlags = TME_LEAVE;
        sTrack.hwndTrack = mhWnd;
        mbTrackingLeave = TrackMouseEvent(&sTrack) != FALSE;
    }
    mpcWindow->PointerMoved(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::MouseLeft(void)
{
    mbTrackingLeave = false;
    mpcWindow->PointerLeft();
}


//////////////////////////////////////////////////////////////////////////
//
// The buttons themselves are read from raw input.  The mouse is captured
// while any is held so that a drag keeps getting moves outside the window.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::MouseButton(HWND hWnd, WPARAM wParam, bool bPressed)
{
    if (bPressed)
    {
        SetCapture(hWnd);
    }
    else if ((wParam & (MK_LBUTTON | MK_RBUTTON | MK_MBUTTON)) == 0)
    {
        ReleaseCapture();
    }
}


//////////////////////////////////////////////////////////////////////////
//
// The locks may have been pressed while another window had focus.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Activated(void)
{
    CComponentInput*    pcComponentInput;
    bool                bCapsLock;
    bool                bNumLock;

    pcComponentInput = mpcWindow->GetComponentInput();
    if (pcComponentInput)
    {
        bCapsLock = (GetKeyState(VK_CAPITAL) & 1) != 0;
        bNumLock = (GetKeyState(VK_NUMLOCK) & 1) != 0;
        pcComponentInput->SetLocks(bCapsLock, bNumLock);
    }
    mpcWindow->Activate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Deactivated(void)
{
    mpcWindow->Deactivate();
}


//////////////////////////////////////////////////////////////////////////
//
// The smallest a resizable dialog's window can be dragged to: its minimum
// client size with its frame around it.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::GetMinimumTrackSize(POINT* psDest)
{
    RECT    sRect;

    sRect.left = 0;
    sRect.top = 0;
    sRect.right = msMinimumSize.x;
    sRect.bottom = msMinimumSize.y;
    AdjustWindowRectEx(&sRect, (DWORD)GetWindowLongPtrA(mhWnd, GWL_STYLE), FALSE, (DWORD)GetWindowLongPtrA(mhWnd, GWL_EXSTYLE));
    psDest->x = sRect.right - sRect.left;
    psDest->y = sRect.bottom - sRect.top;
}


//////////////////////////////////////////////////////////////////////////
//
// The window is laid out again at its new size.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Resized(void)
{
    mpcWindow->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
// NULL while the window isn't on the screen.
//
//////////////////////////////////////////////////////////////////////////
HWND CWinGDIWindow::GetHWnd(void)
{
    return mhWnd;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves and resizes the client area.  A tool window has no frame so its
// window is its client area.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::SetScreenRectangle(CInt32Rect* pcRectangle)
{
    RECT    sRect;

    CNativeWindow::SetScreenRectangle(pcRectangle);
    if (mhWnd == NULL)
    {
        return;
    }
    sRect.left = pcRectangle->GetLeft();
    sRect.top = pcRectangle->GetTop();
    sRect.right = pcRectangle->GetLeft() + pcRectangle->GetWidth();
    sRect.bottom = pcRectangle->GetTop() + pcRectangle->GetHeight();
    AdjustWindowRectEx(&sRect, (DWORD)GetWindowLongPtrA(mhWnd, GWL_STYLE), FALSE, (DWORD)GetWindowLongPtrA(mhWnd, GWL_EXSTYLE));
    SetWindowPos(mhWnd, NULL, sRect.left, sRect.top, sRect.right - sRect.left, sRect.bottom - sRect.top, SWP_NOZORDER | SWP_NOACTIVATE);
    mpcWindow->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::GetScreenRectangle(CInt32Rect* pcDest)
{
    POINT   sOrigin;
    RECT    sClient;

    if ((mhWnd == NULL) || !GetClientRect(mhWnd, &sClient))
    {
        return CNativeWindow::GetScreenRectangle(pcDest);
    }
    sOrigin.x = 0;
    sOrigin.y = 0;
    ClientToScreen(mhWnd, &sOrigin);
    pcDest->Init(sOrigin.x, sOrigin.y, sOrigin.x + sClient.right, sOrigin.y + sClient.bottom);
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::GetPointerScreenPosition(SInt32Vec2* psDest)
{
    POINT   sPoint;

    if (!GetCursorPos(&sPoint))
    {
        return false;
    }
    psDest->Init(sPoint.x, sPoint.y);
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The whole virtual screen, every monitor.
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::GetScreenArea(CInt32Rect* pcDest)
{
    int     iLeft;
    int     iTop;

    iLeft = GetSystemMetrics(SM_XVIRTUALSCREEN);
    iTop = GetSystemMetrics(SM_YVIRTUALSCREEN);
    pcDest->Init(iLeft, iTop, iLeft + GetSystemMetrics(SM_CXVIRTUALSCREEN), iTop + GetSystemMetrics(SM_CYVIRTUALSCREEN));
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Shown now if the pointer is over the window, and by WM_SETCURSOR after.
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::SetCursor(ENativeCursor eCursor)
{
    if (eCursor == meCursor)
    {
        return;
    }
    CNativeWindow::SetCursor(eCursor);
    ShowCursor();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::ShowCursor(void)
{
    LPCSTR  szCursor;

    switch (meCursor)
    {
    case NC_SizeWestEast:
        szCursor = IDC_SIZEWE;
        break;
    case NC_SizeNorthSouth:
        szCursor = IDC_SIZENS;
        break;
    case NC_SizeNorthWestSouthEast:
        szCursor = IDC_SIZENWSE;
        break;
    case NC_SizeNorthEastSouthWest:
        szCursor = IDC_SIZENESW;
        break;
    case NC_Move:
        szCursor = IDC_SIZEALL;
        break;
    default:
        szCursor = IDC_ARROW;
        break;
    }
    ::SetCursor(LoadCursorA(NULL, szCursor));
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Raise(void)
{
    if (mhWnd)
    {
        SetWindowPos(mhWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    }
}


bool CWinGDIWindow::IsCreated(void) { return mhWnd != NULL; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::TitleChanged(void)
{
    if (mhWnd)
    {
        SetWindowTextA(mhWnd, mpcWindow->GetWindowTitle());
    }
}

