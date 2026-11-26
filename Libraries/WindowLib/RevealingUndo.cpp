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
#include "RevealingUndo.h"


//////////////////////////////////////////////////////////////////////////
//
// pcHost isn't held; it must outlive this.
//
//////////////////////////////////////////////////////////////////////////
void CRevealingUndo::Init(CRevealingUndoHost* pcHost)
{
	mpcHost = pcHost;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRevealingUndo::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Undo (or with bRedo, redo) pressed, with keys szKeys ("Ctrl+Z"): the
// change undone if it's in view; otherwise it's shown, and pszMessage
// (emptied) says to press again.  False if there was nothing to undo.
//
//////////////////////////////////////////////////////////////////////////
bool CRevealingUndo::Press(bool bRedo, const char* szKeys, CChars* pszMessage)
{
	pszMessage->Clear();
	if (mpcHost->IsUndoShown(bRedo))
	{
		return mpcHost->DoUndo(bRedo);
	}
	if (!mpcHost->ShowUndo(bRedo))
	{
		return false;
	}
	pszMessage->Append(szKeys);
	pszMessage->Append(bRedo ? " again to redo the change " : " again to undo the change ");
	mpcHost->AppendUndoWhere(bRedo, pszMessage);
	pszMessage->Append(".");
	return true;
}

