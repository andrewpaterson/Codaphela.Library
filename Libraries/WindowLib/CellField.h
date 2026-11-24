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
#ifndef __CELL_FIELD_H__
#define __CELL_FIELD_H__
#include "TextField.h"
#include "ComponentEvent.h"


//How an edit in a cell's field ended.
enum ECellFieldEnd
{
	CFE_Commit,		//Enter, or the field losing focus: what was typed is kept.
	CFE_Cancel,		//Escape: what the cell had is kept.
	CFE_Next,		//Tab: kept, and the next cell edited.
	CFE_Previous,	//Shift tab.
	CFE_Up,			//Kept, and the row above selected.
	CFE_Down,
};


class CCellField;
typedef CComponentValueEvent<CCellField, int32>	CCellFieldEvent;  //ECellFieldEnd.


//The text field a cell is edited in, over the cell (a table's, a tree table's).  Enter, escape, up, down, tab and shift
//tab are its own: each ends the edit, and its ended event says how, for the table to keep what was typed or not and
//move on.  Focus going elsewhere keeps what was typed, as IntelliJ's table does.
class CCellField : public CTextField
{
CONSTRUCTABLE(CCellField);
DESTRUCTABLE(CCellField);
protected:
	CCellFieldEvent		mcEnded;

public:
	void				Init(Ptr<CWindow> pWindow, Ptr<CWindowTheme> pTheme, const char* szText);
	void				Class(void) override;
	void				Free(void) override;

	bool				WantsEnter(void) override;
	bool				WantsEscape(void) override;
	bool				IsTabStop(void) const override;
	void				FocusLost(void) override;
	CCellFieldEvent*	GetEndedEvent(void);

	void				Commit(void);
	void				Cancel(void);
	void				CommitUp(void);
	void				CommitDown(void);
	void				CommitNext(void);
	void				CommitPrevious(void);
};


#endif // __CELL_FIELD_H__

