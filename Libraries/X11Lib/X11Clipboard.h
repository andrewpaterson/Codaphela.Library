#ifndef __X11_CLIPBOARD_H__
#define __X11_CLIPBOARD_H__
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
#include "BaseLib/Chars.h"
#include "WindowLib/NativeClipboard.h"


//The X11 headers are only included by the .cpp files, see X11WindowFactory.h.
struct _XDisplay;
union _XEvent;
typedef unsigned long	X11ID;


//The display's CLIPBOARD selection.  X keeps no copy of copied text: the program that copied it owns the selection and
//gives the text to whoever asks for it, so the text stays here, with a hidden window to own the selection and answer
//the requests, for as long as the factory lives.  Every window's clipboard shares it, so text copied in a dialog can
//still be pasted once the dialog has closed.  The factory gives it the selection events for its window.
//
//The text is given and taken as UTF8_STRING, or STRING (Latin-1) for older programs, converted with BaseLib's CLatin1
//and CUnicodeWriter.  Text bigger than one X request (INCR transfers) isn't supported yet.
class CX11Selection
{
protected:
	_XDisplay*	mpsDisplay;  //NULL if the factory has no display, then nothing is copied or pasted.
	X11ID		mulWindow;  //Hidden, owns the selection.
	X11ID		mulClipboardAtom;
	X11ID		mulUTF8Atom;
	X11ID		mulTargetsAtom;
	X11ID		mulIncrAtom;
	X11ID		mulPropertyAtom;  //Where the text another program gives is put on mulWindow.
	CChars		mszText;  //What this program copied, while it owns the selection.

public:
	void	Init(_XDisplay* psDisplay, int iScreen);
	void	Kill(void);

	bool	SetText(const char* szText);
	bool	GetText(CChars* pszDest);
	bool	HandleEvent(_XEvent* puEvent);

protected:
	bool	Request(X11ID ulTarget, CChars* pszDest);
	void	Answer(_XEvent* puEvent);
	bool	WaitForNotify(_XEvent* puEvent);
};


//A window's clipboard; all of them are the factory's selection.
class CX11Clipboard : public CNativeClipboard
{
CONSTRUCTABLE(CX11Clipboard);
protected:
	bool	SetText(const char* szText) override;
	bool	GetText(CChars* pszDest) override;
};


#endif // __X11_CLIPBOARD_H__

