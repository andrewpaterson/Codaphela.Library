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
#ifndef __SETTINGS_DIALOG_H__
#define __SETTINGS_DIALOG_H__
#include "StandardLib/Array.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"
#include "Dialog.h"
#include "GridContainer.h"
#include "ListContainer.h"


//A page of an application's Settings dialog (CSettingsDialog): what it's called in the list on the left, what it shows, and what it does when OK closes the
//dialog.  It works on copies until then, so Cancel changes nothing.
class CSettingsPage : public CObject
{
CONSTRUCTABLE(CSettingsPage);
DESTRUCTABLE(CSettingsPage);
public:
	void					Init(void);
	void					Class(void) override;
	void					Free(void) override;
	bool					Save(CObjectWriter* pcFile) override;
	bool					Load(CObjectReader* pcFile) override;

	virtual const char*		GetTitle(void);
	virtual Ptr<CComponent>	GetBody(void);
	virtual Ptr<CComponent>	GetFirstEditor(void);
	virtual void			Keep(void);
};


//An application's Settings (File, Settings), as IntelliJ IDEA's: its pages on the left and the one selected on the right.
class CSettingsDialog : public CObject
{
CONSTRUCTABLE(CSettingsDialog);
DESTRUCTABLE(CSettingsDialog);
protected:
	Ptr<CDialog>				mpDialog;
	Ptr<CListContainer>			mpPageList;
	Ptr<CGridContainer>			mpPageHolder;
	CArray<CSettingsPage>	maPages;

public:
	void			Init(Ptr<CWindow> pOwner, SDialogLook* psLook);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			AddPage(Ptr<CSettingsPage> pPage);
	bool			Show(void);
	Ptr<CDialog>	GetDialog(void);

protected:
	Ptr<CComponent>	CreateBody(void);
	void			PageSelected(void);
};


#endif // __SETTINGS_DIALOG_H__

