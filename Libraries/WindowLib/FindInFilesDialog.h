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
#ifndef __FIND_IN_FILES_DIALOG_H__
#define __FIND_IN_FILES_DIALOG_H__
#include "BaseLib/ArrayTemplate.h"
#include "ListContainer.h"
#include "ScrollPane.h"
#include "TabbedContainer.h"
#include "Label.h"
#include "CodePreview.h"
#include "Dialog.h"


#define FIND_IN_FILES_DIALOG_ROWS	100		//The most results shown; IntelliJ shows as many and "100+ matches".


//A place the text was found: its line (as the searcher wants it shown, its leading spaces gone say) with where the
//text found is in that, where it is (its file's name and line), its file (a number of the searcher's, the same for
//every result in the file) and what it is to the searcher.
struct SFindInFilesItem
{
	CChars	szText;
	size	uiMatchStart;
	size	uiMatchLength;
	CChars	szLocation;
	size	uiFile;
	size	uiData;
};


typedef CArrayTemplate<SFindInFilesItem>	CArrayFindInFilesItems;


//The places a search found, in the order it found them: a file at a time, down each.
class CFindInFilesItems
{
public:
	CArrayFindInFilesItems	masItems;

	void				Init(void);
	void				Kill(void);
	void				Clear(void);

	SFindInFilesItem*	Add(const char* szText, size uiMatchStart, size uiMatchLength, const char* szLocation, size uiFile, size uiData);
	size				NumItems(void) const;
	SFindInFilesItem*	Get(size uiIndex);
	size				NumFiles(void);
};


//What a Find in Files dialog is looking for: the text typed, in which of its scopes (and the directory typed, for a
//scope that has one), how it must match (the text can be a regular expression), and which files of the scope are
//looked in (the file mask typed: "*.cpp, *.h", empty for all of them).  A dialog that replaces has what to replace
//with, NULL for one that only finds.
struct SFindInFilesQuery
{
	size		uiScope;
	const char*	szText;
	const char*	szDirectory;
	const char*	szFileMask;
	const char*	szReplace;
	bool		bMatchCase;
	bool		bWholeWords;
	bool		bRegex;
};


//Finds what a Find in Files dialog lists, a little at a time so the dialog stays quick to type in, and gives it the
//text it previews.
class CFindInFilesSearcher
{
public:
	//A new search; the one before is forgotten.  The query's strings are only good during the call.
	virtual void	FindStart(SFindInFilesQuery* psQuery) = 0;

	//Searches for a short time (a few milliseconds), adding what it finds to pcDest.  False once everything was searched.
	virtual bool	FindStep(CFindInFilesItems* pcDest) = 0;

	//Where a result is in its file's text: the preview's title (the file), and the found text's start and length.
	virtual bool	FindPreviewPlace(SFindInFilesItem* psItem, CChars* pszTitle, size* puiStart, size* puiLength) = 0;

	//The file's text, appended to pszText.  Only asked for when the preview's title changes.
	virtual bool	FindPreviewText(SFindInFilesItem* psItem, CChars* pszText) = 0;

	//The preview shows the result, with new text if bNewText: it can be coloured.
	virtual void	FindPreviewShown(SFindInFilesItem* psItem, Ptr<CTextArea> pPreview, bool bNewText);

	//Why the query as it was started can find nothing (its text isn't a regular expression, say), appended to
	//pszDest.  False if nothing is wrong with it.
	virtual bool	FindProblem(CChars* pszDest);

	//The button beside a directory scope's field was pressed: a directory chosen some way (a file chooser), starting
	//from szDirectory, appended to pszDest.  Called at a tick, so a dialog can be shown.  False if none was chosen.
	virtual bool	FindChooseDirectory(const char* szDirectory, CChars* pszDest);
};


//How a Find in Files dialog was closed.
enum EFindInFilesResult
{
	FIFR_Cancelled,
	FIFR_Chosen,		//A result was chosen (GetChosen), to be opened.
	FIFR_FindWindow,	//Open in Find Window was clicked: everything the query finds is wanted.
	FIFR_ReplaceAll,	//Replace All was clicked (a dialog that replaces): everything the query finds is to be replaced.
};


//IntelliJ IDEA's Find in Files: a tab for each scope searched along the top (a scope can have a directory to type),
//the text to find with how it must match (the case, whole words, as a regular expression) beside it, under that the
//file mask (which of the scope's files are looked in) and, in a dialog that replaces (SetReplacing), what to replace
//with, and under those a quick preview of what was
//found: each result's line on the left, the text found highlighted, and its file and line, dimmer, on the right.  A
//result's two parts are sized by what they hold: when both don't fit each gets its share of the row in proportion to
//its own width, so a short line doesn't crowd a long file name nor a long line a short one (each row is its own grid:
//they aren't the columns of a table).  Under the results the selected one's file is shown, as code, at the line it is
//on, the text found coloured (a CCodePreview).  Only the first FIND_IN_FILES_DIALOG_ROWS results are listed.
//
//The searcher finds the results a step at a time (see CFindInFilesSearcher) between the dialog's ticks, starting again
//with each character typed, so typing never waits for a search of every file.
//
//Up, down, page up and page down move through the results while typing.  Enter, or a double click, chooses the
//selected result and closes the dialog; escape closes it; Open in Find Window closes it for its owner to list every
//result, and a replacing dialog's Replace All for its owner to replace every one.  What the searcher says is wrong
//with the query (FindProblem) is shown where the count is.
//
//It is shown in its own window.  A Find in Files dialog is an object: make it with OMalloc and keep it in the root, or
//an object that is, while it's shown.
class CFindInFilesDialog : public CObject
{
CONSTRUCTABLE(CFindInFilesDialog);
DESTRUCTABLE(CFindInFilesDialog);
protected:
	Ptr<CDialog>			mpDialog;
	Ptr<CTabbedContainer>	mpTabs;
	Ptr<CGridContainer>		mpScopeRow;  //Under the tabs: the chosen scope's content.
	CArray<CComponent>		maScopeContents;  //Each scope's: what it searches, or its directory's field.
	CArray<CComponent>		maScopePages;  //Each scope's tab's page: a scope is known by it, wherever its tab is on the bar.
	Ptr<CTextField>			mpField;
	Ptr<CCheckBox>			mpMatchCase;
	Ptr<CCheckBox>			mpWholeWords;
	Ptr<CCheckBox>			mpRegex;
	Ptr<CTextField>			mpFileMask;
	Ptr<CTextField>			mpReplace;
	Ptr<CGridContainer>		mpReplaceRow;  //The replacement's label and field, shown in a dialog that replaces.
	Ptr<CGridContainer>		mpMaskRow;
	Ptr<CButton>			mpReplaceAll;
	Ptr<CGridContainer>		mpBottom;
	bool					mbReplacing;
	Ptr<CTextField>			mpDirectory;  //On the page of the scope that has one, NULL if none does.
	Ptr<CButton>			mpBrowse;  //Beside it: the searcher chooses the directory.
	Ptr<CScrollPane>		mpScrollPane;
	Ptr<CListContainer>		mpList;
	Ptr<CCodePreview>		mpPreview;
	Ptr<CLabel>				mpStatus;
	Ptr<CButton>			mpFindWindow;
	Ptr<CFont>				mpMatchFont;
	Ptr<CFontFamily>		mpCodeFamily;
	CFindInFilesSearcher*	mpcSearcher;
	CFindInFilesItems		mcItems;
	bool					mbSearching;  //The searcher has more to search.
	bool					mbMore;  //There may be more results than are listed.
	int32					miChosen;  //The result chosen, -1 until one is.
	EFindInFilesResult		meResult;

public:
	void			Init(Ptr<CWindow> pOwner, SDialogLook* psLook, const char* szTitle, CFindInFilesSearcher* pcSearcher, Ptr<CFont> pMatchFont, Ptr<CFontFamily> pCodeFamily);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	size				AddScope(const char* szName, const char* szDescription);
	size				AddDirectoryScope(const char* szName, const char* szDirectory);
	void				SetOptions(bool bMatchCase, bool bWholeWords, bool bRegex = false);
	void				SetFileMask(const char* szFileMask);
	void				SetReplacing(const char* szReplace);
	void				Start(size uiScope, const char* szText);
	EFindInFilesResult	Show(void);

	void				SetText(const char* szText);
	void				SetDirectory(const char* szDirectory);
	void				SelectScope(size uiScope);
	size				GetScope(void);
	const char*			GetText(void);
	const char*			GetDirectory(void);
	bool				IsMatchCase(void);
	bool				IsWholeWords(void);
	bool				IsRegex(void);
	const char*			GetFileMask(void);
	const char*			GetReplaceText(void);
	bool				IsReplacing(void) const;
	bool				IsSearching(void) const;
	bool				HasMore(void) const;
	size				NumResults(void) const;
	SFindInFilesItem*	GetResult(size uiIndex);
	int32				GetSelected(void);
	SFindInFilesItem*	GetChosen(void);
	EFindInFilesResult	GetOutcome(void) const;
	const char*			GetPreviewTitle(void);
	const char*			GetStatus(void);
	Ptr<CTextField>		GetField(void) const;
	Ptr<CTextField>		GetDirectoryField(void) const;
	Ptr<CCheckBox>		GetMatchCaseBox(void) const;
	Ptr<CCheckBox>		GetWholeWordsBox(void) const;
	Ptr<CCheckBox>		GetRegexBox(void) const;
	Ptr<CTextField>		GetFileMaskField(void) const;
	Ptr<CTextField>		GetReplaceField(void) const;
	Ptr<CButton>		GetReplaceAllButton(void) const;
	Ptr<CButton>		GetBrowseButton(void) const;
	Ptr<CListContainer>	GetList(void) const;
	Ptr<CCodeEditor>	GetPreview(void);
	Ptr<CCodePreview>	GetCodePreview(void) const;
	Ptr<CButton>		GetFindWindowButton(void) const;
	Ptr<CDialog>		GetDialog(void) const;

	void			Search(void);
	bool			Step(void);
	void			Finish(void);
	bool			Approve(void);
	void			ResultActivated(void);
	void			ResultSelected(void);
	void			ScopeSelected(void);
	void			OpenInFindWindow(void);
	void			ReplaceAll(void);
	void			BrowseClicked(void);
	void			ChooseDirectory(void);
	void			Up(void);
	void			Down(void);
	void			PageUp(void);
	void			PageDown(void);

protected:
	int32			GetMetric(EThemeMetric eMetric);
	Ptr<CComponent>	CreateBody(void);
	Ptr<CComponent>	CreateGap(void);
	size			AddScopePage(const char* szName, Ptr<CComponent> pContent);
	void			ShowScopeContent(void);
	void			AddRow(SFindInFilesItem* psItem);
	Ptr<CComponent>	CreateLine(SFindInFilesItem* psItem, int32* piWidth);
	void			ShowPreview(void);
	void			ClearPreview(void);
	void			ShowStatus(void);
	void			PostedStep(void);
};


#endif // __FIND_IN_FILES_DIALOG_H__

