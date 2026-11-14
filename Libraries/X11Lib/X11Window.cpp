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
#include "BaseLib/Timer.h"
#include "SupportLib/Int32Rect.h"
#include "WindowLib/Window.h"
#include "WindowLib/ComponentInput.h"
#include "X11Input.h"
#include "X11WindowFactory.h"
#include "X11Window.h"
#include <X11/Xlib.h>
#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <poll.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/keysym.h>
#include <X11/Xatom.h>
#include <X11/cursorfont.h>


#define X11_SHARED_PRESENT_TIMEOUT	250  //Milliseconds the X server has to read a window's shared pixels.


#define X11_WINDOW_WIDTH        800
#define X11_WINDOW_HEIGHT       600


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
    mbPresentArea = false;
    mbPointerKnown = false;
    miPointerX = 0;
    miPointerY = 0;
    memset(maulCursors, 0, sizeof(maulCursors));
    mpsSharedImage = NULL;
    mpsSharedSegment = NULL;
    mbSharedFailed = false;
    mbSharedPresenting = false;
    mbSharedTimedOut = false;
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
    XWindowAttributes   sOwnerAttributes;

    if (mulWindow)
    {
        pcFactory = (CX11WindowFactory*)mpcWindowFactory;
        psDisplay = pcFactory->GetDisplay();

        //A main window remembers where it was, so it can be asked where it was after it is closed (its layout is
        //saved then), and is made there again.
        if (!mpcOwner)
        {
            GetScreenRectangle(&mcScreenRectangle);
            mbScreenRectangle = true;
        }

        //The keys go back to the owner, rather than to whatever the window manager chooses.  It may not be on the screen
        //yet, if it's a dialog that opened this one straight away, and a window that isn't can't have the keys.
        if (mpcOwner && ((CX11Window*)mpcOwner)->mulWindow)
        {
            if (XGetWindowAttributes(psDisplay, ((CX11Window*)mpcOwner)->mulWindow, &sOwnerAttributes) && (sOwnerAttributes.map_state == IsViewable))
            {
                XSetInputFocus(psDisplay, ((CX11Window*)mpcOwner)->mulWindow, RevertToParent, CurrentTime);
            }
        }

        pcFactory->RemoveWindow(this);
        FreeSharedImage();
        FreeCursors();
        XFreeGC(psDisplay, mpsGC);
        XDestroyWindow(psDisplay, mulWindow);
        XFlush(psDisplay);
        mpsGC = NULL;
        mulWindow = 0;
        mbPointerKnown = false;
        mbPresentPending = false;
        mbPresentArea = false;
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
    PresentPixels(pcNativeCanvas, 0, 0, iWidth, iHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// The canvas' pixels from iLeft, iTop to iRight, iBottom (exclusive) put
// in the same place in the window.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::PresentPixels(CNativeCanvas* pcNativeCanvas, int32 iLeft, int32 iTop, int32 iRight, int32 iBottom)
{
    CX11WindowFactory*  pcFactory;
    CX11Canvas*         pcX11Canvas;
    Display*            psDisplay;
    XImage*             psImage;
    SInt32Vec2          sSize;
    uint8*              puiPixelData;
    int32               y;

    mbPresentPending = false;
    mbPresentArea = false;
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
    if (iLeft < 0)
    {
        iLeft = 0;
    }
    if (iTop < 0)
    {
        iTop = 0;
    }
    if (iRight > sSize.x)
    {
        iRight = sSize.x;
    }
    if (iBottom > sSize.y)
    {
        iBottom = sSize.y;
    }
    if ((iRight <= iLeft) || (iBottom <= iTop))
    {
        return;
    }

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();

    //The pixels are copied into memory the X server shares and read from there, not sent through the connection.  The
    //server says when it has read them; until then they mustn't be copied over, so the window waits for the last present
    //here rather than straight after it, and draws its next frame meanwhile.  Only one present at a time also stops a
    //window that paints every tick queueing frames faster than they can be shown.
    if (pcFactory->UsesSharedMemory() && !mbSharedFailed && PrepareSharedImage(sSize.x, sSize.y))
    {
        WaitForSharedPresent();
        for (y = iTop; y < iBottom; y++)
        {
            memcpy(&mpsSharedImage->data[y * mpsSharedImage->bytes_per_line + iLeft * 4], &puiPixelData[(y * sSize.x + iLeft) * 4], (iRight - iLeft) * 4);
        }
        XShmPutImage(psDisplay, mulWindow, mpsGC, mpsSharedImage, iLeft, iTop, iLeft, iTop, iRight - iLeft, iBottom - iTop, True);
        XFlush(psDisplay);
        mbSharedPresenting = true;
        return;
    }

    //The image only describes the canvas' pixels.  Its data is cleared before it is destroyed so that XDestroyImage doesn't free them.
    psImage = XCreateImage(psDisplay, DefaultVisual(psDisplay, pcFactory->GetScreen()), DefaultDepth(psDisplay, pcFactory->GetScreen()), ZPixmap, 0, (char*)puiPixelData, sSize.x, sSize.y, 32, sSize.x * 4);
    if (psImage)
    {
        XPutImage(psDisplay, mulWindow, mpsGC, psImage, iLeft, iTop, iLeft, iTop, iRight - iLeft, iBottom - iTop);
        psImage->data = NULL;
        XDestroyImage(psImage);

        //Waiting for the server stops a window that paints every tick queueing frames faster than they can be shown.
        XSync(psDisplay, False);
    }
}


//////////////////////////////////////////////////////////////////////////
//
// A window's shared memory.  XShmCreateImage keeps a pointer to it and
// XShmPutImage reads the segment from it, so it must outlive the image.
//
//////////////////////////////////////////////////////////////////////////
struct SX11SharedSegment
{
    XShmSegmentInfo     sInfo;
};


//////////////////////////////////////////////////////////////////////////
//
// Set while the X server is asked to attach shared memory, see
// PrepareSharedImage.
//
//////////////////////////////////////////////////////////////////////////
static bool gbSharedAttachFailed = false;

static int SharedAttachErrorHandler(Display* psDisplay, XErrorEvent* psError)
{
    gbSharedAttachFailed = true;
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// An image of iWidth by iHeight 32 bit pixels in memory the X server
// shares.  False if it couldn't be made, as on a remote display, then
// the window sends its pixels through the connection from then on.
//
//////////////////////////////////////////////////////////////////////////
bool CX11Window::PrepareSharedImage(int32 iWidth, int32 iHeight)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;
    XImage*             psImage;
    XShmSegmentInfo*    psInfo;
    int                 (*fPreviousHandler)(Display*, XErrorEvent*);
    bool                bAttached;

    if (mpsSharedImage && (mpsSharedImage->width == iWidth) && (mpsSharedImage->height == iHeight))
    {
        return true;
    }
    FreeSharedImage();

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    mpsSharedSegment = NewMalloc<SX11SharedSegment>();
    psInfo = &mpsSharedSegment->sInfo;
    memset(psInfo, 0, sizeof(XShmSegmentInfo));
    psImage = XShmCreateImage(psDisplay, DefaultVisual(psDisplay, pcFactory->GetScreen()), DefaultDepth(psDisplay, pcFactory->GetScreen()), ZPixmap, NULL, psInfo, iWidth, iHeight);
    if (psImage == NULL)
    {
        SafeFree(mpsSharedSegment);
        mbSharedFailed = true;
        return false;
    }
    if (psImage->bits_per_pixel != 32)
    {
        XDestroyImage(psImage);
        SafeFree(mpsSharedSegment);
        mbSharedFailed = true;
        return false;
    }

    psInfo->shmid = shmget(IPC_PRIVATE, psImage->bytes_per_line * psImage->height, IPC_CREAT | 0600);
    if (psInfo->shmid < 0)
    {
        XDestroyImage(psImage);
        SafeFree(mpsSharedSegment);
        mbSharedFailed = true;
        return false;
    }
    psInfo->shmaddr = (char*)shmat(psInfo->shmid, NULL, 0);
    if (psInfo->shmaddr == (char*)-1)
    {
        shmctl(psInfo->shmid, IPC_RMID, NULL);
        XDestroyImage(psImage);
        SafeFree(mpsSharedSegment);
        mbSharedFailed = true;
        return false;
    }
    psImage->data = psInfo->shmaddr;
    psInfo->readOnly = False;

    //Attaching fails asynchronously, as an X error, when the server can't reach the memory.
    gbSharedAttachFailed = false;
    fPreviousHandler = XSetErrorHandler(SharedAttachErrorHandler);
    bAttached = XShmAttach(psDisplay, psInfo);
    XSync(psDisplay, False);
    XSetErrorHandler(fPreviousHandler);

    //Marked for removal now, it goes when both sides have let go of it, even if the process dies.
    shmctl(psInfo->shmid, IPC_RMID, NULL);
    if (!bAttached || gbSharedAttachFailed)
    {
        shmdt(psInfo->shmaddr);
        psImage->data = NULL;
        XDestroyImage(psImage);
        SafeFree(mpsSharedSegment);
        mbSharedFailed = true;
        return false;
    }

    mpsSharedImage = psImage;
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Picks the X server's 'read the shared pixels' event for one window out
// of the queue, leaving every other event where it is.
//
//////////////////////////////////////////////////////////////////////////
struct SSharedCompletion
{
    int     iEventType;
    Window  ulWindow;
};

static Bool IsSharedCompletion(Display* psDisplay, XEvent* puEvent, XPointer pvArgument)
{
    SSharedCompletion*  psCompletion;

    psCompletion = (SSharedCompletion*)pvArgument;
    return (puEvent->type == psCompletion->iEventType) && (puEvent->xany.window == psCompletion->ulWindow);
}


//////////////////////////////////////////////////////////////////////////
//
// Waits until the X server has read the window's last shared present, so
// its pixels can be copied over or let go of.  If the server doesn't say
// so in X11_SHARED_PRESENT_TIMEOUT it is waited for with XSync instead.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::WaitForSharedPresent(void)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;
    SSharedCompletion   sCompletion;
    XEvent              uEvent;
    struct pollfd       sPoll;
    int64               iStart;
    int64               iLeft;

    if (!mbSharedPresenting)
    {
        return;
    }

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    sCompletion.iEventType = pcFactory->GetSharedCompletionEvent();
    sCompletion.ulWindow = mulWindow;
    iStart = GetDebugTickCount();
    while (!XCheckIfEvent(psDisplay, &uEvent, IsSharedCompletion, (XPointer)&sCompletion))
    {
        iLeft = X11_SHARED_PRESENT_TIMEOUT - (GetDebugTickCount() - iStart);
        if (iLeft <= 0)
        {
            if (!mbSharedTimedOut)
            {
                gcLogger.Warning2(__METHOD__, " The X server didn't say it had read a present; waiting for it instead.", NULL);
                mbSharedTimedOut = true;
            }
            XSync(psDisplay, False);
            break;
        }
        sPoll.fd = ConnectionNumber(psDisplay);
        sPoll.events = POLLIN;
        sPoll.revents = 0;
        poll(&sPoll, 1, (int)iLeft);
    }
    mbSharedPresenting = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::FreeSharedImage(void)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;

    if (mpsSharedImage == NULL)
    {
        return;
    }

    WaitForSharedPresent();
    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    XShmDetach(psDisplay, &mpsSharedSegment->sInfo);
    XSync(psDisplay, False);
    shmdt(mpsSharedImage->data);
    mpsSharedImage->data = NULL;
    XDestroyImage(mpsSharedImage);
    mpsSharedImage = NULL;
    SafeFree(mpsSharedSegment);
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
    int                     iWidth;
    int                     iHeight;

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

    GetCreatedSize(&iWidth, &iHeight);
    mulWindow = XCreateWindow(psDisplay,
                              RootWindow(psDisplay, iScreen),
                              mbScreenRectangle ? mcScreenRectangle.GetLeft() : 0,  // Position, the window manager places it
                              mbScreenRectangle ? mcScreenRectangle.GetTop() : 0,   // unless one was set.
                              iWidth, iHeight,                          // Size
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

    miWidth = iWidth;
    miHeight = iHeight;

    XStoreName(psDisplay, mulWindow, mpcWindow->GetWindowTitle());

    //The window manager sends WM_DELETE_WINDOW instead of killing the connection when the window is closed.
    ulDeleteWindow = pcFactory->GetDeleteWindowAtom();
    XSetWMProtocols(psDisplay, mulWindow, &ulDeleteWindow, 1);

    mpsGC = XCreateGC(psDisplay, mulWindow, 0, NULL);

    if (mpcOwner && mbTool)
    {
        SetToolHints();
    }
    else if (mpcOwner)
    {
        SetDialogHints(iWidth, iHeight);
    }
    else
    {
        SetMainHints();
    }

    pcFactory->AddWindow(this);
    XMapWindow(psDisplay, mulWindow);
    XFlush(psDisplay);

    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A dialog stays over its owner, centred on it, can't be resized (unless
// it has a minimum size) and is modal; the window manager is asked for all
// of this.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::SetDialogHints(int iWidth, int iHeight)
{
    CX11WindowFactory*      pcFactory;
    Display*                psDisplay;
    CX11Window*             pcOwner;
    XSizeHints*             psHints;
    XWindowAttributes       sOwnerAttributes;
    Window                  ulChild;
    Atom                    ulType;
    Atom                    ulModal;
    int                     iOwnerX;
    int                     iOwnerY;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    pcOwner = (CX11Window*)mpcOwner;
    if (pcOwner->mulWindow == 0)
    {
        //An owner that isn't on the screen has nothing for the dialog to stay over or be centred on.
        return;
    }

    XSetTransientForHint(psDisplay, mulWindow, pcOwner->mulWindow);

    ulType = pcFactory->InternAtom("_NET_WM_WINDOW_TYPE_DIALOG");
    XChangeProperty(psDisplay, mulWindow, pcFactory->InternAtom("_NET_WM_WINDOW_TYPE"), XA_ATOM, 32, PropModeReplace, (unsigned char*)&ulType, 1);
    ulModal = pcFactory->InternAtom("_NET_WM_STATE_MODAL");
    XChangeProperty(psDisplay, mulWindow, pcFactory->InternAtom("_NET_WM_STATE"), XA_ATOM, 32, PropModeReplace, (unsigned char*)&ulModal, 1);

    //Centred on the owner's client area, where it is on the screen.
    iOwnerX = 0;
    iOwnerY = 0;
    memset(&sOwnerAttributes, 0, sizeof(XWindowAttributes));
    XGetWindowAttributes(psDisplay, pcOwner->mulWindow, &sOwnerAttributes);
    XTranslateCoordinates(psDisplay, pcOwner->mulWindow, RootWindow(psDisplay, pcFactory->GetScreen()), 0, 0, &iOwnerX, &iOwnerY, &ulChild);
    XMoveWindow(psDisplay, mulWindow, iOwnerX + (sOwnerAttributes.width - iWidth) / 2, iOwnerY + (sOwnerAttributes.height - iHeight) / 2);

    psHints = XAllocSizeHints();
    if (psHints)
    {
        psHints->flags = PPosition | PMinSize | PMaxSize;
        psHints->x = iOwnerX + (sOwnerAttributes.width - iWidth) / 2;
        psHints->y = iOwnerY + (sOwnerAttributes.height - iHeight) / 2;
        psHints->min_width = iWidth;
        psHints->max_width = iWidth;
        psHints->min_height = iHeight;
        psHints->max_height = iHeight;
        if (IsResizable())
        {
            psHints->flags = PPosition | PMinSize;
            psHints->min_width = msMinimumSize.x;
            psHints->min_height = msMinimumSize.y;
        }
        XSetWMNormalHints(psDisplay, mulWindow, psHints);
        XFree(psHints);
    }
}


//////////////////////////////////////////////////////////////////////////
//
// A main window's position is its client area's, not its frame's (static
// gravity), so a position read from it (GetScreenRectangle) puts it back
// in the same place.  Placed where it was asked if a position was set,
// else where the window manager chooses.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::SetMainHints(void)
{
    CX11WindowFactory*      pcFactory;
    XSizeHints*             psHints;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psHints = XAllocSizeHints();
    if (psHints)
    {
        psHints->flags = PWinGravity;
        psHints->win_gravity = StaticGravity;
        if (mbScreenRectangle)
        {
            psHints->flags |= USPosition | PPosition;
            psHints->x = mcScreenRectangle.GetLeft();
            psHints->y = mcScreenRectangle.GetTop();
        }
        XSetWMNormalHints(pcFactory->GetDisplay(), mulWindow, psHints);
        XFree(psHints);
    }
}


//////////////////////////////////////////////////////////////////////////
//
// A tool window floats over its owner with no frame: WindowLib draws its
// title and edges and moves it itself.  It isn't on the task bar.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::SetToolHints(void)
{
    CX11WindowFactory*      pcFactory;
    Display*                psDisplay;
    CX11Window*             pcOwner;
    XSizeHints*             psHints;
    Atom                    ulType;
    Atom                    aulStates[1];
    Atom                    ulMotif;
    unsigned long           aulMotifHints[5];
    XWMHints*               psWMHints;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    pcOwner = (CX11Window*)mpcOwner;
    if (pcOwner->mulWindow)
    {
        XSetTransientForHint(psDisplay, mulWindow, pcOwner->mulWindow);
    }
    ulType = pcFactory->InternAtom("_NET_WM_WINDOW_TYPE_UTILITY");
    XChangeProperty(psDisplay, mulWindow, pcFactory->InternAtom("_NET_WM_WINDOW_TYPE"), XA_ATOM, 32, PropModeReplace, (unsigned char*)&ulType, 1);
    aulStates[0] = pcFactory->InternAtom("_NET_WM_STATE_SKIP_TASKBAR");
    XChangeProperty(psDisplay, mulWindow, pcFactory->InternAtom("_NET_WM_STATE"), XA_ATOM, 32, PropModeReplace, (unsigned char*)aulStates, 1);

    //_MOTIF_WM_HINTS: flags (2, decorations), functions, decorations (none), input mode, status.
    ulMotif = pcFactory->InternAtom("_MOTIF_WM_HINTS");
    memset(aulMotifHints, 0, sizeof(aulMotifHints));
    aulMotifHints[0] = 2;
    XChangeProperty(psDisplay, mulWindow, ulMotif, ulMotif, 32, PropModeReplace, (unsigned char*)aulMotifHints, 5);

    //Placed where it was asked, not where the window manager would put it.
    psHints = XAllocSizeHints();
    if (psHints)
    {
        psHints->flags = USPosition | PPosition;
        psHints->x = mcScreenRectangle.GetLeft();
        psHints->y = mcScreenRectangle.GetTop();
        XSetWMNormalHints(psDisplay, mulWindow, psHints);
        XFree(psHints);
    }

    //Shown without the keys, as dragging a pane out of its owner shouldn't take them.
    psWMHints = XAllocWMHints();
    if (psWMHints)
    {
        psWMHints->flags = InputHint;
        psWMHints->input = True;
        XSetWMHints(psDisplay, mulWindow, psWMHints);
        XFree(psWMHints);
    }
}


//////////////////////////////////////////////////////////////////////////
//
// Every waiting event, for every window, is handled each tick.  A mouse
// sends motion far faster than the window ticks.  Returns false once the
// window has been closed.
//
//////////////////////////////////////////////////////////////////////////
bool CX11Window::ExecuteNativeWindow(void)
{
    CX11WindowFactory*  pcFactory;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    pcFactory->ExecuteEvents();
    return mulWindow != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CX11Window::CanWaitForEvents(void)
{
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::WaitForEvents(int64 iMilliseconds)
{
    CX11WindowFactory*  pcFactory;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    pcFactory->WaitForEvents(iMilliseconds);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::PresentIfPending(void)
{
    CNativeCanvas*  pcNativeCanvas;

    if (!mbPresentPending)
    {
        return;
    }
    if (!mbPresentArea)
    {
        CNativeWindow::Present();
        return;
    }

    //Only the area that changed.
    if (BeginPresent())
    {
        pcNativeCanvas = mpcWindow->GetCanvas()->GetNativeCanvas();
        if (pcNativeCanvas)
        {
            PresentPixels(pcNativeCanvas, mcPresentArea.GetLeft(), mcPresentArea.GetTop(), mcPresentArea.GetRight(), mcPresentArea.GetBottom());
        }
        EndPresent();
    }
    mbPresentPending = false;
    mbPresentArea = false;
}


//////////////////////////////////////////////////////////////////////////
//
// A disabled window, the owner of a modal dialog, ignores the keyboard,
// the pointer and being closed.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::HandleEvent(XEvent* puEvent)
{
    CX11WindowFactory*  pcFactory;
    CX11Input*          pcInput;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    pcInput = pcFactory->GetInput();

    //The X server has read the last shared present, but nothing was waiting for it.
    if (puEvent->type == pcFactory->GetSharedCompletionEvent())
    {
        mbSharedPresenting = false;
        return;
    }

    //A key or button released over a disabled window still reaches the input, it was pressed before the dialog
    //opened, as the enter that chose the menu item that opened it was.  Otherwise it would stay down.
    if (!mbEnabled)
    {
        switch (puEvent->type)
        {
            case KeyRelease:
                if (pcInput)
                {
                    pcInput->Key(puEvent->xkey.keycode, false);
                }
                return;
            case ButtonRelease:
                if (pcInput)
                {
                    pcInput->MouseButton(puEvent->xbutton.button, false);
                }
                return;
            case KeyPress:
            case ButtonPress:
            case MotionNotify:
            case EnterNotify:
            case LeaveNotify:
            case FocusIn:
            case FocusOut:
            case ClientMessage:
                return;
        }
    }

    switch (puEvent->type)
    {
        case Expose:
        {
            //The last of a run of exposes presents the whole canvas.
            if (puEvent->xexpose.count == 0)
            {
                mbPresentPending = true;
                mbPresentArea = false;
            }
            break;
        }
        case ConfigureNotify:
        {
            //The window is laid out again at its new size.
            miWidth = puEvent->xconfigure.width;
            miHeight = puEvent->xconfigure.height;
            mpcWindow->Invalidate();
            break;
        }
        case ClientMessage:
        {
            //The application can keep the window open, to ask to save its changes say.
            if ((puEvent->xclient.message_type == pcFactory->GetProtocolsAtom()) && ((Atom)puEvent->xclient.data.l[0] == pcFactory->GetDeleteWindowAtom()) && ApproveClose())
            {
                Stop();
                DestroyNativeWindow();
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
        case MapNotify:
        {
            //A dialog takes the keys from its owner.  A tool window appearing, dragged out of its owner say, doesn't.
            if (mpcOwner && !mbTool)
            {
                XSetInputFocus(pcFactory->GetDisplay(), mulWindow, RevertToParent, CurrentTime);
            }
            break;
        }
    }
}


//////////////////////////////////////////////////////////////////////////
//
// What a paint signalled is put to the server now, not when the events
// are next handled.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::PresentPending(void)
{
    PresentIfPending();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::SignalPresent(void)
{
    mbPresentPending = true;
    mbPresentArea = false;
}


//////////////////////////////////////////////////////////////////////////
//
// Only pcArea is put to the server, unless the whole canvas is pending
// already.  Areas signalled before a present are presented together.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::SignalPresentArea(CInt32Rect* pcArea)
{
    if (!mbPresentPending)
    {
        mbPresentPending = true;
        mbPresentArea = true;
        mcPresentArea.Init(pcArea);
    }
    else if (mbPresentArea)
    {
        mcPresentArea.Init(
            (pcArea->GetLeft() < mcPresentArea.GetLeft()) ? pcArea->GetLeft() : mcPresentArea.GetLeft(),
            (pcArea->GetTop() < mcPresentArea.GetTop()) ? pcArea->GetTop() : mcPresentArea.GetTop(),
            (pcArea->GetRight() > mcPresentArea.GetRight()) ? pcArea->GetRight() : mcPresentArea.GetRight(),
            (pcArea->GetBottom() > mcPresentArea.GetBottom()) ? pcArea->GetBottom() : mcPresentArea.GetBottom());
    }
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::GetRectangle(CInt32Rect* pcDest)
{
    int     iWidth;
    int     iHeight;

    //Before the X window is made (or after it is gone) its client area is what it will be made at, so the window can
    //be laid out before it is shown, as it is on the other platforms.
    if (mulWindow == 0)
    {
        GetCreatedSize(&iWidth, &iHeight);
        pcDest->Init(0, 0, iWidth, iHeight);
        return;
    }
    pcDest->Init(0, 0, miWidth, miHeight);
}


//////////////////////////////////////////////////////////////////////////
//
// The size the X window is made at: as asked for (SetRequestedSize or
// SetScreenRectangle), or X11_WINDOW_WIDTH by X11_WINDOW_HEIGHT.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::GetCreatedSize(int* piWidth, int* piHeight)
{
    *piWidth = (msRequestedSize.x > 0) ? msRequestedSize.x : X11_WINDOW_WIDTH;
    *piHeight = (msRequestedSize.y > 0) ? msRequestedSize.y : X11_WINDOW_HEIGHT;
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

    //A press always says where it is: the window forgets the pointer is over it while it isn't active, so a click
    //that activates it (or comes after a dialog closes) without the pointer moving would be over nothing.
    if (bPressed || !mbPointerKnown || (x != miPointerX) || (y != miPointerY))
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
    mpcWindow->PlatformDeactivated();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
X11ID CX11Window::GetXWindow(void)
{
    return mulWindow;
}


//////////////////////////////////////////////////////////////////////////
//
// Moves and resizes the client area, or says where it will be made.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::SetScreenRectangle(CInt32Rect* pcRectangle)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;
    int                 iWidth;
    int                 iHeight;

    CNativeWindow::SetScreenRectangle(pcRectangle);
    if (mulWindow == 0)
    {
        return;
    }
    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    iWidth = (pcRectangle->GetWidth() > 0) ? pcRectangle->GetWidth() : 1;
    iHeight = (pcRectangle->GetHeight() > 0) ? pcRectangle->GetHeight() : 1;
    XMoveResizeWindow(psDisplay, mulWindow, pcRectangle->GetLeft(), pcRectangle->GetTop(), iWidth, iHeight);
    //The canvas is laid out at the new size straight away, not when the server says it has changed.
    miWidth = iWidth;
    miHeight = iHeight;
    XFlush(psDisplay);
    mpcWindow->Invalidate();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CX11Window::GetScreenRectangle(CInt32Rect* pcDest)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;
    Window              ulChild;
    int                 x;
    int                 y;

    if (mulWindow == 0)
    {
        return CNativeWindow::GetScreenRectangle(pcDest);
    }
    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    x = 0;
    y = 0;
    XTranslateCoordinates(psDisplay, mulWindow, RootWindow(psDisplay, pcFactory->GetScreen()), 0, 0, &x, &y, &ulChild);
    pcDest->Init(x, y, x + miWidth, y + miHeight);
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CX11Window::GetPointerScreenPosition(SInt32Vec2* psDest)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;
    Window              ulRoot;
    Window              ulChild;
    int                 iRootX;
    int                 iRootY;
    int                 iWindowX;
    int                 iWindowY;
    unsigned int        uiMask;

    if (mulWindow == 0)
    {
        return false;
    }
    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    if (!XQueryPointer(psDisplay, mulWindow, &ulRoot, &ulChild, &iRootX, &iRootY, &iWindowX, &iWindowY, &uiMask))
    {
        return false;
    }
    psDest->Init(iRootX, iRootY);
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CX11Window::GetScreenArea(CInt32Rect* pcDest)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    if (psDisplay == NULL)
    {
        return CNativeWindow::GetScreenArea(pcDest);
    }
    pcDest->Init(0, 0, DisplayWidth(psDisplay, pcFactory->GetScreen()), DisplayHeight(psDisplay, pcFactory->GetScreen()));
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The cursor font's shapes, made the first time each is shown.
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::SetCursor(ENativeCursor eCursor)
{
    static unsigned int auiShapes[NC_NumCursors] = { XC_left_ptr, XC_sb_h_double_arrow, XC_sb_v_double_arrow, XC_bottom_right_corner, XC_bottom_left_corner, XC_fleur };
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;

    if ((eCursor == meCursor) || (eCursor < 0) || (eCursor >= NC_NumCursors))
    {
        return;
    }
    CNativeWindow::SetCursor(eCursor);
    if (mulWindow == 0)
    {
        return;
    }
    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    if (maulCursors[eCursor] == 0)
    {
        maulCursors[eCursor] = XCreateFontCursor(psDisplay, auiShapes[eCursor]);
    }
    XDefineCursor(psDisplay, mulWindow, maulCursors[eCursor]);
    XFlush(psDisplay);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::FreeCursors(void)
{
    CX11WindowFactory*  pcFactory;
    Display*            psDisplay;
    int                 i;

    pcFactory = (CX11WindowFactory*)mpcWindowFactory;
    psDisplay = pcFactory->GetDisplay();
    for (i = 0; i < NC_NumCursors; i++)
    {
        if (maulCursors[i])
        {
            XFreeCursor(psDisplay, maulCursors[i]);
            maulCursors[i] = 0;
        }
    }
    meCursor = NC_Arrow;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::Raise(void)
{
    CX11WindowFactory*  pcFactory;

    if (mulWindow)
    {
        pcFactory = (CX11WindowFactory*)mpcWindowFactory;
        XRaiseWindow(pcFactory->GetDisplay(), mulWindow);
        XFlush(pcFactory->GetDisplay());
    }
}


bool CX11Window::IsCreated(void) { return mulWindow != 0; }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CX11Window::TitleChanged(void)
{
    CX11WindowFactory*  pcFactory;

    if (mulWindow)
    {
        pcFactory = (CX11WindowFactory*)mpcWindowFactory;
        XStoreName(pcFactory->GetDisplay(), mulWindow, mpcWindow->GetWindowTitle());
        XFlush(pcFactory->GetDisplay());
    }
}

