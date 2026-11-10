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
#include "ArrayElementNotFound.h"
#include "StringHelper.h"
#include "TextDocument.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size STextDocumentChange::Length(void) { return uiEnd - uiStart; }
size STextDocumentChange::LastLine(void) { return uiLine + uiLines; }  //The line uiEnd is on, for text taken out.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::Init(const char* szText)
{
	mszText.Init(szText == NULL ? "" : szText);
	mauiLineStarts.Init();
	FindLineStarts(0);
	mcNavigation.Init(&mszText);
	mcUndo.Init();
	mbUndoable = true;
	muiChangeCount = 0;
	mapcListeners.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::Kill(void)
{
	mapcListeners.Kill();
	mcUndo.Kill();
	mauiLineStarts.Kill();
	mszText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::AddListener(CTextDocumentListener* pcListener)
{
	mapcListeners.Add(pcListener);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::RemoveListener(CTextDocumentListener* pcListener)
{
	size	uiIndex;

	uiIndex = mapcListeners.Find(pcListener);
	if (uiIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		mapcListeners.RemoveAt(uiIndex, true);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Not an edit: the listeners aren't told and the history is kept (see
// ClearUndo).
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::SetText(const char* szText)
{
	mszText.Kill();
	mszText.Init(szText == NULL ? "" : szText);
	FindLineStarts(0);
}


//////////////////////////////////////////////////////////////////////////
//
// Not an edit: what is anchored to the text stays where it is, even at
// its end, and nothing is kept in the history.
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::Append(const char* szText)
{
	size	uiStart;

	if (szText == NULL)
	{
		return;
	}
	uiStart = mszText.Length();
	mszText.Append(szText);
	FindLineStarts(uiStart);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::Insert(size uiPosition, const char* szText)
{
	STextDocumentChange	sChange;
	size				uiLength;
	size				uiLine;
	size				i;

	uiLength = StrLen(szText);
	if (uiLength == 0)
	{
		return;
	}

	uiLine = FindLine(uiPosition);
	sChange.eChange = TDC_Inserted;
	sChange.uiStart = uiPosition;
	sChange.uiEnd = uiPosition + uiLength;
	sChange.uiLine = uiLine;
	sChange.uiLines = 0;
	sChange.bLineStart = (*mauiLineStarts.Get(uiLine) == uiPosition);

	for (i = uiLine + 1; i < mauiLineStarts.NumElements(); i++)
	{
		*mauiLineStarts.Get(i) += uiLength;
	}
	for (i = 0; i < uiLength; i++)
	{
		if (szText[i] == '\n')
		{
			sChange.uiLines++;
			*mauiLineStarts.InsertAt(uiLine + sChange.uiLines) = (size)(uiPosition + i + 1);
		}
	}
	mszText.Insert(uiPosition, szText);

	Changed(&sChange);
}


//////////////////////////////////////////////////////////////////////////
//
// Takes out bytes [uiStart, uiEnd).
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::Remove(size uiStart, size uiEnd)
{
	STextDocumentChange	sChange;
	size				uiLast;
	size				uiLength;
	size				i;

	if (uiEnd <= uiStart)
	{
		return;
	}

	uiLength = uiEnd - uiStart;
	sChange.eChange = TDC_Removed;
	sChange.uiStart = uiStart;
	sChange.uiEnd = uiEnd;
	sChange.uiLine = FindLine(uiStart);
	uiLast = FindLine(uiEnd);
	sChange.uiLines = uiLast - sChange.uiLine;
	sChange.bLineStart = (sChange.uiLines > 0) && (*mauiLineStarts.Get(sChange.uiLine) == uiStart) && (*mauiLineStarts.Get(uiLast) == uiEnd);

	if (sChange.uiLines > 0)
	{
		mauiLineStarts.RemoveRange(sChange.uiLine + 1, uiLast + 1, true);
	}
	for (i = sChange.uiLine + 1; i < mauiLineStarts.NumElements(); i++)
	{
		*mauiLineStarts.Get(i) -= uiLength;
	}
	mszText.Remove(uiStart, uiEnd);

	Changed(&sChange);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::Changed(STextDocumentChange* psChange)
{
	size	i;

	for (i = 0; i < mapcListeners.NumElements(); i++)
	{
		mapcListeners.GetPtr(i)->DocumentChanged(psChange);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CTextDocument::Text(void) { return mszText.Text(); }
size CTextDocument::Length(void) { return mszText.Length(); }
size CTextDocument::GetChangeCount(void) { return muiChangeCount; }  //Each edit ended counts one more, so the text's owner can tell it changed since it last looked.
CTextNavigation* CTextDocument::GetNavigation(void) { return &mcNavigation; }
size CTextDocument::NumLines(void) { return mauiLineStarts.NumElements(); }
CArrayTextLineStart* CTextDocument::GetLineStarts(void) { return &mauiLineStarts; }


//////////////////////////////////////////////////////////////////////////
//
// The end of the text for a line past the last.
//
//////////////////////////////////////////////////////////////////////////
size CTextDocument::GetLineStart(size uiLine)
{
	if (uiLine >= mauiLineStarts.NumElements())
	{
		return mszText.Length();
	}
	return *mauiLineStarts.Get(uiLine);
}


//////////////////////////////////////////////////////////////////////////
//
// Where line uiLine ends: its line feed, or the end of the text for the
// last line and any past it.
//
//////////////////////////////////////////////////////////////////////////
size CTextDocument::GetLineEnd(size uiLine)
{
	if (uiLine >= mauiLineStarts.NumElements() - 1)
	{
		return mszText.Length();
	}
	return *mauiLineStarts.Get(uiLine + 1) - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// The line uiPosition is on: the last line starting at or before it.
//
//////////////////////////////////////////////////////////////////////////
size CTextDocument::FindLine(size uiPosition)
{
	size	uiLow;
	size	uiHigh;
	size	uiMiddle;

	uiLow = 0;
	uiHigh = mauiLineStarts.NumElements() - 1;
	while (uiLow < uiHigh)
	{
		uiMiddle = (size)((uiLow + uiHigh + 1) / 2);
		if (*mauiLineStarts.Get(uiMiddle) <= uiPosition)
		{
			uiLow = uiMiddle;
		}
		else
		{
			uiHigh = uiMiddle - 1;
		}
	}
	return uiLow;
}


//////////////////////////////////////////////////////////////////////////
//
// The line starts from uiFrom on, those before it being right.
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::FindLineStarts(size uiFrom)
{
	size	uiPosition;
	size	uiLength;
	char*	szText;

	while ((mauiLineStarts.NumElements() > 0) && (*mauiLineStarts.Tail() > uiFrom))
	{
		mauiLineStarts.RemoveTail();
	}
	if (mauiLineStarts.NumElements() == 0)
	{
		*mauiLineStarts.Add() = 0;
	}
	szText = mszText.Text();
	uiLength = mszText.Length();
	for (uiPosition = uiFrom; uiPosition < uiLength; uiPosition++)
	{
		if (szText[uiPosition] == '\n')
		{
			*mauiLineStarts.Add() = uiPosition + 1;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A document that isn't undoable keeps no history: its edits are
// dropped and it can't be undone or redone.  The editor clears the
// history after changing it (ClearUndo).
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::SetUndoable(bool bUndoable)
{
	mbUndoable = bUndoable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextDocument::IsUndoable(void) { return mbUndoable; }
CTextUndo* CTextDocument::GetUndo(void) { return &mcUndo; }
void CTextDocument::BeginEdit(size uiCaret, size uiAnchor) { mcUndo.BeginEdit(uiCaret, uiAnchor); }


//////////////////////////////////////////////////////////////////////////
//
// Begins an edit unless one has been begun: an edit made of several
// changes keeps the caret from before the first.
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::StartEdit(size uiCaret, size uiAnchor)
{
	if (!mcUndo.IsEditStarted())
	{
		mcUndo.BeginEdit(uiCaret, uiAnchor);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Ends an edit (or the changes of an undo or redo loading) with the
// caret and selection after it: it counts as a change and is kept in
// the history.
//
//////////////////////////////////////////////////////////////////////////
ETextUndoKeep CTextDocument::EndEdit(size uiCaret, size uiAnchor)
{
	muiChangeCount++;
	if (!mbUndoable)
	{
		mcUndo.DropEdit();
		return TUK_None;
	}
	return mcUndo.Keep(mszText.Text(), uiCaret, uiAnchor);
}


//////////////////////////////////////////////////////////////////////////
//
// The history starts again with the text as it is.
//
//////////////////////////////////////////////////////////////////////////
void CTextDocument::ClearUndo(size uiCaret, size uiAnchor)
{
	mcUndo.Clear(mbUndoable ? mszText.Text() : NULL, uiCaret, uiAnchor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CTextDocument::CanUndo(void) { return mbUndoable && (mcUndo.FindUndo() != ARRAY_ELEMENT_NOT_FOUND); }
bool CTextDocument::CanRedo(void) { return mbUndoable && (mcUndo.FindRedo() != ARRAY_ELEMENT_NOT_FOUND); }


//////////////////////////////////////////////////////////////////////////
//
// Changes the text to state uiIndex of the history (an undo or redo),
// changing only the part that differs.  Its caret and selection are
// returned for the editor to put back before it ends the load as an edit
// (EndEdit) and calls the history's Loaded.
//
//////////////////////////////////////////////////////////////////////////
bool CTextDocument::Load(size uiIndex, size* puiCaret, size* puiAnchor)
{
	char*	szTarget;
	size	uiTargetLength;
	size	uiPrefix;
	size	uiSuffix;
	CChars	szMiddle;

	if (!mbUndoable || !mcUndo.CanLoad(uiIndex))
	{
		return false;
	}

	szTarget = mcUndo.GetLoadText(uiIndex, puiCaret, puiAnchor);
	uiTargetLength = StrLen(szTarget);
	mcUndo.FindDifference(mszText.Text(), mszText.Length(), szTarget, uiTargetLength, &uiPrefix, &uiSuffix);
	szMiddle.Init(szTarget, uiPrefix, uiTargetLength - uiSuffix);
	mcUndo.Loading(uiIndex);
	Remove(uiPrefix, mszText.Length() - uiSuffix);
	Insert(uiPrefix, szMiddle.Text());
	szMiddle.Kill();
	return true;
}

