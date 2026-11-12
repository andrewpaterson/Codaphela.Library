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
#include "StandardLib/ClassDefines.h"
#include "ClipboardHistory.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClipboardHistory::Init(void)
{
	PreInit();
	maszEntries.Init();
	mauiBlocks.Init();
	muiLimit = CLIPBOARD_HISTORY_LIMIT;
	muiMaxEntryBytes = CLIPBOARD_HISTORY_MAX_ENTRY_BYTES;
	mpcChooser = NULL;
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClipboardHistory::Class(void)
{
	U_Unknown(CArrayChars, maszEntries);
	U_Unknown(CArrayTemplate<uint8>, mauiBlocks);
	U_Size(muiLimit);
	U_Size(muiMaxEntryBytes);
	U_Pointer(mpcChooser);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClipboardHistory::Free(void)
{
	mauiBlocks.Kill();
	maszEntries.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// It is for the session only.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboardHistory::Save(CObjectWriter* pcFile) { return false; }
bool CClipboardHistory::Load(CObjectReader* pcFile) { return false; }


//////////////////////////////////////////////////////////////////////////
//
// The most entries kept, at least one.  Fewer than there are drops the
// oldest.
//
//////////////////////////////////////////////////////////////////////////
void CClipboardHistory::SetLimit(size uiLimit)
{
	muiLimit = (uiLimit > 0) ? uiLimit : 1;
	while (maszEntries.NumElements() > muiLimit)
	{
		Remove(maszEntries.NumElements() - 1);
	}
}


size CClipboardHistory::GetLimit(void) { return muiLimit; }
void CClipboardHistory::SetMaxEntryBytes(size uiBytes) { muiMaxEntryBytes = uiBytes; }  //A text bigger than this isn't kept.
void CClipboardHistory::SetChooser(CClipboardChooser* pcChooser) { mpcChooser = pcChooser; }  //What asks which entry to paste, NULL for nothing.
CClipboardChooser* CClipboardHistory::GetChooser(void) { return mpcChooser; }


//////////////////////////////////////////////////////////////////////////
//
// szText becomes the newest entry: added, or if it is already an entry
// that one moved to the front.  It is a block (copied from a box) or not
// as bBlock says, whatever it was before.  The oldest goes if that is
// more than the limit.  False if it isn't kept: it is empty or too big.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboardHistory::Add(const char* szText, bool bBlock)
{
	size	uiIndex;

	if ((szText == NULL) || (szText[0] == '\0') || (strlen(szText) > muiMaxEntryBytes))
	{
		return false;
	}
	if (Find(szText, &uiIndex))
	{
		Remove(uiIndex);
	}
	maszEntries.Add(szText);
	*mauiBlocks.Add() = bBlock ? 1 : 0;
	if (maszEntries.NumElements() > muiLimit)
	{
		Remove(maszEntries.NumElements() - 1);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The entry was copied from a box selection: its lines are a column.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboardHistory::IsBlock(size uiIndex) { return *mauiBlocks.Get(IndexOf(uiIndex)) != 0; }


//////////////////////////////////////////////////////////////////////////
//
// szText is the newest entry and a block: what is on the clipboard, if
// szText is, was copied here from a box.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboardHistory::IsBlockText(const char* szText) { return IsNewest(szText) && IsBlock(0); }


//////////////////////////////////////////////////////////////////////////
//
// Entries are numbered from 0, the newest.
//
//////////////////////////////////////////////////////////////////////////
void CClipboardHistory::Remove(size uiIndex)
{
	size	i;

	for (i = IndexOf(uiIndex); i + 1 < mauiBlocks.NumElements(); i++)
	{
		*mauiBlocks.Get(i) = *mauiBlocks.Get(i + 1);
	}
	maszEntries.Remove(IndexOf(uiIndex));
	mauiBlocks.SetUsedElements(mauiBlocks.NumElements() - 1);
}


void CClipboardHistory::Clear(void) { maszEntries.ReInit(); mauiBlocks.ReInit(); }


//////////////////////////////////////////////////////////////////////////
//
// The entry becomes the newest, as when it is chosen to be pasted.
//
//////////////////////////////////////////////////////////////////////////
void CClipboardHistory::MoveToFront(size uiIndex)
{
	CChars	szText;
	bool	bBlock;

	if (uiIndex == 0)
	{
		return;
	}
	szText.Init(GetText(uiIndex));
	bBlock = IsBlock(uiIndex);
	Remove(uiIndex);
	maszEntries.Add(szText.Text());
	*mauiBlocks.Add() = bBlock ? 1 : 0;
	szText.Kill();
}


size CClipboardHistory::NumEntries(void) { return maszEntries.NumElements(); }
char* CClipboardHistory::GetText(size uiIndex) { return maszEntries.GetText(IndexOf(uiIndex)); }


//////////////////////////////////////////////////////////////////////////
//
// The entry that is szText exactly.  False if there is none.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboardHistory::Find(const char* szText, size* puiIndex)
{
	size	i;

	for (i = 0; i < maszEntries.NumElements(); i++)
	{
		if (strcmp(GetText(i), szText) == 0)
		{
			*puiIndex = i;
			return true;
		}
	}
	return false;
}


bool CClipboardHistory::IsNewest(const char* szText) { return (NumEntries() > 0) && (strcmp(GetText(0), szText) == 0); }


//////////////////////////////////////////////////////////////////////////
//
// Where entry uiIndex (0 the newest) is kept: they are kept oldest first.
//
//////////////////////////////////////////////////////////////////////////
size CClipboardHistory::IndexOf(size uiIndex)
{
	return maszEntries.NumElements() - 1 - uiIndex;
}

