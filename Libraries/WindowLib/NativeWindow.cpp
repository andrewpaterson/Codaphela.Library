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
	mbPainting = false;
	mbEnabled = true;
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
//
//////////////////////////////////////////////////////////////////////////
bool CNativeWindow::Present(void)
{
	bool            bResult;
	CNativeCanvas*  pcNativeCanvas;
	Ptr<CCanvas>    pCanvas;
	SInt32Vec2           sSize;

	bResult = BeginPresent();
	if (bResult)
	{
		pCanvas = mpcWindow->GetCanvas();

		pcNativeCanvas = pCanvas->GetNativeCanvas();
		if (pcNativeCanvas)
		{
			sSize = pCanvas->GetActualSize();
			Present(pcNativeCanvas, sSize.x, sSize.y);
		}

		EndPresent();
		return true;
	}
	return false;
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
//
//////////////////////////////////////////////////////////////////////////
CNativeWindow* CNativeWindow::GetOwner(void) { return mpcOwner; }
SInt32Vec2 CNativeWindow::GetRequestedSize(void) { return msRequestedSize; }
bool CNativeWindow::IsEnabled(void) { return mbEnabled; }
