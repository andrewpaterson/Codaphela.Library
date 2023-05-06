/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 1998 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifdef _WIN32
#include "Define.h"
#include "WinInit.h"
#include "ErrorHandler.h"
#include "FloatHelper.h"
#include "DataTypes.h"
#include "Logger.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long FAR PASCAL WindowProcCB(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWin32Window*		pcWindow;

	pcWindow = (CWin32Window*)((size_t)GetWindowLongPtr(hWnd, GWLP_USERDATA));

	//Do nothing if the window is not yet created.
	if (pcWindow)
	{
		switch (message)
		{
		case WM_INPUT:
			if (pcWindow->mRawInputFunc)
			{
				UINT 		dwSize;
				BYTE 		lpb[40];
				RAWINPUT*	lpRawInput;

				dwSize = 40;
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

				lpRawInput = (RAWINPUT*)lpb;
				pcWindow->mRawInputFunc(pcWindow->mpvRawInputObject, lpRawInput);
			}
			break;

		case WM_PAINT:
			break;
		case WM_WINDOWPOSCHANGING:
			break;
		case WM_GETTEXT:
			break;
		case WM_ERASEBKGND:
			break;
		case WM_ACTIVATE:
			break;
		case WM_CANCELMODE:
			break;
		case WM_ENTERSIZEMOVE:
			break;
		case WM_EXITSIZEMOVE:
			break;
		case WM_CAPTURECHANGED:
			break;
		case WM_MOVING:
			break;
		case WM_MOVE:
			break;
		case WM_KILLFOCUS:
			break;
		case WM_CHAR:
			break;
		case WM_SYSKEYUP:
			break;
		case WM_SYSKEYDOWN:
			break;

		case WM_SIZE:
			if (pcWindow->mSizeFunc)
				pcWindow->mSizeFunc((int)wParam, (int)LOWORD(lParam), (int)HIWORD(lParam));
			return 0L;

		case WM_ACTIVATEAPP:
			pcWindow->mbActive = (bool)wParam;
			if (pcWindow->mActivateFunc)
				pcWindow->mActivateFunc(pcWindow->mbActive);
			return 1L;

		case WM_CLOSE:
			pcWindow->Destroy();
			break;

		case WM_DESTROY:
			if (pcWindow->mDestroyFunc)
				pcWindow->mDestroyFunc();
			PostQuitMessage(0);
			return 1L;

		case WM_WINDOWPOSCHANGED:
			if (pcWindow->mChangingFunc)
				pcWindow->mChangingFunc((WINDOWPOS*)lParam);
			return 1L;

		case WM_SHOWWINDOW:
			if (pcWindow->mShowFunc)
				pcWindow->mShowFunc((EShowType)wParam);
			break;

		case WM_KEYDOWN:
			if (pcWindow->mKeyFunc)
				pcWindow->mKeyFunc((int)wParam, KT_Down);
			return 1L;
		case WM_KEYUP:
			if (pcWindow->mKeyFunc)
				pcWindow->mKeyFunc((int)wParam, KT_Up);
			return 1L;

		case WM_SETCURSOR:
			if (pcWindow->mSetCursorFunc)
			{
				pcWindow->mSetCursorFunc();
				return 1L;
			}
			break;

		case WM_SETFOCUS:
			if (pcWindow->mSetFocusFunc)
			{
				pcWindow->mSetFocusFunc();
				return 1L;
			}
			break;
		}
	}
	return (long)DefWindowProc(hWnd, message, wParam, lParam);;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWin32Window::Init(HINSTANCE hInst, MainFunc Main, ActivateFunc Activate, ShowFunc Show, DestroyFunc Destroy, SetCursorFunc SetCursor, KeyFunc Key, SetFocusFunc SetFocus, ChangingFunc changing, SizeFunc size, RawInputFunc rawInput, void* pvRawInputObject)
{
	hWnd = NULL;
	hInstance = hInst;
	mActivateFunc = Activate;
	mShowFunc = Show;
	mDestroyFunc = Destroy;
	mSetCursorFunc = SetCursor;
	mKeyFunc = Key;
	mSetFocusFunc = SetFocus;
	mChangingFunc = changing;
	mSizeFunc = size;
	mpvRawInputObject = pvRawInputObject;
	mRawInputFunc = rawInput;
	mMainFunc = Main;
	mbActive = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWin32Window::GetSize(int* px, int* py)
{
	RECT	sRect;

	if (GetWindowRect(hWnd, &sRect))
	{
		*px = sRect.right - sRect.left;
		*py = sRect.bottom - sRect.top;
	}
	else
	{
		*px = *py = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWin32Window::GetPosition(int* px, int* py)
{
	RECT	sRect;

	if (GetWindowRect(hWnd, &sRect))
	{
		*px = sRect.left;
		*py = sRect.top;
	}
	else
	{
		*px = *py = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWin32Window::GetScreenWidth(void)
{
	return GetSystemMetrics(SM_CXSCREEN);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWin32Window::GetScreenHeight(void)
{
	return GetSystemMetrics(SM_CYSCREEN);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWin32Window::CreateFullScreenWindow(int nCmdShow, char* szName)
{
	WNDCLASS	wc;

	className.Init("DRG.");
	className.Append(szName);
	hCursor = LoadCursor(NULL, IDC_ARROW);

	clientAreaLeftOffset = 0;
	clientAreaTopOffset = 0;
	clientAreaRightOffset = 0;
	clientAreaBottomOffset = 0;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WindowProcCB;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = hCursor;
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName = szName;
	wc.lpszClassName = className.Text();
	RegisterClass(&wc);

	hWnd = CreateWindowEx(0,
		className.Text(),
		szName,
		WS_POPUP,
		0,
		0,
		GetScreenWidth(),
		GetScreenHeight(),
		NULL,
		NULL,
		hInstance,
		NULL);
	if (!hWnd)
	{
		gcUserError.Set("Could not Create Window");
		return false;
	}
	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)((size_t)this));
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	SetCursor(hCursor);
	mbActive = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWin32Window::CreateWindowedWindow(int nCmdShow, char* szName, int xp, int yp, int xs, int ys)
{
	WNDCLASS	wc;
	RECT		rect;
	int			iStyle;

	className.Init("DRG.");
	className.Append(szName);
	hCursor = LoadCursor(NULL, IDC_ARROW);

	iStyle = WS_POPUPWINDOW | WS_MAXIMIZEBOX | WS_CAPTION | WS_MINIMIZEBOX | WS_SIZEBOX;
	iStyle = WS_POPUPWINDOW | WS_MAXIMIZEBOX | WS_CAPTION | WS_MINIMIZEBOX | WS_SIZEBOX;
	rect.left = xp;
	rect.top = yp;
	rect.right = xp + xs;
	rect.bottom = yp + ys;
    AdjustWindowRect(&rect, iStyle, false);
	clientAreaLeftOffset = xp - rect.left;
	clientAreaTopOffset = yp - rect.top;
	clientAreaRightOffset = (xp+xs) - rect.right;
	clientAreaBottomOffset = (yp+ys) - rect.bottom;
	xs = rect.right - rect.left;
	ys = rect.bottom - rect.top;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)WindowProcCB;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = NULL;
	wc.hCursor = hCursor;
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.lpszMenuName = szName;
	wc.lpszClassName = className.Text();
	RegisterClass(&wc);
	hWnd = CreateWindowEx(0, className.Text(), szName, iStyle, xp, yp, xs, ys, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		gcUserError.Set("Could not Create Window");
		return false;
	}

	SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG)((size_t)this));
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	SetFocus(hWnd);
	SetCursor(hCursor);
	mbActive = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWin32Window::Destroy(void)
{
	mShowFunc = NULL;
	mSetCursorFunc = NULL;
	mKeyFunc = NULL;
	mSetFocusFunc = NULL;
	mChangingFunc = NULL;
	mMainFunc = NULL;
	mbActive = false;
	DestroyWindow(hWnd);
	UnregisterClass(className.Text(), hInstance);
	className.Kill();
	hWnd = NULL;
	mbLoop = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWin32Window::WinLoop(void)
{
	MSG		msg;

	mbLoop = true;
	gcLogger.Add("Start winloop...\n");

	while (mbLoop)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0))
			{
				gcLogger.Add("Shutdown winloop [window destroyed]\n");
				return (int)msg.wParam;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//If the window is active (ie: not minimised)
		if (mbActive)
		{
			//and if there is a main function.
			if (mMainFunc)
			{
				//Run the main function.
				mMainFunc();
			}
		}
		else
		{
			WaitMessage();
		}
	}
	gcLogger.Add("Shutdown winloop [shutdown requested]\n");
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWin32Window::GetCursorPositionInClientArea(int* px, int* py)
{
	RECT	r;
	POINT	p;

	GetWindowRect(hWnd, &r);
	GetCursorPos(&p);

	*px = p.x - r.left - clientAreaLeftOffset;
	*py = p.y - r.top - clientAreaTopOffset;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWin32Window::GetClientSize(int* px, int* py)
{
	int	xs;
	int	ys;

	GetSize(&xs, &ys);

	*px = xs-clientAreaLeftOffset+clientAreaRightOffset-1;
	*py = ys-clientAreaTopOffset+clientAreaBottomOffset-1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWin32Window::GetNormalisedCursorPos(SFloat2* psPosition)
{
	int	xp;
	int	yp;
	int xc;
	int yc;

	GetCursorPositionInClientArea(&xp, &yp);
	GetClientSize(&xc, &yc);

    psPosition->x = NormaliseFloat((float)xc, (float)xp);
	psPosition->y = NormaliseFloat((float)yc, (float)yp);
}


#endif // _WIN32

