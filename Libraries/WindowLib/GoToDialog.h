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
#ifndef __GO_TO_DIALOG_H__
#define __GO_TO_DIALOG_H__
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/NameMatcher.h"
#include "ListContainer.h"
#include "ScrollPane.h"
#include "TabbedContainer.h"
#include "Label.h"
#include "IconComponent.h"
#include "Dialog.h"
#include "CommandSet.h"


#define GO_TO_DIALOG_ROWS		100		//The most results shown; IntelliJ shows a page and "more".


//A result: the name the pattern matched, where it is (a class's file, a file's folder), the dialog's icon for it (see
//CGoToDialog::AddIcon, -1 for none) and what it is to whoever found it.
struct SGoToItem
{
	CChars		szName;
	CChars		szLocation;
	int32		iIcon;
	size		uiData;
	SNameMatch	sMatch;
};


typedef CArrayTemplate<SGoToItem>	CArrayGoToItems;


//The results a search found, best first once sorted.
class CGoToItems
{
public:
	CArrayGoToItems		masItems;

	void		Init(void);
	void		Kill(void);
	void		Clear(void);

	SGoToItem*	Add(const char* szName, const char* szLocation, int32 iIcon, size uiData, SNameMatch* psMatch);
	size		NumItems(void);
	SGoToItem*	Get(size uiIndex);
	void		Sort(void);
};


//Finds what a tab of a Go to dialog lists: every item the matcher matches, with its match.
class CGoToSearcher
{
public:
	virtual void	GoToSearch(size uiTab, CNameMatcher* pcMatcher, CGoToItems* pcDest) = 0;
};


//IntelliJ IDEA's Go to Class, Go to File and Go to Symbol: a tab for each kind of thing along the top, the text typed
//to find it, and what it matches best first under it, each with its icon, its name with the matched characters
//highlighted, and where it is, dimmer, after it.  Only the best GO_TO_DIALOG_ROWS are listed.  The searcher finds
//them, as each character is typed, for the tab chosen.
//
//Up, down, page up and page down move through the results while typing; tab and shift tab go to the next and previous
//tab, and the keys of the application's command that opens the dialog on a tab (AddTab) go to that tab, as IntelliJ's do.
//A tab chosen searches for the text already typed.  Enter, or a double click, chooses the selected result and closes the dialog; escape closes it.  A pattern
//ending in a colon and a number, "TextArea.cpp:120", finds what the rest does and chooses the line too.
//
//It is shown in its own window, without buttons.  A Go to dialog is an object: make it with OMalloc and keep it in the
//root, or an object that is, while it's shown.
class CGoToDialog : public CObject
{
CONSTRUCTABLE(CGoToDialog);
DESTRUCTABLE(CGoToDialog);
protected:
	Ptr<CDialog>			mpDialog;
	Ptr<CTabbedContainer>	mpTabs;
	Ptr<CTextField>			mpField;
	Ptr<CScrollPane>		mpScrollPane;
	Ptr<CListContainer>		mpList;
	Ptr<CFont>				mpMatchFont;
	Ptr<CCommandSet>		mpCommands;  //The dialog window's: a command for each tab that has keys, named by its index.
	CArray<CImageCel>		maIcons;
	CGoToSearcher*			mpcSearcher;
	CGoToItems				mcItems;
	CNameMatcher			mcMatcher;
	int32					miChosen;  //The result chosen, -1 until one is.
	int32					miLine;  //The line typed after the pattern (from 1), -1 if none was.

public:
	void			Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, CGoToSearcher* pcSearcher, Ptr<CFont> pMatchFont);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	size			AddTab(const char* szName, Ptr<CCommand> pOpener = NULL);
	int32			AddIcon(Ptr<CImageCel> pIcon);
	void			Start(size uiTab, const char* szText);
	bool			Show(void);

	void				SetText(const char* szText);
	void				SelectTab(size uiTab);
	size				GetTab(void);
	size				NumResults(void);
	SGoToItem*			GetResult(size uiIndex);
	int32				GetSelected(void);
	SGoToItem*			GetChosen(void);
	int32				GetLine(void);
	char*				GetPattern(void);
	Ptr<CTextField>		GetField(void);
	Ptr<CListContainer>	GetList(void);
	Ptr<CDialog>		GetDialog(void);

	void			Search(void);
	bool			Approve(void);
	void			ResultActivated(void);
	void			TabSelected(void);
	void			TabCommand(CCommand* pcCommand);
	void			Up(void);
	void			Down(void);
	void			PageUp(void);
	void			PageDown(void);
	void			NextTab(void);
	void			PreviousTab(void);

protected:
	int32			GetMetric(EThemeMetric eMetric);
	Ptr<CComponent>	CreateBody(void);
	void			FillList(void);
	void			AddRow(SGoToItem* psItem);
	Ptr<CComponent>	CreateName(SGoToItem* psItem);
	void			SplitLine(CChars* pszPattern);
};


#endif // __GO_TO_DIALOG_H__

