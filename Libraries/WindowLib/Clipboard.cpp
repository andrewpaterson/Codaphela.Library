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
#include "NativeWindowFactory.h"
#include "NativeClipboard.h"
#include "Window.h"
#include "Clipboard.h"


//////////////////////////////////////////////////////////////////////////
//
// The window's factory makes the native clipboard.
//
//////////////////////////////////////////////////////////////////////////
void CClipboard::Init(Ptr<CWindow> pWindow)
{
	PreInit();

	mpWindow = pWindow;
	mpcNativeClipboard = pWindow->GetFactory()->CreateNativeClipboard(this);
	mbDetached = false;
	mszText.Init();

	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClipboard::Free(void)
{
	CNativeWindowFactory*	pcFactory;

	if (mpcNativeClipboard)
	{
		pcFactory = mpcNativeClipboard->GetFactory();
		pcFactory->DestroyNativeClipboard(mpcNativeClipboard);
		mpcNativeClipboard = NULL;
	}
	mszText.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClipboard::Class(void)
{
	U_Pointer(mpcNativeClipboard);
	Nes(M_Pointer(mpWindow));
	M_Pointer(mpHistory);
	M_Pointer(mpShared);
	U_Bool(mbDetached);
	U_Unknown(CChars, mszText);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClipboard::Save(CObjectWriter* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CClipboard::Load(CObjectReader* pcFile)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Replaces what is on the clipboard with szText (UTF8), which becomes the
// newest entry of the history.  Returns false if the clipboard couldn't be
// written, as when another program has it open: the history has it still.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboard::SetText(const char* szText)
{
	CClipboard*	pcDetached;

	GetHistory()->Add(szText);
	pcDetached = FindDetached();
	if (pcDetached)
	{
		pcDetached->mszText.Set(szText == NULL ? "" : szText);
		return true;
	}
	if (mpcNativeClipboard == NULL)
	{
		return false;
	}
	return mpcNativeClipboard->SetText(szText == NULL ? "" : szText);
}


//////////////////////////////////////////////////////////////////////////
//
// Appends the clipboard's text (UTF8) to pszDest.  Returns false, and
// appends nothing, if the clipboard has no text or couldn't be read.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboard::GetText(CChars* pszDest)
{
	CClipboard*	pcDetached;

	pcDetached = FindDetached();
	if (pcDetached)
	{
		pszDest->Append(pcDetached->mszText);
		return !pcDetached->mszText.Empty();
	}
	if (mpcNativeClipboard == NULL)
	{
		return false;
	}
	return mpcNativeClipboard->GetText(pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
// As GetText, to paste it: the text is made the newest entry of the
// history if it isn't, as it isn't when another program put it there.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboard::TakeText(CChars* pszDest)
{
	size	uiLength;
	char*	szText;

	uiLength = pszDest->Length();
	if (!GetText(pszDest))
	{
		return false;
	}
	szText = &pszDest->Text()[uiLength];
	if ((szText[0] != '\0') && !GetHistory()->IsNewest(szText))
	{
		GetHistory()->Add(szText);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Looks at the system's clipboard: its text is made the newest entry of
// the history if it isn't.  True if it had text.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboard::Sync(void)
{
	CChars	szText;
	bool	bText;

	szText.Init();
	bText = TakeText(&szText);
	szText.Kill();
	return bText;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
// The history what is put on the clipboard is kept in: the one of the
// clipboard it shares with, else its own, made when it is first wanted.
//
//////////////////////////////////////////////////////////////////////////
Ptr<CClipboardHistory> CClipboard::GetHistory(void)
{
	if (mpShared.IsNotNull())
	{
		return mpShared->GetHistory();
	}
	if (mpHistory.IsNull())
	{
		mpHistory = OMalloc<CClipboardHistory>();
	}
	return mpHistory;
}


//////////////////////////////////////////////////////////////////////////
//
// The clipboard's history is pClipboard's from now, as a dialog's is its
// owner's; NULL for its own again.  Its own, if it had one, is let go.
//
//////////////////////////////////////////////////////////////////////////
void CClipboard::ShareHistory(Ptr<CClipboard> pClipboard)
{
	mpShared = (pClipboard == this) ? Ptr<CClipboard>() : pClipboard;
	if (mpShared.IsNotNull())
	{
		mpHistory = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What asks which of the history's entries to paste, NULL if there is
// nothing to ask: no history has been wanted yet, or it has no chooser.
//
//////////////////////////////////////////////////////////////////////////
CClipboardChooser* CClipboard::GetChooser(void)
{
	if (mpShared.IsNotNull())
	{
		return mpShared->GetChooser();
	}
	return mpHistory.IsNotNull() ? mpHistory->GetChooser() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Detached, the clipboard's text is its own, not the system's: nothing
// it is given reaches other programs and nothing they copy reaches it.
// The clipboards that share its history have its text too.
//
//////////////////////////////////////////////////////////////////////////
void CClipboard::SetDetached(bool bDetached)
{
	mbDetached = bDetached;
	mszText.Clear();
}


bool CClipboard::IsDetached(void) { return FindDetached() != NULL; }


//////////////////////////////////////////////////////////////////////////
//
// The clipboard whose own text is this one's: itself or the one it shares
// with, if detached.  NULL if its text is the system's.
//
//////////////////////////////////////////////////////////////////////////
CClipboard* CClipboard::FindDetached(void)
{
	if (mbDetached)
	{
		return this;
	}
	return mpShared.IsNotNull() ? mpShared->FindDetached() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CWindow> CClipboard::GetWindow(void) { return mpWindow; }
CNativeClipboard* CClipboard::GetNativeClipboard(void) { return mpcNativeClipboard; }

