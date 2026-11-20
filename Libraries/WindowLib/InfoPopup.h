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
//It is an object: make it with OMalloc and keep it in the root, or an object that is, while it is used.
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
	void			Show(SInt32Vec2 sPoint, bool bAbove);
	bool			Hide(void);
	bool			IsShown(void);
	SInt32Vec2		GetPoint(void) const;
	bool			IsAbove(void) const;
	Ptr<CTextArea>	GetTextArea(void) const;
	Ptr<CPopup>		GetPopup(void) const;

protected:
	void			PopupClosed(void);
};


#endif // __INFO_POPUP_H__

