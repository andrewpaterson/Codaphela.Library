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
#include "BaseLib/Define.h"
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
#include "BaseLib/Chars.h"
#include "BaseLib/DebugOutput.h"
#include "SupportLib/PNGWriter.h"
#include "SupportLib/Int32Rect.h"
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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::Kill(void)
{
    muiFrame = -1;
    mszImagePath = NULL;
    mbCreated = false;
    CNativeWindow::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefWindow::CreateNativeWindow(void)
{
    if ((msRequestedSize.x > 0) && (msRequestedSize.y > 0))
    {
        mcWindowBounds.Init(mcWindowBounds.GetLeft(), mcWindowBounds.GetTop(), mcWindowBounds.GetLeft() + msRequestedSize.x, mcWindowBounds.GetTop() + msRequestedSize.y);
    }
    mbCreated = true;
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::DestroyNativeWindow(void)
{
    mbCreated = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefWindow::ExecuteNativeWindow(void)
{
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight)
{
    CWinRefCanvas*  pcRefCanvas;
    CFileUtil       cFileUtil;
    CChars          szFileName;

    pcRefCanvas = (CWinRefCanvas*)pcNativeCanvas;

    //No image path means the frames are not wanted; don't write them into the working directory.
    if ((mszImagePath != NULL) && (mszImagePath[0] != '\0'))
    {
        //A dialog's frames are kept apart from its owner's.
        szFileName.Init(mszImagePath);
        cFileUtil.AppendToPath(&szFileName, mpcOwner ? (mbTool ? "ToolFrame" : "DialogFrame") : "Frame");
        szFileName.Append(muiFrame);
        szFileName.Append(".png");

        //You should create a new image here and size to iWidth and iHeight before calling Save.
        SavePNG(pcRefCanvas->GetNativeImage(), szFileName.Text());
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
    CNativeWindow::Present();
}


//////////////////////////////////////////////////////////////////////////
//
// Only pcArea changed.  The reference window still saves all of it, but
// notes what it was asked to show.
//
//////////////////////////////////////////////////////////////////////////
void CWinRefWindow::SignalPresentArea(CInt32Rect* pcArea)
{
    mcPresented.Add(pcArea);
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
    return ((CWinRefWindowFactory*)mpcWindowFactory)->GetScreenArea(pcDest);
}

