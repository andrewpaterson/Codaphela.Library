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
#include <string.h>
#include <unistd.h>
#include "BaseLib/Logger.h"
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/UnicodeWriter.h"
#include "BaseLib/UTF8.h"
#include "BaseLib/Latin1.h"
#include "X11WindowFactory.h"
#include "X11Clipboard.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>


#define X11_CLIPBOARD_WAIT_POLLS		200  //How many times to look for another program's answer...
#define X11_CLIPBOARD_POLL_MICROSECONDS	5000  //...this far apart, a second in all.


//////////////////////////////////////////////////////////////////////////
//
// Converts with pcReader and a CUnicodeWriter, then appends the bytes.
// Returns false, appending nothing, if the text isn't valid.
//
//////////////////////////////////////////////////////////////////////////
static bool AppendConverted(CChars* pszDest, CUnicodeReader* pcReader, EUnicodeEncoding eEncoding)
{
    CUnicodeWriter  cWriter;
    CArrayUint8     auiConverted;
    bool            bResult;

    auiConverted.Init();
    cWriter.Init(pcReader, eEncoding, &auiConverted);
    bResult = cWriter.WriteCharacters() == -1;
    cWriter.Kill();
    if (bResult && (auiConverted.NumElements() > 0))
    {
        pszDest->Append((const char*)auiConverted.GetData(), auiConverted.NumElements());
    }
    auiConverted.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Latin-1 is never invalid, see CLatin1.
//
//////////////////////////////////////////////////////////////////////////
static void AppendLatin1AsUTF8(CChars* pszDest, uint8* puiLatin1, size uiLength)
{
    CLatin1     cLatin1;

    cLatin1.Init(puiLatin1, uiLength);
    AppendConverted(pszDest, &cLatin1, UE_UTF8);
    cLatin1.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A character Latin-1 doesn't have becomes a question mark.  Returns
// false if szUTF8 isn't valid UTF8.
//
//////////////////////////////////////////////////////////////////////////
static bool AppendUTF8AsLatin1(CChars* pszDest, const char* szUTF8)
{
    CUTF8   cUTF8;
    bool    bResult;

    cUTF8.Init(szUTF8);
    bResult = AppendConverted(pszDest, &cUTF8, UE_Latin1);
    cUTF8.Kill();
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// psDisplay can be NULL, the factory couldn't open its display.
//
//////////////////////////////////////////////////////////////////////////
void CX11Selection::Init(_XDisplay* psDisplay, int iScreen)
{
    XSetWindowAttributes    sAttributes;

    mpsDisplay = psDisplay;
    mulWindow = 0;
    mulClipboardAtom = 0;
    mulUTF8Atom = 0;
    mulTargetsAtom = 0;
    mulIncrAtom = 0;
    mulPropertyAtom = 0;
    mszText.Init();
    if (mpsDisplay == NULL)
    {
        return;
    }

    //An input only window is never shown.  Selection events reach it whatever its event mask.
    memset(&sAttributes, 0, sizeof(XSetWindowAttributes));
    mulWindow = XCreateWindow(mpsDisplay, RootWindow(mpsDisplay, iScreen), 0, 0, 1, 1, 0, 0, InputOnly, NULL, 0, &sAttributes);
    mulClipboardAtom = XInternAtom(mpsDisplay, "CLIPBOARD", False);
    mulUTF8Atom = XInternAtom(mpsDisplay, "UTF8_STRING", False);
    mulTargetsAtom = XInternAtom(mpsDisplay, "TARGETS", False);
    mulIncrAtom = XInternAtom(mpsDisplay, "INCR", False);
    mulPropertyAtom = XInternAtom(mpsDisplay, "CODAPHELA_CLIPBOARD", False);
}


//////////////////////////////////////////////////////////////////////////
//
// Before the display is closed.  Text this program copied goes with it.
//
//////////////////////////////////////////////////////////////////////////
void CX11Selection::Kill(void)
{
    if (mpsDisplay && mulWindow)
    {
        XDestroyWindow(mpsDisplay, mulWindow);
    }
    mulWindow = 0;
    mszText.Kill();
    mpsDisplay = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Takes the selection with szText (UTF8).  Other programs ask for it
// when they paste.
//
//////////////////////////////////////////////////////////////////////////
bool CX11Selection::SetText(const char* szText)
{
    if (mpsDisplay == NULL)
    {
        return false;
    }

    mszText.Set(szText);
    XSetSelectionOwner(mpsDisplay, mulClipboardAtom, mulWindow, CurrentTime);
    if (XGetSelectionOwner(mpsDisplay, mulClipboardAtom) != mulWindow)
    {
        mszText.Clear();
        return gcLogger.Error2(__METHOD__, " Could not take the clipboard selection.", NULL);
    }
    XFlush(mpsDisplay);
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the selection's text as UTF8.  Text this program copied is
// taken straight from it, otherwise the owner is asked for UTF8_STRING,
// then STRING, and its answer waited for.
//
//////////////////////////////////////////////////////////////////////////
bool CX11Selection::GetText(CChars* pszDest)
{
    X11ID   ulOwner;

    if (mpsDisplay == NULL)
    {
        return false;
    }

    ulOwner = XGetSelectionOwner(mpsDisplay, mulClipboardAtom);
    if (ulOwner == None)
    {
        return false;
    }
    if (ulOwner == mulWindow)
    {
        if (mszText.Empty())
        {
            return false;
        }
        pszDest->Append(&mszText);
        return true;
    }

    if (Request(mulUTF8Atom, pszDest))
    {
        return true;
    }
    return Request(XA_STRING, pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// Asks the selection's owner for its text as ulTarget, put in a property
// of the hidden window.  Returns false if the owner can't give it as
// that, or doesn't answer.
//
//////////////////////////////////////////////////////////////////////////
bool CX11Selection::Request(X11ID ulTarget, CChars* pszDest)
{
    XEvent          uEvent;
    Atom            ulType;
    int             iFormat;
    unsigned long   ulItems;
    unsigned long   ulAfter;
    unsigned char*  puc;
    bool            bResult;

    XDeleteProperty(mpsDisplay, mulWindow, mulPropertyAtom);
    XConvertSelection(mpsDisplay, mulClipboardAtom, ulTarget, mulPropertyAtom, mulWindow, CurrentTime);
    XFlush(mpsDisplay);
    if (!WaitForNotify(&uEvent))
    {
        return gcLogger.Error2(__METHOD__, " The program with the clipboard didn't answer.", NULL);
    }
    if (uEvent.xselection.property == None)
    {
        return false;
    }

    puc = NULL;
    if (XGetWindowProperty(mpsDisplay, mulWindow, mulPropertyAtom, 0, 0x1FFFFFFF, True, AnyPropertyType, &ulType, &iFormat, &ulItems, &ulAfter, &puc) != Success)
    {
        return false;
    }

    bResult = false;
    if (ulType == mulIncrAtom)
    {
        gcLogger.Error2(__METHOD__, " Pasting text too big for one X request (INCR) isn't supported.", NULL);
    }
    else if ((iFormat == 8) && (ulItems > 0))
    {
        if (ulType == XA_STRING)
        {
            AppendLatin1AsUTF8(pszDest, puc, (size)ulItems);
        }
        else
        {
            pszDest->Append((const char*)puc, (size)ulItems);
        }
        bResult = true;
    }

    if (puc)
    {
        XFree(puc);
    }
    return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The owner's answer comes to the hidden window as a SelectionNotify.
// Other events are left queued for the factory.
//
//////////////////////////////////////////////////////////////////////////
bool CX11Selection::WaitForNotify(_XEvent* puEvent)
{
    int     i;

    for (i = 0; i < X11_CLIPBOARD_WAIT_POLLS; i++)
    {
        if (XCheckTypedWindowEvent(mpsDisplay, mulWindow, SelectionNotify, puEvent))
        {
            return true;
        }
        usleep(X11_CLIPBOARD_POLL_MICROSECONDS);
    }
    return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Returns true if the event was for the hidden window, then nothing else
// needs it.  Another program asking for the text is answered; losing the
// selection to another program's copy drops the text.
//
//////////////////////////////////////////////////////////////////////////
bool CX11Selection::HandleEvent(_XEvent* puEvent)
{
    if ((mpsDisplay == NULL) || (puEvent->xany.window != mulWindow))
    {
        return false;
    }

    if (puEvent->type == SelectionRequest)
    {
        Answer(puEvent);
    }
    else if (puEvent->type == SelectionClear)
    {
        mszText.Clear();
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Puts the text, or the targets it can be given as, in the property the
// requestor asked for and tells it.  A refusal is a SelectionNotify with
// no property.
//
//////////////////////////////////////////////////////////////////////////
void CX11Selection::Answer(_XEvent* puEvent)
{
    XSelectionRequestEvent*     psRequest;
    XEvent                      uReply;
    Atom                        aulTargets[3];
    Atom                        ulProperty;
    CChars                      szAnswer;
    bool                        bValid;
    unsigned long               ulMaxBytes;

    psRequest = &puEvent->xselectionrequest;
    memset(&uReply, 0, sizeof(XEvent));
    uReply.xselection.type = SelectionNotify;
    uReply.xselection.display = psRequest->display;
    uReply.xselection.requestor = psRequest->requestor;
    uReply.xselection.selection = psRequest->selection;
    uReply.xselection.target = psRequest->target;
    uReply.xselection.time = psRequest->time;
    uReply.xselection.property = None;

    //Programs written before ICCCM ask with no property, the target is used.
    ulProperty = (psRequest->property != None) ? psRequest->property : psRequest->target;

    //XMaxRequestSize is in four byte units; leave room for the request's header.
    ulMaxBytes = (unsigned long)XMaxRequestSize(mpsDisplay) * 4 - 64;

    if ((psRequest->selection == mulClipboardAtom) && !mszText.Empty())
    {
        if (psRequest->target == mulTargetsAtom)
        {
            aulTargets[0] = mulTargetsAtom;
            aulTargets[1] = mulUTF8Atom;
            aulTargets[2] = XA_STRING;
            XChangeProperty(mpsDisplay, psRequest->requestor, ulProperty, XA_ATOM, 32, PropModeReplace, (unsigned char*)aulTargets, 3);
            uReply.xselection.property = ulProperty;
        }
        else if ((psRequest->target == mulUTF8Atom) || (psRequest->target == XA_STRING))
        {
            szAnswer.Init();
            bValid = true;
            if (psRequest->target == XA_STRING)
            {
                bValid = AppendUTF8AsLatin1(&szAnswer, mszText.Text());
            }
            else
            {
                szAnswer.Append(&mszText);
            }

            if (!bValid)
            {
                gcLogger.Error2(__METHOD__, " The copied text isn't valid UTF8.", NULL);
            }
            else if ((unsigned long)szAnswer.Length() > ulMaxBytes)
            {
                gcLogger.Error2(__METHOD__, " Copying text too big for one X request (INCR) isn't supported.", NULL);
            }
            else
            {
                XChangeProperty(mpsDisplay, psRequest->requestor, ulProperty, psRequest->target, 8, PropModeReplace, (unsigned char*)szAnswer.Text(), (int)szAnswer.Length());
                uReply.xselection.property = ulProperty;
            }
            szAnswer.Kill();
        }
    }

    XSendEvent(mpsDisplay, psRequest->requestor, False, NoEventMask, &uReply);
    XFlush(mpsDisplay);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CX11Clipboard::SetText(const char* szText)
{
    return ((CX11WindowFactory*)mpcWindowFactory)->GetSelection()->SetText(szText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CX11Clipboard::GetText(CChars* pszDest)
{
    return ((CX11WindowFactory*)mpcWindowFactory)->GetSelection()->GetText(pszDest);
}

