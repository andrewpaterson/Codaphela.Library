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
#ifndef __INFO_POPUP_H__
#define __INFO_POPUP_H__
#include "ComponentEvent.h"
#include "Popup.h"
#include "TextArea.h"
#include "WindowTheme.h"


#define INFO_POPUP_GAP	2	//Between the point it is shown at and the popup.


//A popup that shows a few lines of text by something in the window: what a name is declared as, the parameters of
//the call being typed, an expression's type.  The text is a text area's, so it can be code in a family's faces with
//parts of it coloured or bold (AddSpan), and it isn't edited.
//
//It is shown at a point of the window (Show): under it, or with bAbove over it, as over the line the caret is on;
//it is moved back in where it would pass the window's edge.  The pointer passes through it to what is under it, and
//it takes no keys: whoever shows it hides it (Hide) when what it is about has gone, and shows it again to move it.
//A press anywhere closes it, as it does every popup; other popups can be opened over it and stay when it hides.
//
//One that shows more, as a name's documentation, can be given a maximum height (SetMaximumHeight): taller text scrolls
//in it, by the wheel and by whoever shows it (Scroll, ScrollPages), as it has no keys of its own.  It can take the
//pointer (SetTakesPointer) rather than let it pass: then the wheel over it scrolls it and a press on it calls its
//press event with the position of the character pressed, MAX_SIZE for a press beside its text, for text with parts
//that go somewhere.  A press in a popup closes none, so it stays; but the press takes the keys from whatever had them,
//as a press on anything that can't have them does, and the event is called before the press is given to anything: who
//listens gives the keys back, and does what the press asks, afterwards (at its next tick), not in the event.
//
//It is an object: make it with OMalloc and keep it in the root, or an object that is, while it is used.
class CInfoPopup;
typedef CComponentValueEvent<CInfoPopup, size>	CInfoPopupPressEvent;  //The position of the character pressed, or MAX_SIZE.


class CInfoPopup : public CObject
{
CONSTRUCTABLE(CInfoPopup);
DESTRUCTABLE(CInfoPopup);
protected:
	Ptr<CWindow>		mpWindow;
	Ptr<CWindowTheme>	mpTheme;
	Ptr<CTextArea>		mpText;
	Ptr<CPopup>			mpPopup;  //While it is shown.
	SInt32Vec2			msPoint;  //Where it was last shown, and whether over it.
	bool				mbAbove;
	int32				miMaximumHeight;  //Of the text, -1 for none.
	bool				mbTakesPointer;
	CInfoPopupPressEvent	mcPressed;

public:
	void			Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, Ptr<CFontFamily> pFamily);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			SetText(const char* szText);
	const char*		GetText(void);
	bool			AddSpan(size uiStart, size uiLength, ARGB32 sColour);
	bool			AddSpan(size uiStart, size uiLength, STextStyleOverride* psOverride);
	void			SetMaximumHeight(int32 iHeight);
	int32			GetMaximumHeight(void) const;
	void			SetTakesPointer(bool bTakesPointer);
	bool			IsTakesPointer(void) const;
	CInfoPopupPressEvent*	GetPressEvent(void);
	void			Show(SInt32Vec2 sPoint, bool bAbove);
	bool			Scroll(int32 iLines);
	bool			ScrollPages(int32 iPages);
	bool			IsScrolling(void);
	int32			GetScrollPosition(void);
	bool			Hide(void);
	bool			IsShown(void);
	SInt32Vec2		GetPoint(void) const;
	bool			IsAbove(void) const;
	Ptr<CTextArea>	GetTextArea(void) const;
	Ptr<CPopup>		GetPopup(void) const;

protected:
	void			PopupClosed(void);
	void			FocusPressed(void);
	void			Closed(void);
	bool			ScrollBy(int32 iPixels);
};


#endif // __INFO_POPUP_H__

