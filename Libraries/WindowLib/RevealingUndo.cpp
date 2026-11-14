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
	mszShown.Init();
	mbShownRedo = false;
}


void CRevealingUndo::Kill(void)
{
	mszShown.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Undo (or with bRedo, redo) pressed, with keys szKeys ("Ctrl+Z"): the
// change undone if it's in view, or if it was shown by the press before
// and nothing has changed since; otherwise it's shown, and pszMessage
// (emptied) says to press again.  False if there was nothing to undo.
//
//////////////////////////////////////////////////////////////////////////
bool CRevealingUndo::Press(bool bRedo, const char* szKeys, CChars* pszMessage)
{
	CChars	szState;
	bool	bAgain;

	pszMessage->Clear();
	szState.Init();
	mpcHost->AppendUndoState(&szState);
	bAgain = !mszShown.Empty() && (mbShownRedo == bRedo) && szState.Equals(mszShown.Text());
	szState.Kill();
	Forget();

	if (bAgain || mpcHost->IsUndoShown(bRedo))
	{
		return mpcHost->DoUndo(bRedo);
	}
	if (!mpcHost->ShowUndo(bRedo))
	{
		return false;
	}

	mpcHost->AppendUndoState(&mszShown);
	mbShownRedo = bRedo;
	pszMessage->Append(szKeys);
	pszMessage->Append(bRedo ? " again to redo the change " : " again to undo the change ");
	mpcHost->AppendUndoWhere(bRedo, pszMessage);
	pszMessage->Append(".");
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The next press starts again, as after anything else is done.
//
//////////////////////////////////////////////////////////////////////////
void CRevealingUndo::Forget(void)
{
	mszShown.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the last press showed a change the next would undo.
//
//////////////////////////////////////////////////////////////////////////
bool CRevealingUndo::IsShowing(void)
{
	return !mszShown.Empty();
}

