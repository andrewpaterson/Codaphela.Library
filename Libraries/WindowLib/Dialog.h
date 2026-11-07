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
#include "StandardLib/MemberCall.h"
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


//The width of a dialog's buttons, unless a caption needs more.
#define DIALOG_BUTTON_WIDTH		72

//The narrowest a dialog sized to what it holds is (CDialog::Show).
#define DIALOG_MIN_WIDTH		360


//What every dialog is drawn with, made once by its owner.
struct SDialogLook
{
	CWindowTheme*			pcTheme;  //The widgets' look and the labels' font.  Whoever fills the look in holds it.
	CFont*					pcDimFont;  //For a divider's title.
	ARGB32					sBackground;
	ARGB32					sDivider;

	void Kill(void);
};


//Ticks a dialog, calling its ticker if it has one: a member function called outside of the dialog's input, so it can
//do what can't be done from an input action, like show another dialog over it (see CWindow::ShowModal).
class CDialogTick : public CWindowTick
{
CONSTRUCTABLE(CDialogTick);
DESTRUCTABLE(CDialogTick);
protected:
	SMemberCall		msTicker;  //Not set if there is none.

public:
	void	Init(void);
	void	Class(void) override;
	void	Tick(Ptr<CWindow> pWindow, int64 iUpdateTimeInMillieconds, int64 iTotalTimeInMillieconds) override;
	template<class M, class O>
	void	SetTicker(O* pcObject, void(M::*fTick)(void));
	void	ClearTicker(void);
};


//A form in its own window, shown modally over its owner's window, in IntelliJ IDEA's style: a label and an editor on
//each row, dividers with a title between groups of rows, and the buttons along the bottom on the right.  OK is the
//default button, enter clicks it; escape clicks Cancel.
//
//A dialog that isn't a form of rows sets a body instead, which fills the dialog above the buttons, and is shown at a
//size of its own.
//
//OK can have another caption, like a file chooser's Open, and an approver, but it still closes the dialog as OK.
//Cancel can have another caption or none, and an extra button can go before OK; they close the dialog as their
//captions.
//
//A dialog is an object: make it with OMalloc and keep it in the root, or in an object that is, while it's shown.  It
//holds its window.
class CDialog : public CObject
{
CONSTRUCTABLE(CDialog);
DESTRUCTABLE(CDialog);
protected:
	SDialogLook*			mpsLook;
	Ptr<CWindow>			mpOwner;
	Ptr<CWindow>			mpDialog;
	Ptr<CDialogTick>		mpTick;
	Ptr<CGridContainer>		mpForm;
	Ptr<CButtonBar>			mpButtons;
	Ptr<CButton>			mpOK;
	Ptr<CButton>			mpCancel;
	Ptr<CButton>			mpExtra;  //Like Save Defaults, NULL if there isn't one.
	bool					mbCancel;  //Cancel is shown (SetCancelCaption).
	Ptr<CComponent>			mpFirstEditor;  //Has focus when the dialog opens.
	SMemberCall				msApprover;  //Asked whether OK may close the dialog, not set if OK always closes it.
	CChars					mszClicked;
	int32					miRows;

public:
	void				Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle);
	void				Class(void) override;
	void				Free(void) override;

	bool				Save(CObjectWriter* pcFile) override;
	bool				Load(CObjectReader* pcFile) override;

	Ptr<CTextField>		AddTextField(const char* szLabel, const char* szText);
	Ptr<CCheckBox>		AddCheckBox(const char* szLabel, bool bTicked);
	Ptr<CComboBox>		AddComboBox(const char* szLabel, const char** aszItems, size iNumItems, int iSelected);
	void				AddEditor(const char* szLabel, Ptr<CComponent> pEditor);
	void				AddDivider(const char* szTitle);
	void				AddExtraButton(const char* szCaption);
	void				SetBody(Ptr<CComponent> pBody, Ptr<CComponent> pFirstEditor);
	void				SetOKCaption(const char* szCaption);
	void				SetCancelCaption(const char* szCaption);
	void				RemoveButtons(void);
	void				SetResizable(int32 iMinimumWidth, int32 iMinimumHeight);
	template<class M, class O>
	void				SetApprover(O* pcObject, bool(M::*fApprove)(void));
	void				ClearApprover(void);
	template<class M, class O>
	void				SetTicker(O* pcObject, void(M::*fTick)(void));
	void				ClearTicker(void);

	bool				Show(void);
	bool				Show(int32 iWidth);
	bool				Show(int32 iWidth, int32 iHeight);
	void				Close(const char* szClicked);
	char*				GetClicked(void);
	Ptr<CWindow>		GetWindow(void);
	SDialogLook*		GetLook(void);

	void				OKClicked(void);
	void				CloseClicked(CButton* pcButton);

protected:
	void				AddRow(const char* szLabel, Ptr<CComponent> pEditor);
	void				RemoveFromButton(Ptr<CButton> pButton);
	void				ArrangeButtons(void);
	void				SizeButton(Ptr<CButton> pButton);
};


//////////////////////////////////////////////////////////////////////////
//
// fTick is called each tick.  The object isn't held; it calls ClearTicker
// before it goes.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CDialogTick::SetTicker(O* pcObject, void(M::*fTick)(void))
{
	msTicker.Init(pcObject, fTick);
}


//////////////////////////////////////////////////////////////////////////
//
// OK, or enter, only closes the dialog if fApprove answers true, as a file
// chooser opens the folder typed in it instead.  The object isn't held;
// it calls ClearApprover before it goes.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CDialog::SetApprover(O* pcObject, bool(M::*fApprove)(void))
{
	msApprover.Init(pcObject, fApprove);
}


//////////////////////////////////////////////////////////////////////////
//
// fTick is called each tick while the dialog is shown, outside its input
// (see CDialogTick).  The object isn't held; it calls ClearTicker before
// it goes.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void CDialog::SetTicker(O* pcObject, void(M::*fTick)(void))
{
	mpTick->SetTicker(pcObject, fTick);
}


#endif // __DIALOG_H__

