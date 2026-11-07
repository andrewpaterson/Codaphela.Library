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
#ifndef __MESSAGE_DIALOG_H__
#define __MESSAGE_DIALOG_H__
#include "Dialog.h"
#include "ScrollPane.h"


//The most items a message dialog shows before its list scrolls.
#define MESSAGE_DIALOG_MAX_ITEMS	10


//A message in a dialog of its own, shown modally over its owner's window, and the buttons that answer it: OK and Cancel
//to begin with, each captioned as the caller says or Cancel left out, and an extra button before OK, as Save, Don't
//Save and Cancel (see CDialog).  A list of items can go under the message, like the files a question is about; a long
//one scrolls.  The dialog is as big as its message, items and buttons.  Show it from a tick, not from input (see
//CWindow::ShowModal).
class CMessageDialog : public CObject
{
CONSTRUCTABLE(CMessageDialog);
DESTRUCTABLE(CMessageDialog);
protected:
	Ptr<CDialog>		mpDialog;
	Ptr<CGridContainer>	mpBody;
	Ptr<CGridContainer>	mpItems;  //NULL until an item is added.
	Ptr<CScrollPane>	mpItemsPane;
	int32				miRows;  //In the body.
	size				muiItems;

public:
	void				Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, const char* szMessage);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	void				AddItem(const char* szItem);
	Ptr<CDialog>		GetDialog(void);
	bool				Show(void);
	char*				GetClicked(void);

protected:
	Ptr<CLabel>			AddLabel(Ptr<CGridContainer> pGrid, int32 iRow, const char* szText);
};


#endif // __MESSAGE_DIALOG_H__

