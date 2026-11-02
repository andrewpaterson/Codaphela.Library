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
#include "BaseLib/Logger.h"
#include "BaseLib/ArrayUint8.h"
#include "BaseLib/ArrayUint16.h"
#include "BaseLib/UnicodeWriter.h"
#include "BaseLib/UTF8.h"
#include "BaseLib/UTF16.h"
#include "WindowLib/Window.h"
#include "WindowLib/Clipboard.h"
#include "WinGDIWindow.h"
#include "WinGDIWindowFactory.h"
#include "WinGDIClipboard.h"


#define WIN_GDI_CLIPBOARD_OPEN_ATTEMPTS		10  //Another program may have the clipboard open for a moment.
#define WIN_GDI_CLIPBOARD_OPEN_WAIT			10  //Milliseconds between attempts.


//////////////////////////////////////////////////////////////////////////
//
// Puts szText on the clipboard as UTF16 with carriage return and line
// feed line endings.  The clipboard must be opened by a window to be
// written, the clipboard's own if it is on the screen, otherwise the
// factory's.
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIClipboard::SetText(const char* szText)
{
    CChars          szWindows;
    CArrayUint16    auiUTF16;
    CUTF8           cUTF8;
    CUnicodeWriter  cWriter;
    int             iResult;
    HWND            hOwner;
    HGLOBAL         hGlobal;
    void*           pvText;
    size            i;
    bool            bResult;

    hOwner = GetOwner();
    if (hOwner == NULL)
    {
        return gcLogger.Error2(__METHOD__, " There is no window to own the clipboard.", NULL);
    }

    //Line feeds become carriage return and line feed pairs, those already in pairs are left.
    szWindows.Init();
    for (i = 0; szText[i] != '\0'; i++)
    {
        if ((szText[i] == '\n') && ((i == 0) || (szText[i - 1] != '\r')))
        {
            szWindows.Append('\r');
        }
        szWindows.Append(szText[i]);
    }

    auiUTF16.Init();
    cUTF8.Init(szWindows.Text());
    cWriter.Init(&cUTF8, UE_UTF16LE, &auiUTF16);
    iResult = cWriter.WriteCharacters();
    cWriter.Kill();
    cUTF8.Kill();
    szWindows.Kill();
    if (iResult != -1)
    {
        auiUTF16.Kill();
        return gcLogger.Error2(__METHOD__, " The text isn't valid UTF8.", NULL);
    }
    auiUTF16.Push(0);

    hGlobal = GlobalAlloc(GMEM_MOVEABLE, (SIZE_T)auiUTF16.ByteSize());
    if (hGlobal == NULL)
    {
        auiUTF16.Kill();
        return gcLogger.Error2(__METHOD__, " Could not allocate the clipboard's text.", NULL);
    }
    pvText = GlobalLock(hGlobal);
    memcpy(pvText, auiUTF16.GetData(), auiUTF16.ByteSize());
    GlobalUnlock(hGlobal);
    auiUTF16.Kill();

    if (!Open(hOwner))
    {
        GlobalFree(hGlobal);
        return gcLogger.Error2(__METHOD__, " Could not open the clipboard.", NULL);
    }

    //Once set the clipboard owns the memory.
    EmptyClipboard();
    bResult = SetClipboardData(CF_UNICODETEXT, hGlobal) != NULL;
    if (!bResult)
    {
        GlobalFree(hGlobal);
    }
    CloseClipboard();

    if (!bResult)
    {
        return gcLogger.Error2(__METHOD__, " Could not set the clipboard's text.", NULL);
    }
    return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the clipboard's text as UTF8 with line feed line endings.
// Windows makes CF_UNICODETEXT from CF_TEXT, so text another program put
// on the clipboard in either is found.
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIClipboard::GetText(CChars* pszDest)
{
    HGLOBAL         hGlobal;
    WCHAR*          pwszText;
    CArrayUint8     auiUTF8;
    CUTF16          cUTF16;
    CUnicodeWriter  cWriter;
    int             iResult;
    char*           szUTF8;
    size            uiLength;
    size            i;
    bool            bAppended;

    if (!Open(GetOwner()))
    {
        return false;
    }

    bAppended = false;
    hGlobal = IsClipboardFormatAvailable(CF_UNICODETEXT) ? GetClipboardData(CF_UNICODETEXT) : NULL;
    pwszText = hGlobal ? (WCHAR*)GlobalLock(hGlobal) : NULL;
    if (pwszText)
    {
        uiLength = wcslen(pwszText);
        if (uiLength > 0)
        {
            auiUTF8.Init();
            cUTF16.Init((uint16*)pwszText, uiLength);
            cWriter.Init(&cUTF16, UE_UTF8, &auiUTF8);
            iResult = cWriter.WriteCharacters();
            cWriter.Kill();
            cUTF16.Kill();

            if (iResult == -1)
            {
                //Carriage return and line feed pairs, and carriage returns alone, become line feeds.
                szUTF8 = (char*)auiUTF8.GetData();
                uiLength = auiUTF8.NumElements();
                for (i = 0; i < uiLength; i++)
                {
                    if (szUTF8[i] == '\r')
                    {
                        if ((i + 1 >= uiLength) || (szUTF8[i + 1] != '\n'))
                        {
                            pszDest->Append('\n');
                        }
                    }
                    else
                    {
                        pszDest->Append(szUTF8[i]);
                    }
                }
                bAppended = true;
            }
            else
            {
                gcLogger.Error2(__METHOD__, " The clipboard's text isn't valid UTF16.", NULL);
            }
            auiUTF8.Kill();
        }
        GlobalUnlock(hGlobal);
    }

    CloseClipboard();
    return bAppended;
}


//////////////////////////////////////////////////////////////////////////
//
// The clipboard's window if it is on the screen, otherwise the factory's
// main window.  NULL if neither is.
//
//////////////////////////////////////////////////////////////////////////
HWND CWinGDIClipboard::GetOwner(void)
{
    CWinGDIWindow*          pcWindow;
    CWinGDIWindowFactory*   pcFactory;
    HWND                    hWnd;

    hWnd = NULL;
    pcWindow = (CWinGDIWindow*)mpcClipboard->GetWindow()->GetNativeWindow();
    if (pcWindow)
    {
        hWnd = pcWindow->GetHWnd();
    }
    if (hWnd == NULL)
    {
        pcFactory = (CWinGDIWindowFactory*)mpcWindowFactory;
        hWnd = pcFactory->GetHWnd();
    }
    return hWnd;
}


//////////////////////////////////////////////////////////////////////////
//
// Tries a few times; another program may have the clipboard open.
// hOwner may be NULL to read the clipboard.
//
//////////////////////////////////////////////////////////////////////////
bool CWinGDIClipboard::Open(HWND hOwner)
{
    int     i;

    for (i = 0; i < WIN_GDI_CLIPBOARD_OPEN_ATTEMPTS; i++)
    {
        if (OpenClipboard(hOwner))
        {
            return true;
        }
        Sleep(WIN_GDI_CLIPBOARD_OPEN_WAIT);
    }
    return false;
}

