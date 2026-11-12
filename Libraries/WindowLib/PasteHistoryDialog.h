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
#ifndef __PASTE_HISTORY_DIALOG_H__
#define __PASTE_HISTORY_DIALOG_H__
#include "BaseLib/ArrayTemplate.h"
#include "SupportLib/FontFamily.h"
#include "ClipboardHistory.h"
#include "ListContainer.h"
#include "ScrollPane.h"
#include "SplitContainer.h"
#include "TextArea.h"
#include "Label.h"
#include "Dialog.h"


#define PASTE_HISTORY_ROW_CHARACTERS	100		//The most of an entry's text a row shows.


typedef CArrayTemplate<size>	CArrayPasteHistoryRows;


//IntelliJ IDEA's Choose Content to Paste (Edit, Paste from History, Control+Shift+V): the clipboard history's entries
//in a list, newest first, each row the start of its text on one line (line breaks and tabs as spaces, "..." where it
//is cut), over a preview of what would be pasted: the selected entry's whole text, or with several rows selected
//their texts in the list's order, a line feed between them.
//
//The list has the keys.  Enter, OK or a double click chooses what is selected and closes the dialog; escape or Cancel
//closes it with nothing chosen.  Shift with the moves, or with a press, and Control with a press select several rows.
//Delete takes the selected entries out of the history.
//
//What is typed filters the list, as a speed search does: only the entries whose text has what was typed in it, case
//not mattering, are listed, and what was typed shows over the list.  Backspace takes a character off it and escape
//clears it; escape with nothing typed closes the dialog.  The first nine rows have their number before them, and
//with nothing typed 1 to 9 chooses that row at once (typed after something else, a digit is part of the filter).
//
//It is a CClipboardChooser: a history it is set on (CClipboardHistory::SetChooser) asks it which entry to paste.  It
//is shown in its own window over whichever window asks.  A paste history dialog is an object: make it with OMalloc
//and keep it in the root, or an object that is, while it's the chooser.
class CPasteHistoryDialog : public CObject, public CClipboardChooser
{
CONSTRUCTABLE(CPasteHistoryDialog);
DESTRUCTABLE(CPasteHistoryDialog);
protected:
	Ptr<CDialog>			mpDialog;
	Ptr<CLabel>				mpFilter;  //What was typed to filter the list, over it.
	Ptr<CSplitContainer>	mpSplit;
	Ptr<CScrollPane>		mpListPane;
	Ptr<CListContainer>		mpList;
	Ptr<CScrollPane>		mpPreviewPane;
	Ptr<CTextArea>			mpPreview;
	Ptr<CClipboardHistory>	mpHistory;  //What is being chosen from, NULL when nothing is.
	CArrayPasteHistoryRows	mauiRows;  //Each row's entry in the history.
	CChars					mszFilter;  //Only the entries with it in them are listed; empty for all.
	CChars					mszChosen;
	bool					mbChosen;

public:
	void			Init(Ptr<CWindow> pOwner, SDialogLook* psLook, Ptr<CFontFamily> pPreviewFamily);
	void			Init(Ptr<CWindow> pOwner, SDialogLook* psLook, Ptr<CFont> pPreviewFont);
	void			Class(void) override;
	void			Free(void) override;
	bool			Save(CObjectWriter* pcFile) override;
	bool			Load(CObjectReader* pcFile) override;

	bool			ChooseContent(Ptr<CWindow> pOwner, Ptr<CClipboardHistory> pHistory, CChars* pszDest) override;
	void			Start(Ptr<CClipboardHistory> pHistory);
	bool			Show(void);
	bool			IsChosen(void);
	char*			GetChosen(void);

	Ptr<CListContainer>	GetList(void);
	Ptr<CTextArea>		GetPreview(void);
	Ptr<CLabel>			GetFilterLabel(void);
	Ptr<CDialog>		GetDialog(void);
	size				GetRowEntry(int32 iRow);
	void				GetRowText(int32 iRow, CChars* pszDest);

	bool			Approve(void);
	void			RowActivated(void);
	void			SelectionChanged(void);
	void			DeleteSelected(void);
	void			Typed(SComponentInputEvent* psEvent);
	void			Backspace(void);
	void			Escape(void);
	void			SetFilter(const char* szFilter);
	char*			GetFilter(void);

protected:
	void			Init(Ptr<CWindow> pOwner, SDialogLook* psLook, Ptr<CFontFamily> pPreviewFamily, Ptr<CFont> pPreviewFont);
	int32			GetMetric(EThemeMetric eMetric);
	Ptr<CComponent>	CreateBody(Ptr<CFontFamily> pPreviewFamily, Ptr<CFont> pPreviewFont);
	void			FillList(int32 iSelect);
	void			AddRow(size uiEntry);
	bool			IsListed(size uiEntry);
	void			AppendRowText(CChars* pszDest, const char* szText);
	void			AppendSelected(CChars* pszDest);
};


#endif // __PASTE_HISTORY_DIALOG_H__

