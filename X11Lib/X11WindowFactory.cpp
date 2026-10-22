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
#include "BaseLib/Logger.h"
#include "X11Window.h"
#include "X11Canvas.h"
#include "X11WindowFactory.h"
#include <X11/Xlib.h>
#include <X11/XKBlib.h>


//////////////////////////////////////////////////////////////////////////
//
// szDisplayName may be NULL, the DISPLAY environment variable is used
// then.
//
//////////////////////////////////////////////////////////////////////////
bool CX11WindowFactory::Init(CMallocator* pcMallocator, const char* szDisplayName)
{
    Bool    bDetectable;

    CNativeWindowFactory::Init(pcMallocator);

    mpcInput = NULL;
    mulDeleteWindowAtom = 0;
    mulProtocolsAtom = 0;
    miScreen = 0;

    mpsDisplay = XOpenDisplay(szDisplayName);
    if (mpsDisplay == NULL)
    {
        gcLogger.Error2(__METHOD__, " Could not open X display [", XDisplayName(szDisplayName), "].", NULL);
        return false;
    }

    miScreen = DefaultScreen(mpsDisplay);
    mulDeleteWindowAtom = XInternAtom(mpsDisplay, "WM_DELETE_WINDOW", False);
    mulProtocolsAtom = XInternAtom(mpsDisplay, "WM_PROTOCOLS", False);

    //A held key then repeats as presses only, as Windows raw input does, rather than as release and press pairs.
    bDetectable = False;
    XkbSetDetectableAutoRepeat(mpsDisplay, True, &bDetectable);
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11WindowFactory::Kill(void)
{
    if (mpsDisplay)
    {
        XCloseDisplay(mpsDisplay);
        mpsDisplay = NULL;
    }
    CNativeWindowFactory::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeWindow* CX11WindowFactory::CreateNativeWindow(CWindow* pcWindow)
{
    CX11Window*     pcNativeWindow;

    pcNativeWindow = NativeMalloc<CX11Window>();
    pcNativeWindow->Init(pcWindow, this);

    return pcNativeWindow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CNativeCanvas* CX11WindowFactory::CreateNativeCanvas(CCanvas* pcCanvas)
{
    CX11Canvas*     pcNativeCanvas;
    bool            bResult;

    pcNativeCanvas = NativeMalloc<CX11Canvas>();
    pcNativeCanvas->Init(pcCanvas, this);
    bResult = pcNativeCanvas->CreateNativeCanvas();
    if (!bResult)
    {
        DestroyNativeCanvas(pcNativeCanvas);
        return NULL;
    }
    else
    {
        return pcNativeCanvas;
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
_XDisplay* CX11WindowFactory::GetDisplay(void) { return mpsDisplay; }
int CX11WindowFactory::GetScreen(void) { return miScreen; }
X11ID CX11WindowFactory::GetDeleteWindowAtom(void) { return mulDeleteWindowAtom; }
X11ID CX11WindowFactory::GetProtocolsAtom(void) { return mulProtocolsAtom; }


//////////////////////////////////////////////////////////////////////////
//
// Without it key and mouse button events are ignored.
//
//////////////////////////////////////////////////////////////////////////
void CX11WindowFactory::SetInput(CX11Input* pcInput) { mpcInput = pcInput; }
CX11Input* CX11WindowFactory::GetInput(void) { return mpcInput; }

