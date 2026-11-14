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
#ifndef __KEYMAP_PAGE_H__
#define __KEYMAP_PAGE_H__
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayInt.h"
#include "BaseLib/ArrayTemplate.h"
#include "Button.h"
#include "CommandSet.h"
#include "ComponentInput.h"
#include "KeyCaptureField.h"
#include "Label.h"
#include "ScrollPane.h"
#include "TextField.h"
#include "SettingsDialog.h"


class CKeymapPage;


//A group of the keymap, in the order the page shows them: the part of a command's name before its dot ("File" for
//"File.Save") and what's shown for it.  A command in none goes under "Other", after them.
struct SKeymapGroup
{
	const char*	szPrefix;
	const char*	szCaption;
};


//What watches the page's dialogs for an application's scripted checks (the HCP65 IDE's CIDEScript): Add Shortcut, as
//it's shown, and the settings dialog again after.  ShowCaptureWhenDrawn asks for Add Shortcut to be shown once the
//settings dialog is drawn (CKeymapPage::ApplyEdits's capture:<keys>).  None is needed otherwise.
class CKeymapPageWatcher
{
public:
	virtual void	WatchDialog(Ptr<CDialog> pDialog) = 0;
	virtual void	StopWatchingDialog(void) = 0;
	virtual void	ShowCaptureWhenDrawn(CKeymapPage* pcPage) = 0;
};


//Add Shortcut: the keys pressed, and what giving them to the command would mean.  OK isn't allowed for keys that can't
//be given to it.
class CKeyCaptureDialog : public CObject
{
CONSTRUCTABLE(CKeyCaptureDialog);
DESTRUCTABLE(CKeyCaptureDialog);
protected:
	Ptr<CDialog>			mpDialog;
	Ptr<CKeyCaptureField>	mpField;
	Ptr<CLabel>				mpMessage;
	CKeymapPage*			mpcPage;  //Not held: it holds this dialog.
	size					muiRow;

public:
	void			Init(Ptr<CWindow> pOwner, SDialogLook* psLook, CKeymapPage* pcPage, size uiRow);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	bool			Show(void);
	char*			GetKeys(void);
	void			SetKeys(const char* szKeys);
	Ptr<CDialog>	GetDialog(void);
	bool			Approve(void);

protected:
	void			KeysCaptured(void);
};


//A command and the keys it will have if OK closes the dialog.
struct SKeymapRow
{
	CCommand*	pcCommand;
	CArrayChars	aszKeys;
};


typedef CArrayTemplate<SKeymapRow>	CArrayKeymapRows;


//Settings' Keymap, as IntelliJ IDEA's: every command in the groups a user knows them by, with a filter, and the keys
//each has.  A key belongs to one command, so giving it to another moves it.  Refused are keys that type a character, and
//for a command that works everywhere keys a dialog or text field needs, as they don't have the keys of an editor's.
class CKeymapPage : public CSettingsPage
{
CONSTRUCTABLE(CKeymapPage);
DESTRUCTABLE(CKeymapPage);
protected:
	Ptr<CDialog>				mpDialog;
	Ptr<CCommandSet>			mpCommands;
	CComponentInput*			mpcInput;  //Not held: the application holds it.
	CKeymapPageWatcher*			mpcWatcher;  //Watches Add Shortcut as it does the dialog the page is on, NULL for none.  Not held.
	SKeymapGroup*				masGroups;  //Not held: the application's.
	size						muiNumGroups;
	CChars						mszScriptedKeys;  //What a check has Add Shortcut capture, ApplyEdits's capture:<keys>.
	Ptr<CComponent>				mpBody;
	Ptr<CTextField>				mpFilter;
	Ptr<CListContainer>			mpList;
	Ptr<CScrollPane>			mpScrollPane;
	Ptr<CButton>				mpAdd;
	Ptr<CButton>				mpRemove;
	Ptr<CButton>				mpReset;
	Ptr<CButton>				mpResetAll;
	Ptr<CLabel>					mpStatus;
	Ptr<CKeyCaptureDialog>	mpCapture;  //Add Shortcut, while it's shown.
	CArrayKeymapRows			masRows;
	CArrayInt					maiListRows;  //The row in masRows of each row of the list, -1 for a group's heading.
	bool						mbUpdating;  //The list is being remade, not selected in by the user.

public:
	void			Init(Ptr<CDialog> pDialog, Ptr<CCommandSet> pCommands, CComponentInput* pcInput, SKeymapGroup* asGroups, size uiNumGroups, CKeymapPageWatcher* pcWatcher = NULL);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	const char*		GetTitle(void) override;
	Ptr<CComponent>	GetBody(void) override;
	Ptr<CComponent>	GetFirstEditor(void) override;
	void			Keep(void) override;

	void			ApplyEdits(const char* szEdits);
	bool			DescribeKeys(size uiRow, const char* szKeys, CChars* pszMessage);
	void			ShowCapture(void);

protected:
	Ptr<CComponent>	CreateBody(void);
	Ptr<CButton>	CreateButton(Ptr<CGridContainer> pButtons, int32 iColumn, const char* szCaption);
	void			AddRows(void);
	void			ShowList(void);
	void			ShowKeys(size uiRow, CChars* pszDest);
	bool			IsShown(size uiRow);
	const char*		GetGroupCaption(CCommand* pcCommand);
	int32			GetSelectedRow(void);
	void			SelectRow(size uiRow);
	bool			AddKeys(size uiRow, const char* szKeys);
	void			RemoveKeys(size uiRow);
	void			ResetKeys(size uiRow);
	void			ResetAllKeys(void);
	void			FilterChanged(void);
	void			RowActivated(void);
	void			AddClicked(CButton* pcButton);
	void			RemoveClicked(CButton* pcButton);
	void			ResetClicked(CButton* pcButton);
	void			ResetAllClicked(CButton* pcButton);
};


#endif // __KEYMAP_PAGE_H__

