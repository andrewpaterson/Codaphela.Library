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
#include "BaseLib/WindowsHeaders.h"
#include "BaseLib/Logger.h"
#include "WindowLib/ComponentFactory.h"
#include "WinGDIWindow.h"


long FAR PASCAL CWindowCB(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Init(HINSTANCE hInst, char* szName, CInput* pcInput)
{
	mszMenuName.Init(szName);
	mszWindowName.Init(szName);
	mszClassName.Init("DRG.");
	mszClassName.Append(szName);
	mhInstance = hInst;
	hWnd = NULL;
	mbActive = false;
	mpcInput = pcInput;
	mpcViewport = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Kill(void)
{
	mszWindowName.Kill();
	mszMenuName.Kill();
	mszClassName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Layout(void)
{
	mpcViewport->SetActualSize();
	mpcViewport->Layout();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::Draw(void)
{
	return mpcViewport->Draw();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::Create(int nCmdShow, int xp, int yp, int xs, int ys, int iStyle)
{
	WNDCLASS	wc;
	RECT		rect;

	hCursor = LoadCursor(NULL, IDC_ARROW);

	rect.left = xp;
	rect.top = yp;
	rect.right = xp + xs;
	rect.bottom = yp + ys;
	mcClientRect.miLeft = rect.left;
	mcClientRect.miTop = rect.top;
	mcClientRect.miRight = rect.right;
	mcClientRect.miBottom = rect.bottom;
	AdjustWindowRect(&rect, iStyle, false);
	xs = mcClientRect.GetWidth();
	ys = mcClientRect.GetHeight();

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = (WNDPROC)CWindowCB;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = mhInstance;
	wc.hIcon = NULL;
	wc.hCursor = hCursor;
	wc.hbrBackground = (HBRUSH)GetStockObject(GRAY_BRUSH);
	wc.lpszMenuName = mszMenuName.Text();
	wc.lpszClassName = mszClassName.Text();
	RegisterClass(&wc);
	hWnd = CreateWindowEx(0, mszClassName.Text(), mszWindowName.Text(), iStyle, xp, yp, xs, ys, NULL, NULL, mhInstance, NULL);

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
bool CWinGDIWindow::FullScreen(int nCmdShow)
{
	return Create(nCmdShow, 0, 0, GetScreenWidth(), GetScreenHeight(), WS_POPUP);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::Windowed(int nCmdShow, int xp, int yp, int xs, int ys)
{
	return Create(nCmdShow, xp, yp, xs, ys, WS_POPUPWINDOW | WS_MAXIMIZEBOX | WS_CAPTION | WS_MINIMIZEBOX | WS_SIZEBOX);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Destroy(void)
{
	mbActive = false;
	DestroyWindow(hWnd);
	UnregisterClass(mszClassName.Text(), mhInstance);
	hWnd = NULL;
	mbLoop = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWinGDIWindow::GetScreenWidth(void)
{
	return GetSystemMetrics(SM_CXSCREEN);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWinGDIWindow::GetScreenHeight(void)
{
	return GetSystemMetrics(SM_CYSCREEN);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWinGDIWindow::WinLoop(void)
{
	MSG		msg;

	mbLoop = true;
	gcLogger.Add("Start winloop...\n");

	int		iCount;

	iCount = 0;
	mcLoopTimer.Init();
	while (mbLoop)
	{
		while (PeekMessage(&msg, hWnd, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, hWnd, 0, 0))
			{
				EndingWinloop("window destroyed");
				return (int)msg.wParam;
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		//If the window is active (ie: not minimised)
		if (mbActive)
		{
			ActiveTick();
		}
		else
		{
			WaitMessage();
		}
		//if (iCount == 100)
		//{
		//	mbLoop = false;
		//}
		iCount++;
	}

	EndingWinloop("shutdown requested");
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::EndingWinloop(char* szReason)
{
	CChars	szShutdown;

	mcLoopTimer.Update();
	szShutdown.Init("Shutdown winloop [");
	szShutdown.Append(szReason);
	szShutdown.Append("] - ");
	mcLoopTimer.HumanReadable(&szShutdown);
	mcLoopTimer.Kill();
	szShutdown.AppendNewLine();
	gcLogger.Add(szShutdown.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::ActiveTick(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::GetClientSize(int* px, int* py)
{
	*px = mcClientRect.GetWidth();
	*py = mcClientRect.GetHeight();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWinGDIWindow::GetClientWidth(void)
{
	return mcClientRect.GetWidth();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CWinGDIWindow::GetClientHeight(void)
{
	return mcClientRect.GetHeight();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::AddComponent(CComponent* pcComponent)
{
	mpcViewport->AddComponent(pcComponent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::RemoveComponent(CComponent* pcComponent, bool bKillComponent)
{
	mpcViewport->RemoveComponent(pcComponent, bKillComponent);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::RemoveAllComponents(bool bKillComponents)
{
	mpcViewport->RemoveAllComponents(bKillComponents);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponent* CWinGDIWindow::FindComponentAtRelative(int x, int y)
{
	return mpcViewport->FindComponentAt(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::IsPointInRelative(int x, int y)
{
	return mpcViewport->IsPointIn(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CComponent* CWinGDIWindow::FindComponentAtAbsolute(int x, int y)
{
	x -= mcClientRect.GetLeft();
	y -= mcClientRect.GetTop();
	return mpcViewport->FindComponentAt(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIWindow::IsPointInAbsolute(int x, int y)
{
	return mpcViewport->IsPointInAbsolute(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::UpdateRectangle(void)
{
	RECT	rect;
	bool	bResult;
	POINT	point;

	point.x = point.y = 0;
	bResult = ClientToScreen(hWnd, &point);
	if (bResult)
	{
		bResult = GetClientRect(hWnd, &rect);
		if (bResult)
		{
			mcClientRect.miLeft = rect.left + point.x;
			mcClientRect.miTop = rect.top + point.y;
			mcClientRect.miRight = rect.right + point.x;
			mcClientRect.miBottom = rect.bottom + point.y;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Activate(void)
{
	mbActive = true;
	if (mpcViewport)
	{
		mpcViewport->Activate();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinGDIWindow::Deactivate(void)
{
	mbActive = false;
	if (mpcViewport)
	{
		mpcViewport->Deactivate();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CViewport* CWinGDIWindow::GetViewport(void) { return mpcViewport; }
CInput* CWinGDIWindow::GetInput(void) { return mpcInput; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long FAR PASCAL CWindowCB(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWinGDIWindow* pcWindow;

	pcWindow = (CWinGDIWindow*)((size_t)GetWindowLongPtr(hWnd, GWLP_USERDATA));

	//Do nothing if the window is not yet created.
	if (pcWindow)
	{
		switch (message)
		{
		case WM_INPUT:
			// Need a WinRaw hook here somehow.

			//UINT 		dwSize;
			//BYTE 		lpb[40];
			//RAWINPUT* lpRawInput;

			//dwSize = 40;
			//GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

			//lpRawInput = (RAWINPUT*)lpb;
			//WinRawInputEvent(pcWindow->GetInput()->GetWinInput()->GetRaw(), lpRawInput);
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
		case WM_KILLFOCUS:
			break;
		case WM_CHAR:
			break;
		case WM_SYSKEYUP:
			break;
		case WM_SYSKEYDOWN:
			break;

		case WM_MOVING:
			pcWindow->UpdateRectangle();
			break;
		case WM_MOVE:
			pcWindow->UpdateRectangle();
			break;
		case WM_SIZE:
			pcWindow->UpdateRectangle();
			break;
		case WM_ACTIVATEAPP:
			if ((bool)wParam)
			{
				pcWindow->Activate();
			}
			else
			{
				pcWindow->Deactivate();
			}
			break;

		case WM_CLOSE:
			pcWindow->Destroy();
			break;

		case WM_DESTROY:
			PostQuitMessage(0);
			return 1L;

		case WM_WINDOWPOSCHANGED:
			break;
		case WM_SHOWWINDOW:
			pcWindow->UpdateRectangle();
			break;

		case WM_KEYDOWN:
			break;
		case WM_KEYUP:
			break;
		case WM_SETCURSOR:
			break;
		case WM_SETFOCUS:
			break;
		}
	}
	return (long)DefWindowProc(hWnd, message, wParam, lParam);;
}

