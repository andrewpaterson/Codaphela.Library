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
#include "SupportLib/Int32Rect.h"
#include "WindowLib/Window.h"
#include "WindowLib/ComponentInput.h"
#include "X11Input.h"
#include "X11WindowFactory.h"
#include "X11Window.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>


#define X11_WINDOW_WIDTH		800
#define X11_WINDOW_HEIGHT		600


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::Init(CWindow* pcWindow, CNativeWindowFactory* pcWindowFactory)
{
    CNativeWindow::Init(pcWindow, pcWindowFactory);

    mulWindow = 0;
    mpsGC = NULL;
    miWidth = 0;
    miHeight = 0;
    mbPresentPending = false;
    mbPointerKnown = false;
    miPointerX = 0;
    miPointerY = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::Kill(void)
{
    DestroyNativeWindow();
    CNativeWindow::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::DestroyNativeWindow(void)
{
    Display*            psDisplay;
    CX11WindowFactory*  pcFactory;

    if (mulWindow)
    {
        pcFactory = (CX11WindowFactory*)mpcWindowFactory;
        psDisplay = pcFactory->GetDisplay();

        XFreeGC(psDisplay, mpsGC);
        XDestroyWindow(psDisplay, mulWindow);
        XFlush(psDisplay);
        mpsGC = NULL;
        mulWindow = 0;
    }
}


//////////////////////////////////////////////////////////////////////////
//
// The canvas is laid out as B, G, R, X which is what a 24 bit TrueColor
// visual with red in 0xFF0000 holds in 32 bits on a little endian
// machine.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::Present(CNativeCanvas* pcNativeCanvas, int32 iWidth, int32 iHeight)
{
    CX11WindowFactory*  pcFactory;
    CX11Canvas*         pcX11Canvas;
    Display*            psDisplay;
    XImage*             psImage;
    SInt32Vec2          sSize;
    uint8*              puiPixelData;

    mbPresentPending = false;
    if (mulWindow == 0)
    {
        return;
    }

    pcX11Canvas = (CX11Canvas*)pcNativeCanvas;
    puiPixelData = pcX11Canvas->GetPixelData();
    if (puiPixelData == NULL)
    {
        return;
    }

    sSize = pcX11Canvas->GetSize();
    if (iWidth > sSize.x)
    {
        iWidth = sSize.x;
    }
    if (iHeight > sSize.y)
    {
        iHeight = sSize.y;
    }
    if ((iWidth <= 0) || (iHeight <= 0))
    {
        return;
    }

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();

    //The image only describes the canvas' pixels.  Its data is cleared before it is destroyed so that XDestroyImage doesn't free them.
    psImage = XCreateImage(psDisplay, DefaultVisual(psDisplay, pcFactory->GetScreen()), DefaultDepth(psDisplay, pcFactory->GetScreen()), ZPixmap, 0, (char*)puiPixelData, sSize.x, sSize.y, 32, sSize.x * 4);
    if (psImage)
    {
        XPutImage(psDisplay, mulWindow, mpsGC, psImage, 0, 0, 0, 0, iWidth, iHeight);
        psImage->data = NULL;
        XDestroyImage(psImage);

        //Waiting for the server stops a window that paints every tick queueing frames faster than they can be shown.
        XSync(psDisplay, False);
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CX11Window::CreateNativeWindow(void)
{
    CX11WindowFactory*      pcFactory;
    Display*                psDisplay;
    int                     iScreen;
    Visual*                 psVisual;
    int                     iDepth;
    XSetWindowAttributes    sAttributes;
    Atom                    ulDeleteWindow;
    CChars                  sz;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    if (psDisplay == NULL)
    {
        gcLogger.Error2(__METHOD__, " The factory has no X display.", NULL);
        return false;
    }

    iScreen = pcFactory->GetScreen();
    psVisual = DefaultVisual(psDisplay, iScreen);
    iDepth = DefaultDepth(psDisplay, iScreen);
    if ((psVisual->c_class != TrueColor) || ((iDepth != 24) && (iDepth != 32)) || (psVisual->red_mask != 0xFF0000) || (psVisual->green_mask != 0xFF00) || (psVisual->blue_mask != 0xFF))
    {
        sz.Init(" The default visual must be 24 bit TrueColor with red in 0xFF0000 but it has depth [");
        sz.Append(iDepth);
        sz.Append("].");
        gcLogger.Error2(__METHOD__, sz.Text(), NULL);
        sz.Kill();
        return false;
    }

    //There is no background so the window isn't cleared before each expose, as WM_ERASEBKGND isn't handled by the GDI window.
    memset(&sAttributes, 0, sizeof(XSetWindowAttributes));
    sAttributes.background_pixmap = None;
    sAttributes.bit_gravity = NorthWestGravity;
    sAttributes.event_mask = ExposureMask | StructureNotifyMask | FocusChangeMask |
                             KeyPressMask | KeyReleaseMask |
                             ButtonPressMask | ButtonReleaseMask | PointerMotionMask | EnterWindowMask | LeaveWindowMask;

    mulWindow = XCreateWindow(psDisplay,
                              RootWindow(psDisplay, iScreen),
                              0, 0,                                     // Position, the window manager places it
                              X11_WINDOW_WIDTH, X11_WINDOW_HEIGHT,      // Size
                              0,                                        // Border width
                              iDepth,
                              InputOutput,
                              psVisual,
                              CWBackPixmap | CWBitGravity | CWEventMask,
                              &sAttributes);
    if (mulWindow == 0)
    {
        gcLogger.Error2(__METHOD__, " XCreateWindow failed.", NULL);
        return false;
    }

    miWidth = X11_WINDOW_WIDTH;
    miHeight = X11_WINDOW_HEIGHT;

    XStoreName(psDisplay, mulWindow, mpcWindow->GetWindowTitle());

    //The window manager sends WM_DELETE_WINDOW instead of killing the connection when the window is closed.
    ulDeleteWindow = pcFactory->GetDeleteWindowAtom();
    XSetWMProtocols(psDisplay, mulWindow, &ulDeleteWindow, 1);

    mpsGC = XCreateGC(psDisplay, mulWindow, 0, NULL);

    XMapWindow(psDisplay, mulWindow);
    XFlush(psDisplay);

    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every waiting event is handled each tick.  A mouse sends motion far
// faster than the window ticks.
//
// There is only one window so every event on the display is for it.
// More windows will need the factory to route the events.
//
//////////////////////////////////////////////////////////////////////////
bool CX11Window::ExecuteNativeWindow(void)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;
    XEvent              uEvent;
    bool                bResult;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();

    while (XPending(psDisplay) > 0)
    {
        XNextEvent(psDisplay, &uEvent);
        bResult = HandleEvent(&uEvent);
        if (!bResult)
        {
            return false;
        }
    }

    if (mbPresentPending)
    {
        CNativeWindow::Present();
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns false when the window has been closed.
//
//////////////////////////////////////////////////////////////////////////
bool CX11Window::HandleEvent(XEvent* puEvent)
{
    CX11WindowFactory*  pcFactory;
    CX11Input*          pcInput;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    pcInput = pcFactory->GetInput();

    switch (puEvent->type)
    {
        case Expose:
        {
            //The last of a run of exposes presents the whole canvas.
            if (puEvent->xexpose.count == 0)
            {
                mbPresentPending = true;
            }
            break;
        }
        case ConfigureNotify:
        {
            miWidth = puEvent->xconfigure.width;
            miHeight = puEvent->xconfigure.height;
            break;
        }
        case ClientMessage:
        {
            if ((puEvent->xclient.message_type == pcFactory->GetProtocolsAtom()) && ((Atom)puEvent->xclient.data.l[0] == pcFactory->GetDeleteWindowAtom()))
            {
                Stop();
                DestroyNativeWindow();
                return false;
            }
            break;
        }
        case KeyPress:
        case KeyRelease:
        {
            if (pcInput)
            {
                pcInput->Key(puEvent->xkey.keycode, puEvent->type == KeyPress);
            }
            break;
        }
        case ButtonPress:
        case ButtonRelease:
        {
            MouseButton(puEvent->xbutton.button, puEvent->xbutton.state, puEvent->xbutton.x, puEvent->xbutton.y, puEvent->type == ButtonPress);
            break;
        }
        case MotionNotify:
        {
            MouseMoved(puEvent->xmotion.x, puEvent->xmotion.y);
            break;
        }
        case EnterNotify:
        {
            MouseMoved(puEvent->xcrossing.x, puEvent->xcrossing.y);
            break;
        }
        case LeaveNotify:
        {
            if (puEvent->xcrossing.mode == NotifyNormal)
            {
                MouseLeft(puEvent->xcrossing.state);
            }
            break;
        }
        case FocusIn:
        {
            //Focus moving to and from a keyboard grab (e.g. the window manager's Alt+Tab) is not the window being activated.
            if ((puEvent->xfocus.mode != NotifyGrab) && (puEvent->xfocus.mode != NotifyUngrab))
            {
                Activated();
            }
            break;
        }
        case FocusOut:
        {
            if ((puEvent->xfocus.mode != NotifyGrab) && (puEvent->xfocus.mode != NotifyUngrab))
            {
                Deactivated();
            }
            break;
        }
        case MappingNotify:
        {
            XRefreshKeyboardMapping(&puEvent->xmapping);
            break;
        }
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::SignalPresent(void)
{
    mbPresentPending = true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::GetRectangle(CInt32Rect* pcDest)
{
    pcDest->Init(0, 0, miWidth, miHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// x and y are in client space.  They can be outside the window while a
// button is held, X grabs the pointer for the window then.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::MouseMoved(int x, int y)
{
    CX11WindowFactory*  pcFactory;
    CX11Input*          pcInput;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    pcInput = pcFactory->GetInput();

    if (mbPointerKnown && pcInput)
    {
        if ((x != miPointerX) || (y != miPointerY))
        {
            pcInput->MouseMoved(x - miPointerX, y - miPointerY);
        }
    }
    miPointerX = x;
    miPointerY = y;
    mbPointerKnown = true;

    mpcWindow->PointerMoved(x, y);
}


//////////////////////////////////////////////////////////////////////////
//
// uiState holds the buttons that were down before this one changed.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::MouseButton(unsigned int uiButton, unsigned int uiState, int x, int y, bool bPressed)
{
    CX11WindowFactory*  pcFactory;
    CX11Input*          pcInput;
    unsigned int        uiHeld;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    pcInput = pcFactory->GetInput();

    if (!mbPointerKnown || (x != miPointerX) || (y != miPointerY))
    {
        MouseMoved(x, y);
    }

    if (pcInput)
    {
        pcInput->MouseButton(uiButton, bPressed);
    }

    if (!bPressed)
    {
        //The pointer was left outside the window by a drag.  It wasn't told when it left as the window had the pointer grabbed.
        uiHeld = uiState & (Button1Mask | Button2Mask | Button3Mask);
        if ((uiButton >= Button1) && (uiButton <= Button3))
        {
            uiHeld &= ~(Button1Mask << (uiButton - Button1));
        }
        if ((uiHeld == 0) && ((x < 0) || (y < 0) || (x >= miWidth) || (y >= miHeight)))
        {
            mbPointerKnown = false;
            mpcWindow->PointerLeft();
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
// A drag keeps the pointer even when it leaves the window.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::MouseLeft(unsigned int uiState)
{
    if ((uiState & (Button1Mask | Button2Mask | Button3Mask)) == 0)
    {
        mbPointerKnown = false;
        mpcWindow->PointerLeft();
    }
}


//////////////////////////////////////////////////////////////////////////
//
// The locks may have been pressed while another window had focus.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::Activated(void)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;
    CComponentInput*    pcComponentInput;
    XkbStateRec         sState;
    unsigned int        uiNumLockMask;
    bool                bCapsLock;
    bool                bNumLock;

    pcComponentInput = mpcWindow->GetComponentInput();
    if (pcComponentInput)
    {
        pcFactory = (CX11WindowFactory*)mpcWindowFactory;
        psDisplay = pcFactory->GetDisplay();

        memset(&sState, 0, sizeof(XkbStateRec));
        XkbGetState(psDisplay, XkbUseCoreKbd, &sState);
        uiNumLockMask = XkbKeysymToModifiers(psDisplay, XK_Num_Lock);

        bCapsLock = (sState.locked_mods & LockMask) != 0;
        bNumLock = (uiNumLockMask != 0) && ((sState.locked_mods & uiNumLockMask) != 0);
        pcComponentInput->SetLocks(bCapsLock, bNumLock);
    }
    mpcWindow->Activate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::Deactivated(void)
{
    mpcWindow->Deactivate();
}

