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
#include "Window.h"
#include "NativeWindowFactory.h"
#include "NativeCanvas.h"
#include "NativeWindow.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory)
{
	CNativeComponent::Init(pcWindowFactory);
	mpcWindowFactory = pcWindowFactory;
	mpcWindow = pcWindow;
	mpcOwner = NULL;
	mcLastRectangle.Init(-1, -1, -1, -1);
	msRequestedSize.Init(-1, -1);
	msMinimumSize.Init(-1, -1);
	mbPainting = false;
	mbEnabled = true;
	mbTool = false;
	mcScreenRectangle.Init(0, 0, 0, 0);
	mbScreenRectangle = false;
	meCursor = NC_Arrow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::Kill(void)
{
	CNativeComponent::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::BeginPresent(void)
{
	if (!mbPainting)
	{
		mbPainting = true;
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::EndPresent(void)
{
	mbPainting = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Shows the canvas in the native window.  Returns false if nothing was
// shown: the window hasn't been painted yet, so its canvas has no native
// canvas, or it is already presenting.
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::Present(void)
{
	bool            bPresented;
	CNativeCanvas*  pcNativeCanvas;
	Ptr<CCanvas>    pCanvas;
	SInt32Vec2      sSize;

	if (!BeginPresent())
	{
		return false;
	}

	bPresented = false;
	pCanvas = mpcWindow->GetCanvas();
	pcNativeCanvas = pCanvas->GetNativeCanvas();
	if (pcNativeCanvas)
	{
		sSize = pCanvas->GetActualSize();
		Present(pcNativeCanvas, sSize.x, sSize.y);
		bPresented = true;
	}
	EndPresent();
	return bPresented;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::Stop(void)
{
	mpcWindow->Stop();
}


//////////////////////////////////////////////////////////////////////////
//
// Set before the window is created.  A dialog is shown over its owner.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::SetOwner(CNativeWindow* pcOwner)
{
	mpcOwner = pcOwner;
}


//////////////////////////////////////////////////////////////////////////
//
// Set before the window is created.  The size of its client area.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::SetRequestedSize(int32 iWidth, int32 iHeight)
{
	msRequestedSize.Init(iWidth, iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// Set before the window is created.  A dialog is shown at its requested
// size and can't be resized, unless it has a minimum size: then it can be,
// no smaller than that.  -1, -1 for none.  A main window can always be.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::SetMinimumSize(int32 iWidth, int32 iHeight)
{
	msMinimumSize.Init(iWidth, iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// A main window, or a dialog with a minimum size.
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::IsResizable(void)
{
	return !mpcOwner || ((msMinimumSize.x >= 0) && (msMinimumSize.y >= 0));
}


//////////////////////////////////////////////////////////////////////////
//
// A native window that can stop its keyboard and pointer reaching the
// window overrides this too.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::SetEnabled(bool bEnabled)
{
	mbEnabled = bEnabled;
}


//////////////////////////////////////////////////////////////////////////
//
// A native window that can sleep until it has events overrides this and
// WaitForEvents.  One that can't, a reference window drawing off screen
// say, is ticked and painted continuously.
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::CanWaitForEvents(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns when the window has events to execute, or after iMilliseconds;
// -1 waits until there are events.  Anything presented but not yet shown
// is shown first.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::WaitForEvents(int64 iMilliseconds)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeWindow* CNativeWindow::GetOwner(void) { return mpcOwner; }
SInt32Vec2 CNativeWindow::GetRequestedSize(void) { return msRequestedSize; }
SInt32Vec2 CNativeWindow::GetMinimumSize(void) { return msMinimumSize; }
bool CNativeWindow::IsEnabled(void) { return mbEnabled; }


//////////////////////////////////////////////////////////////////////////
//
// Set before the window is created, with an owner.  A tool window is a
// floating window that stays over its owner without being modal: it has
// no frame of its own (WindowLib draws its title and edges), isn't on the
// task bar and doesn't take the keys when it appears.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::SetTool(bool bTool)
{
	mbTool = bTool;
}


//////////////////////////////////////////////////////////////////////////
//
// The client area's place and size on the screen.  Before the window is
// created it is where it will be made; after, the window is moved and
// resized.  A native window that can move itself overrides this and calls
// it.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::SetScreenRectangle(CInt32Rect* pcRectangle)
{
	mcScreenRectangle = *pcRectangle;
	mbScreenRectangle = true;
	msRequestedSize.Init(pcRectangle->GetWidth(), pcRectangle->GetHeight());
}


//////////////////////////////////////////////////////////////////////////
//
// The client area on the screen.  Returns false if it isn't known, then
// pcDest is the client area at 0, 0.
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::GetScreenRectangle(CInt32Rect* pcDest)
{
	CInt32Rect	cClient;

	GetRectangle(&cClient);
	if (mbScreenRectangle)
	{
		pcDest->Init(mcScreenRectangle.GetLeft(), mcScreenRectangle.GetTop(), mcScreenRectangle.GetLeft() + cClient.GetWidth(), mcScreenRectangle.GetTop() + cClient.GetHeight());
		return true;
	}
	pcDest->Init(0, 0, cClient.GetWidth(), cClient.GetHeight());
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Where the pointer is on the screen.  Returns false if the platform
// can't say; the window works it out from the client area then.
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::GetPointerScreenPosition(SInt32Vec2* psDest)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The screen's area, for keeping floating windows on it.  Returns false
// if the platform can't say; nothing is kept on the screen then.
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::GetScreenArea(CInt32Rect* pcDest)
{
	pcDest->Init(0, 0, 0, 0);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A native window that can change the pointer's shape overrides this and
// calls it.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::SetCursor(ENativeCursor eCursor)
{
	meCursor = eCursor;
}


//////////////////////////////////////////////////////////////////////////
//
// The window is on the screen.  It isn't once it has been closed.
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::IsCreated(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the window over its owner's other windows.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::Raise(void)
{
}


bool CNativeWindow::IsTool(void) { return mbTool; }
ENativeCursor CNativeWindow::GetCursor(void) { return meCursor; }


//////////////////////////////////////////////////////////////////////////
//
// The window's title changed (CWindow::SetWindowTitle).  A native window
// with a title of its own on the screen overrides this.
//
//////////////////////////////////////////////////////////////////////////
void CNativeWindow::TitleChanged(void)
{
}

