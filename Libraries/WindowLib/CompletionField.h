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
#ifndef __COMPLETION_FIELD_H__
#define __COMPLETION_FIELD_H__
#include "BaseLib/ArrayChars.h"
#include "TextField.h"
#include "MenuContainer.h"


//A text field that offers what could be meant by what is typed in it, like the fields of IntelliJ's dialogs that
//complete a class or a file: a list under the field, as wide as it, each row a text and, at its right, a note (the
//path of a file, say).
//
//The field doesn't know what completes: whoever owns it listens to its change event and gives it the completions for
//its text as it is then (SetCompletions), or none, which closes the list.  The list is a menu in a popup the field
//owns.  The keys stay with the field: typing goes on, down and up move through the list, enter takes the row they
//chose, and escape closes the list.  A row can be pressed too.  Taking one makes it the field's text, with the caret
//at its end, and closes the list; the change that makes isn't completed again.
//
//With no row chosen enter is the window's default button's, as it is for a text field; so what was typed can be
//accepted with the list open.  The list closes when the field loses the keys.
class CCompletionField : public CTextField
{
CONSTRUCTABLE(CCompletionField);
DESTRUCTABLE(CCompletionField);
protected:
	Ptr<CMenuStyle>		mpStyle;
	Ptr<CMenuContainer>	mpMenu;
	bool				mbTaking;  //A completion is becoming the text: that change isn't one to complete.

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				SetCompletions(CArrayChars* paszTexts, CArrayChars* paszNotes);
	void				CloseCompletions(void);
	bool				IsCompleting(void);
	size				NumCompletions(void);
	const char*			GetCompletion(size uiIndex);
	const char*			GetCompletionNote(size uiIndex);
	bool				TakeCompletion(size uiIndex);
	Ptr<CMenuContainer>	GetMenu(void) const;

	bool				WantsEnter(void) override;
	void				PopupItemClicked(Ptr<CComponent> pContent, Ptr<CComponent> pItem) override;
	void				FocusOrMouseOverChanged(void) override;
	void				Up(void);
	void				Down(void);
	void				Enter(void);

protected:
	void				Open(void);
	void				Take(const char* szText);
};


#endif // __COMPLETION_FIELD_H__

