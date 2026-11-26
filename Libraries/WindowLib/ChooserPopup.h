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
#ifndef __CHOOSER_POPUP_H__
#define __CHOOSER_POPUP_H__
#include "BaseLib/ArrayInt.h"
#include "BaseLib/NameMatcher.h"
#include "ComponentEvent.h"
#include "NameRow.h"
#include "ScrollPane.h"
#include "TextField.h"
#include "Button.h"
#include "CodePreview.h"
#include "Popup.h"


#define CHOOSER_HEADING		MAX_SIZE	//The data of an item that is a heading: its row is dim and can't be chosen.


//A popup that lists things to choose one of, as IntelliJ IDEA's choosers are (its usages popup, File Structure,
//Recent Files): rows drawn as a Go to dialog's results (AddNameRow: an icon, a name, where it is, dimmer), under an
//optional title and an optional field that filters them as it is typed in (the names the pattern matches, as Go to
//matches them, the matched characters in the match font), with an optional preview of the selected one's code beside
//them and an optional button under them.
//
//What it has is chosen before it is opened (SetTitle, AddFilter, AddPreview, AddButton), then its icons and items are
//added (AddIcon, AddItem, AddHeading) and it is opened at a point of the window (Open), no taller than its rows need
//or the theme's TM_ChooserRows of them: its top left there, or its bottom left for a chooser that opens above what
//it is for (SetOpensAbove), as one opened from the bottom of the window is.  With a filter the keys stay with the field: up, down, page up and page down
//move through the rows and enter chooses the selected one.  Without one the list has them, and typing finds a row
//(CListContainer::SetTypeToFind).  A click chooses too.
//Escape, or a press outside it, closes it.  Filtered items keep the order they were added in unless they are sorted
//best match first (SetSortMatches).
//
//A driven chooser (SetDriven) is one something else has the keys for, as an editor that goes on being typed in has
//for the list of what could be written at its caret: it has no title and no field, it doesn't take the keys when it
//opens, and whoever opened it gives it what to filter by (SetFilterText), moves its selection (Up, Down, PageUp,
//PageDown) and chooses (ChooseSelected).  A press on a row still chooses it.  Items can be added to it while it is
//open (AddItem, then ItemsChanged): the selected item stays selected and it grows to its rows.  A note (SetNote) is a
//dim line under its rows, as "Working..." while more are found.
//
//Its events: chosen (GetChosen is the item, the popup is closed already), selected (the selected item changed: fill
//the preview from GetSelectedItem), the button's click (the popup is closed already) and closed, however it went.
//
//It is an object: make it with OMalloc and keep it in the root, or an object that is, while it is open.
class CChooserPopup : public CObject
{
CONSTRUCTABLE(CChooserPopup);
DESTRUCTABLE(CChooserPopup);
protected:
	Ptr<CWindow>			mpWindow;
	SDialogLook*			mpsLook;  //Not held: whoever made the chooser keeps it.
	Ptr<CFont>				mpMatchFont;
	Ptr<CGridContainer>		mpPanel;  //What the popup holds.
	Ptr<CLabel>				mpTitle;  //NULL for a driven chooser.
	Ptr<CLabel>				mpNote;  //A driven chooser's, under its rows.
	Ptr<CTextField>			mpField;  //NULL without a filter.
	Ptr<CScrollPane>		mpScrollPane;
	Ptr<CListContainer>		mpList;
	Ptr<CCodePreview>		mpPreview;  //NULL without one.
	Ptr<CButton>			mpButton;  //NULL without one.
	Ptr<CPopup>				mpPopup;  //While it is open.
	Ptr<CComponent>			mpPreviousFocus;  //What had the keys when it was opened, which gets them back.
	CArray<CImageCel>		maIcons;
	CChars					mszTitle;
	CGoToItems				mcItems;  //Every item, in the order added.
	CArrayInt				maiRows;  //The item each row shows.
	CNameMatcher			mcMatcher;
	bool					mbSortMatches;
	bool					mbOpensAbove;  //Open's position is where its bottom left goes.
	bool					mbDriven;  //The keys stay with what opened it.
	CChars					mszFilter;  //A driven chooser's pattern, in place of a field's text.
	CChars					mszNote;
	SInt32Vec2				msOpenPosition;  //Where it was opened, for when its size changes.
	int32					miChosen;  //The item chosen, -1 until one is.

	CComponentEvent<CChooserPopup>	mcChosen;
	CComponentEvent<CChooserPopup>	mcSelected;
	CComponentEvent<CChooserPopup>	mcButtonClicked;
	CComponentEvent<CChooserPopup>	mcClosed;

public:
	void			Init(Ptr<CWindow> pWindow, SDialogLook* psLook, Ptr<CFont> pMatchFont);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	void			SetTitle(const char* szTitle);
	void			AddFilter(void);
	void			AddPreview(Ptr<CFont> pTitleFont, Ptr<CFontFamily> pCodeFamily);
	void			AddButton(const char* szCaption);
	void			SetSortMatches(bool bSortMatches);
	void			SetOpensAbove(bool bOpensAbove);
	void			SetDriven(bool bDriven);
	bool			IsDriven(void) const;
	void			SetNote(const char* szNote);
	const char*		GetNote(void) const;

	int32			AddIcon(Ptr<CImageCel> pIcon);
	SGoToItem*		AddItem(const char* szName, const char* szLocation, int32 iIcon, size uiData);
	SGoToItem*		AddHeading(const char* szName);
	void			RemoveAllItems(void);
	void			ItemsChanged(void);
	size			NumItems(void) const;
	SGoToItem*		GetItem(size uiIndex);

	void			Open(SInt32Vec2 sPosition, Ptr<CComponent> pOwner, size uiSelectedItem = 0);
	void			Close(void);
	bool			IsOpen(void) const;

	void			SetFilterText(const char* szText);
	const char*		GetFilterText(void);
	size			NumRows(void) const;
	SGoToItem*		GetRowItem(size uiRow);
	bool			SelectRow(int32 iRow);
	int32			GetSelectedRow(void);
	SGoToItem*		GetSelectedItem(void);
	SGoToItem*		GetChosen(void);
	bool			ChooseSelected(void);
	void			Up(void);
	void			Down(void);
	void			PageUp(void);
	void			PageDown(void);

	Ptr<CComponent>		GetPanel(void) const;
	Ptr<CTextField>		GetField(void) const;
	Ptr<CListContainer>	GetList(void) const;
	Ptr<CCodePreview>	GetPreview(void) const;
	Ptr<CButton>		GetButton(void) const;
	Ptr<CPopup>			GetPopup(void) const;
	const char*			GetTitle(void) const;

	CComponentEvent<CChooserPopup>*	GetChosenEvent(void);
	CComponentEvent<CChooserPopup>*	GetSelectedEvent(void);
	CComponentEvent<CChooserPopup>*	GetButtonClickEvent(void);
	CComponentEvent<CChooserPopup>*	GetClosedEvent(void);

protected:
	void			CreatePanel(void);
	void			Filter(void);
	void			FillList(void);
	int32			FindItemRow(size uiItem);
	int32			FindFirstRow(void);
	void			RowSelected(void);
	void			RowActivated(void);
	void			ButtonClicked(void);
	void			PopupClosed(void);
	void			Enter(void);
	const char*		GetPattern(void);
	void			Resize(void);
};


#endif // __CHOOSER_POPUP_H__

