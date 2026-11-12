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
#ifndef __CLIPBOARD_HISTORY_H__
#define __CLIPBOARD_HISTORY_H__
#include "BaseLib/ArrayChars.h"
#include "StandardLib/Object.h"
#include "StandardLib/Pointer.h"


#define CLIPBOARD_HISTORY_LIMIT				20			//The entries kept, as IntelliJ IDEA's default.
#define CLIPBOARD_HISTORY_MAX_ENTRY_BYTES	(1024 * 1024)	//A text bigger than this is copied but not kept.


//The texts cut and copied in an application, newest first, as IntelliJ IDEA keeps them for Paste from History
//(Control+Shift+V): every text component's cut and copy adds to it, through its window's CClipboard, and so does text
//copied in another program when the clipboard is next looked at (CClipboard::Sync).  A text that is already in it
//moves to the front rather than being kept twice, and past its limit the oldest goes.  Empty text isn't kept, nor a
//text bigger than its biggest entry.
//
//There is one for an application: a window's clipboard makes its own, and a window shown over or owned by another (a
//dialog, a tool window) is given its owner's (CClipboard::SetHistory).  It is for the session: it isn't saved.
class CClipboardHistory : public CObject
{
CONSTRUCTABLE(CClipboardHistory);
DESTRUCTABLE(CClipboardHistory);
protected:
	CArrayChars		maszEntries;  //Oldest first.
	size			muiLimit;
	size			muiMaxEntryBytes;

public:
	void	Init(void);
	void	Class(void) override;
	void	Free(void) override;
	bool	Save(CObjectWriter* pcFile) override;
	bool	Load(CObjectReader* pcFile) override;

	void	SetLimit(size uiLimit);
	size	GetLimit(void);
	void	SetMaxEntryBytes(size uiBytes);

	bool	Add(const char* szText);
	void	Remove(size uiIndex);
	void	Clear(void);
	void	MoveToFront(size uiIndex);
	size	NumEntries(void);
	char*	GetText(size uiIndex);
	bool	Find(const char* szText, size* puiIndex);
	bool	IsNewest(const char* szText);

protected:
	size	IndexOf(size uiIndex);
};


#endif // __CLIPBOARD_HISTORY_H__

