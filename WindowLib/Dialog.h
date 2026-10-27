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
#ifndef __DIALOG_H__
#define __DIALOG_H__
#include "SupportLib/Font.h"
#include "Window.h"
#include "WindowTick.h"
#include "GridContainer.h"
#include "ButtonBar.h"
#include "Button.h"
#include "TextField.h"
#include "CheckBox.h"
#include "ComboBox.h"
#include "WindowTheme.h"


//What every dialog is drawn with, made once by its owner.
struct SDialogLook
{
	Ptr<CWindowTheme>		pTheme;  //The widgets' look and the labels' font.
	Ptr<CFont>				pDimFont;  //For a divider's title.
	ARGB32					sBackground;
	ARGB32					sDivider;

	void Kill(void);
};


//Paints a dialog each tick.
class CDialogTick : public CWindowTick
{
CONSTRUCTABLE(CDialogTick);
DESTRUCTABLE(CDialogTick);
public:
	void	Init(void);
	void	Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override;
};


//Asked whether OK may close a dialog, as a file chooser opens the folder typed in it instead.
class CDialogApprover
{
public:
	virtual bool	Approve(void) = 0;
};


//A form in its own window, shown modally over its owner's window, in IntelliJ IDEA's style: a label and an editor on
//each row, dividers with a title between groups of rows, and the buttons along the bottom on the right.  OK is the
//default button, enter clicks it; escape clicks Cancel.
//
//A dialog that isn't a form of rows sets a body instead, which fills the dialog above the buttons, and is shown at a
//size of its own.
//
//OK can have another caption, like a file chooser's Open, and an approver, but it still closes the dialog as OK.
//
//The dialog's window is in the root while the dialog exists.
class CDialog : public CButtonListener
{
CONSTRUCTABLE(CDialog);
protected:
	SDialogLook*		mpsLook;
	Ptr<CWindow>			mpOwner;
	Ptr<CWindow>			mpDialog;
	Ptr<CGridContainer>		mpForm;
	Ptr<CButtonBar>			mpButtons;
	Ptr<CButton>			mpOK;
	Ptr<CButton>			mpCancel;
	Ptr<CButton>			mpExtra;  //Like Save Defaults, NULL if there isn't one.
	Ptr<CComponent>			mpFirstEditor;  //Has focus when the dialog opens.
	CDialogApprover*	mpcApprover;  //NULL if OK always closes the dialog.
	CChars					mszClicked;
	int						miRows;

public:
	void				Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle);
	void				Kill(void);

	Ptr<CTextField>		AddTextField(const char* szLabel, const char* szText);
	Ptr<CCheckBox>		AddCheckBox(const char* szLabel, bool bTicked);
	Ptr<CComboBox>		AddComboBox(const char* szLabel, const char** aszItems, int iNumItems, int iSelected);
	void				AddEditor(const char* szLabel, Ptr<CComponent> pEditor);
	void				AddDivider(const char* szTitle);
	void				AddExtraButton(const char* szCaption);
	void				SetBody(Ptr<CComponent> pBody, Ptr<CComponent> pFirstEditor);
	void				SetOKCaption(const char* szCaption);
	void				SetApprover(CDialogApprover* pcApprover);

	bool				Show(int32 iWidth);
	bool				Show(int32 iWidth, int32 iHeight);
	void				Close(const char* szClicked);
	char*				GetClicked(void);
	Ptr<CWindow>		GetWindow(void);
	SDialogLook*	GetLook(void);

	void				ButtonClicked(CUnknown* pcSource, void* pvContext) override;

protected:
	void				AddRow(const char* szLabel, Ptr<CComponent> pEditor);
};


#endif // __DIALOG_H__

