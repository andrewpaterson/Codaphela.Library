/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseLib/NativeThread.h"
#include "BaseLib/FileUtil.h"
#include "SupportLib/PNGWriter.h"
#include "WindowLib/Window.h"
#include "WinRefWindowFactory.h"
#include "WinRefWindow.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory, CInt32Rect* pcBounds, char* szImagePath)
{
	CNativeWindow::Init(pcWindow, pcWindowFactory);
	mbCreated = false;
	mcWindowBounds.Init(pcBounds);
	mszImagePath = szImagePath;
	muiFrame = 0;
	mcPresented.Init();
	mcPresenting.Init();
	mcShown.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::Kill(void)
{
	GetFactory()->GetScreen()->Remove(this);
	mcShown.Kill();
	mszImagePath = NULL;
	mbCreated = false;
	CNativeWindow::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The window goes on top of the screen's.
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefWindow::CreateNativeWindow(void)
{
	if ((msRequestedSize.x > 0) && (msRequestedSize.y > 0))
	{
		mcWindowBounds.Init(mcWindowBounds.GetLeft(), mcWindowBounds.GetTop(), mcWindowBounds.GetLeft() + msRequestedSize.x, mcWindowBounds.GetTop() + msRequestedSize.y);
	}
	GetFactory()->GetScreen()->Add(this);
	mbCreated = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::DestroyNativeWindow(void)
{
	GetFactory()->GetScreen()->Remove(this);
	mbCreated = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing outside the program sends a reference window events.
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefWindow::ExecuteNativeWindow(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Only if the factory says (CWinRefWindowFactory::SetWaitForEvents):
// otherwise the window is ticked and painted continuously.
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefWindow::CanWaitForEvents(void)
{
	return GetFactory()->GetMaxWait() >= 0;
}


//////////////////////////////////////////////////////////////////////////
//
// No event will come, so the window sleeps until something is due, but
// no longer than the factory's longest wait: nothing in the program can
// wake it, and a wait for ever (-1) would never end.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::WaitForEvents(int64 iMilliseconds)
{
	int64	iMaxWait;

	iMaxWait = GetFactory()->GetMaxWait();
	if ((iMilliseconds < 0) || (iMilliseconds > iMaxWait))
	{
		iMilliseconds = iMaxWait;
	}
	if (iMilliseconds > 0)
	{
		ThreadSleep((int)iMilliseconds);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::Raise(void)
{
	GetFactory()->GetScreen()->Raise(this);
}


//////////////////////////////////////////////////////////////////////////
//
// What was asked to be shown since the last present goes to the screen,
// and the frame is saved if the factory saves it.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight)
{
	CWinRefCanvas*			pcRefCanvas;
	CWinRefWindowFactory*	pcFactory;
	Ptr<CImage>				pImage;
	CWinRefPixels			cCanvas;
	size					uiPixelBytes;
	CFileUtil				cFileUtil;
	CChars					szFileName;

	pcRefCanvas = (CWinRefCanvas*)pcNativeCanvas;
	pcFactory = GetFactory();
	pImage = pcRefCanvas->GetNativeImage();
	if (pcFactory->GetScreen()->IsWatching() && pImage.IsNotNull())
	{
		//The canvas's image can be bigger than the canvas; what is past it isn't shown.
		uiPixelBytes = pImage->GetPixelByteStride();
		iWidth = (iWidth < (int32)pImage->GetWidth()) ? iWidth : (int32)pImage->GetWidth();
		iHeight = (iHeight < (int32)pImage->GetHeight()) ? iHeight : (int32)pImage->GetHeight();
		cCanvas.Init(pImage->GetData(), iWidth, iHeight, uiPixelBytes, (lsize)pImage->GetWidth() * uiPixelBytes);
		pcFactory->GetScreen()->Present(this, &cCanvas, &mcPresenting);
		cCanvas.Kill();
	}
	mcPresenting.Clear();

	//No image path means the frames are not wanted; don't write them into the working directory.  A dialog's frames are
	//kept apart from its owner's.
	if ((mszImagePath != NULL) && (mszImagePath[0] != '\0') && pcFactory->IsFrameSaved(muiFrame))
	{
		szFileName.Init(mszImagePath);
		cFileUtil.AppendToPath(&szFileName, mpcOwner ? (mbTool ? "ToolFrame" : "DialogFrame") : "Frame");
		szFileName.Append(muiFrame);
		szFileName.Append(".png");
		SavePNG(pImage, szFileName.Text());
		szFileName.Kill();
	}

	muiFrame++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::SignalPresent(void)
{
	mcPresented.AddAll();
	mcPresenting.AddAll();
	CNativeWindow::Present();
}


//////////////////////////////////////////////////////////////////////////
//
// Only pcArea changed.  A saved frame is still all of the window.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::SignalPresentArea(CInt32Rect* pcArea)
{
	mcPresented.Add(pcArea);
	mcPresenting.Add(pcArea);
	CNativeWindow::Present();
}


//////////////////////////////////////////////////////////////////////////
//
// A paint's damage, however many areas or none, is one present, as when
// all of the window is shown, so tests see a frame per paint.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::SignalPresentDamage(CDamageArea* pcDamage)
{
	size	i;

	if (pcDamage->IsAll())
	{
		mcPresented.AddAll();
		mcPresenting.AddAll();
	}
	for (i = 0; i < pcDamage->NumRectangles(); i++)
	{
		mcPresented.Add(pcDamage->GetRectangle(i));
		mcPresenting.Add(pcDamage->GetRectangle(i));
	}
	CNativeWindow::Present();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDamageArea* CWinRefWindow::GetPresented(void)
{
	return &mcPresented;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::ClearPresented(void)
{
	mcPresented.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::GetRectangle(CInt32Rect* pcDest)
{
	pcDest->Init(&mcWindowBounds);
}


//////////////////////////////////////////////////////////////////////////
//
// The client area is the rectangle's size, at 0, 0 in the window.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::SetScreenRectangle(CInt32Rect* pcRectangle)
{
	CNativeWindow::SetScreenRectangle(pcRectangle);
	mcWindowBounds.Init(0, 0, pcRectangle->GetWidth(), pcRectangle->GetHeight());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefWindow::IsCreated(void)
{
	return mbCreated;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefWindow::GetScreenArea(CInt32Rect* pcDest)
{
	return GetFactory()->GetScreenArea(pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
const char* CWinRefWindow::GetTitle(void)
{
	return mpcWindow->GetWindowTitle();
}


CWinRefPixels* CWinRefWindow::GetShown(void) { return &mcShown; }
CWinRefWindowFactory* CWinRefWindow::GetFactory(void) { return (CWinRefWindowFactory*)mpcWindowFactory; }

