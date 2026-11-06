/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __TEXT_UNDO_H__
#define __TEXT_UNDO_H__
#include "UndoStack.h"


#define TEXT_UNDO_TYPING_PAUSE	1000	//Milliseconds.


enum ETextUndoKeep
{
	TUK_None,		//Nothing was kept: an undo or redo is loading.
	TUK_Started,	//The history starts with the text kept; what it was before isn't known.
	TUK_Pushed,		//The text kept is a new state.
	TUK_Merged		//Typing went on the state of the typing before it.
};


//The undo history of a text being edited.  The text after each edit is kept (Keep) with the caret and selection from
//before the edit (BeginEdit, or as they are when it is kept) and after it.  Typing goes on the state of the typing
//before it, as one edit, while each character is typed (Typed) where the last left the caret, without a pause
//(SetTypingPause), up to the space after a word or a new line.  Loading a state to undo or redo to isn't an edit: its
//text is got (GetLoadText), put in the editor's text, and the history moved to it (Loading) before the editor does
//what it does with a changed text, then Loaded.  CTextArea and CTextField keep their text's history in one.
class CTextUndo
{
protected:
	CUndoStack	mcStack;
	bool		mbEditStarted;  //BeginEdit noted the caret and selection before the edit being made.
	size		muiEditCaret;
	size		muiEditAnchor;
	bool		mbTyping;  //The edit being made is a character typed.
	bool		mbLastTyping;  //The last edit kept was.
	char		mcTyped;
	char		mcLastTyped;
	size		muiTypedAt;  //Where the character was typed, MAX_SIZE over a selection.
	size		muiTypedEnd;  //The caret after the last edit kept.
	int64		miTypedTime;  //When the last character typed was kept.
	int32		miTypingPause;  //Typing that stops this long (milliseconds) starts a new edit.
	bool		mbLoading;

public:
	void			Init(void);
	void			Kill(void);

	void			BeginEdit(size uiCaret, size uiAnchor);
	bool			IsEditStarted(void);
	void			Typed(char c, size uiAt);
	ETextUndoKeep	Keep(char* szText, size uiCaret, size uiAnchor);
	void			DropEdit(void);
	void			Clear(char* szText, size uiCaret, size uiAnchor);
	void			SetTypingPause(int32 iMilliseconds);

	size			FindUndo(void);
	size			FindRedo(void);
	size			FindId(size uiId);
	size			GetCurrentId(void);
	bool			CanLoad(size uiIndex);
	bool			IsUndo(size uiIndex);
	char*			GetLoadText(size uiIndex, size* puiCaret, size* puiAnchor);
	void			Loading(size uiIndex);
	void			Loaded(size uiCaret);

	void			FindDifference(char* szText, size uiLength, char* szTarget, size uiTargetLength, size* puiPrefix, size* puiSuffix);

protected:
	bool			MergesTyping(int64 iNow);
};


#endif // __TEXT_UNDO_H__

