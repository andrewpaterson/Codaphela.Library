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
#include "Timer.h"
#include "ArrayElementNotFound.h"
#include "StringHelper.h"
#include "TextUndo.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextUndo::Init(void)
{
	mcStack.Init();
	mbEditStarted = false;
	muiEditCaret = 0;
	muiEditAnchor = MAX_SIZE;
	mbTyping = false;
	mbLastTyping = false;
	mcTyped = '\0';
	mcLastTyped = '\0';
	muiTypedAt = 0;
	muiTypedEnd = 0;
	miTypedTime = 0;
	miTypingPause = TEXT_UNDO_TYPING_PAUSE;
	mbLoading = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextUndo::Kill(void)
{
	mcStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// An edit begins (typing, deleting, pasting ...): the caret and selection
// are noted as they are, before it moves them or changes the text, for
// undo to put back.  Not while an undo or redo is loading.
//
//////////////////////////////////////////////////////////////////////////
void CTextUndo::BeginEdit(size uiCaret, size uiAnchor)
{
	if (!mbLoading)
	{
		mbEditStarted = true;
		muiEditCaret = uiCaret;
		muiEditAnchor = uiAnchor;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The edit being made is c typed at uiAt (MAX_SIZE when it replaces a
// selection).
//
//////////////////////////////////////////////////////////////////////////
void CTextUndo::Typed(char c, size uiAt)
{
	mbTyping = true;
	mcTyped = c;
	muiTypedAt = uiAt;
}


//////////////////////////////////////////////////////////////////////////
//
// The text after an edit, with the caret and selection after it, is kept:
// a new state or, typing on, the state of the typing before it.  The edit
// is then over.
//
//////////////////////////////////////////////////////////////////////////
ETextUndoKeep CTextUndo::Keep(char* szText, size uiCaret, size uiAnchor)
{
	ETextUndoKeep	eKeep;
	int64			iNow;
	size			auiMarks[UNDO_STACK_MARKS];

	if (mbLoading)
	{
		DropEdit();
		return TUK_None;
	}
	if (!mbEditStarted)
	{
		muiEditCaret = uiCaret;
		muiEditAnchor = uiAnchor;
	}
	auiMarks[0] = muiEditCaret;
	auiMarks[1] = muiEditAnchor;
	auiMarks[2] = uiCaret;
	auiMarks[3] = uiAnchor;
	iNow = GetDebugTickCount();

	if (mcStack.NumSnapshots() == 0)
	{
		mcStack.Push(szText, false, auiMarks);
		eKeep = TUK_Started;
	}
	else if (MergesTyping(iNow))
	{
		//The merged edit's caret before is the first typing's.
		auiMarks[0] = mcStack.GetMarks(mcStack.GetCurrent())[0];
		auiMarks[1] = mcStack.GetMarks(mcStack.GetCurrent())[1];
		mcStack.ReplaceCurrent(szText, auiMarks);
		eKeep = TUK_Merged;
	}
	else
	{
		mcStack.Push(szText, false, auiMarks);
		eKeep = TUK_Pushed;
	}

	mbLastTyping = mbTyping;
	mcLastTyped = mcTyped;
	muiTypedEnd = uiCaret;
	if (mbTyping)
	{
		miTypedTime = iNow;
	}
	mbTyping = false;
	mbEditStarted = false;
	return eKeep;
}


//////////////////////////////////////////////////////////////////////////
//
// Typing goes on the state before it if that is the newest state and the
// typing before's (not the state the history starts with), the character
// is typed where the last left the caret, soon enough, and it isn't a new
// line or the space after a word.
//
//////////////////////////////////////////////////////////////////////////
bool CTextUndo::MergesTyping(int64 iNow)
{
	size	uiCurrent;

	uiCurrent = mcStack.GetCurrent();
	if ((uiCurrent == 0) || (uiCurrent != mcStack.NumSnapshots() - 1))
	{
		return false;
	}
	if (!mbTyping || !mbLastTyping || (muiTypedAt != muiTypedEnd) || ((iNow - miTypedTime) >= miTypingPause))
	{
		return false;
	}
	if ((mcTyped == '\n') || (mcTyped == '\r'))
	{
		return false;
	}
	return !((mcTyped == ' ') && (mcLastTyped != ' '));
}


//////////////////////////////////////////////////////////////////////////
//
// The edit being made isn't kept (the text didn't change, or can't).
//
//////////////////////////////////////////////////////////////////////////
void CTextUndo::DropEdit(void)
{
	mbTyping = false;
	mbEditStarted = false;
}


//////////////////////////////////////////////////////////////////////////
//
// The history is forgotten.  It starts again with szText, with the caret
// and selection it has, or is empty if szText is NULL.
//
//////////////////////////////////////////////////////////////////////////
void CTextUndo::Clear(char* szText, size uiCaret, size uiAnchor)
{
	mcStack.Clear();
	mbLastTyping = false;
	DropEdit();
	if (szText)
	{
		Keep(szText, uiCaret, uiAnchor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The state the text would go back to (FindUndo) or forward to
// (FindRedo), ARRAY_ELEMENT_NOT_FOUND if there is none.
//
//////////////////////////////////////////////////////////////////////////
size CTextUndo::FindUndo(void) { return mcStack.FindUndo(false); }
size CTextUndo::FindRedo(void) { return mcStack.FindRedo(false); }
size CTextUndo::FindId(size uiId) { return mcStack.FindId(uiId); }
size CTextUndo::GetCurrentId(void) { return mcStack.GetCurrentId(); }
void CTextUndo::SetTypingPause(int32 iMilliseconds) { miTypingPause = iMilliseconds; }
bool CTextUndo::IsEditStarted(void) { return mbEditStarted; }


//////////////////////////////////////////////////////////////////////////
//
// Whether state uiIndex can be loaded: it is in the history and isn't
// the current state.
//
//////////////////////////////////////////////////////////////////////////
bool CTextUndo::CanLoad(size uiIndex)
{
	return (uiIndex != ARRAY_ELEMENT_NOT_FOUND) && (uiIndex < mcStack.NumSnapshots()) && (uiIndex != mcStack.GetCurrent());
}


//////////////////////////////////////////////////////////////////////////
//
// Loading state uiIndex undoes (it is before the current state) rather
// than redoes.
//
//////////////////////////////////////////////////////////////////////////
bool CTextUndo::IsUndo(size uiIndex)
{
	return uiIndex < mcStack.GetCurrent();
}


//////////////////////////////////////////////////////////////////////////
//
// State uiIndex's text, and where the caret and selection go: as they
// were before the first edit undone, or after the last edit redone, kept
// in the text.  The selection is MAX_SIZE if there is none.
//
//////////////////////////////////////////////////////////////////////////
char* CTextUndo::GetLoadText(size uiIndex, size* puiCaret, size* puiAnchor)
{
	char*	szText;
	size*	pauiMarks;
	size	uiLength;

	szText = mcStack.Get(uiIndex);
	uiLength = StrLen(szText);
	if (IsUndo(uiIndex))
	{
		pauiMarks = mcStack.GetMarks(uiIndex + 1);
		*puiCaret = pauiMarks[0];
		*puiAnchor = pauiMarks[1];
	}
	else
	{
		pauiMarks = mcStack.GetMarks(uiIndex);
		*puiCaret = pauiMarks[2];
		*puiAnchor = pauiMarks[3];
	}

	if (*puiCaret > uiLength)
	{
		*puiCaret = uiLength;
	}
	if ((*puiAnchor != MAX_SIZE) && ((*puiAnchor > uiLength) || (*puiAnchor == *puiCaret)))
	{
		*puiAnchor = MAX_SIZE;
	}
	return szText;
}


//////////////////////////////////////////////////////////////////////////
//
// State uiIndex's text is the editor's: the history is at it, and edits
// aren't kept until Loaded.
//
//////////////////////////////////////////////////////////////////////////
void CTextUndo::Loading(size uiIndex)
{
	mcStack.MoveTo(uiIndex);
	mbLoading = true;
}


//////////////////////////////////////////////////////////////////////////
//
// The load is done, the caret at uiCaret.  Typing after it starts a new
// edit.
//
//////////////////////////////////////////////////////////////////////////
void CTextUndo::Loaded(size uiCaret)
{
	mbLoading = false;
	mbLastTyping = false;
	muiTypedEnd = uiCaret;
}


//////////////////////////////////////////////////////////////////////////
//
// How much of szText's start (*puiPrefix) and end (*puiSuffix) szTarget
// has too, so only what is between changes.  The difference starts and
// ends on whole UTF-8 characters.
//
//////////////////////////////////////////////////////////////////////////
void CTextUndo::FindDifference(char* szText, size uiLength, char* szTarget, size uiTargetLength, size* puiPrefix, size* puiSuffix)
{
	size	uiPrefix;
	size	uiSuffix;

	uiPrefix = 0;
	while ((uiPrefix < uiLength) && (uiPrefix < uiTargetLength) && (szText[uiPrefix] == szTarget[uiPrefix]))
	{
		uiPrefix++;
	}
	uiSuffix = 0;
	while ((uiSuffix < uiLength - uiPrefix) && (uiSuffix < uiTargetLength - uiPrefix) && (szText[uiLength - 1 - uiSuffix] == szTarget[uiTargetLength - 1 - uiSuffix]))
	{
		uiSuffix++;
	}

	while ((uiPrefix > 0) && ((szTarget[uiPrefix] & 0xC0) == 0x80))
	{
		uiPrefix--;
	}
	while ((uiSuffix > 0) && ((szTarget[uiTargetLength - uiSuffix] & 0xC0) == 0x80))
	{
		uiSuffix--;
	}
	*puiPrefix = uiPrefix;
	*puiSuffix = uiSuffix;
}

