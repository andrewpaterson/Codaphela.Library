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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CClipboard::Class(void)
{
		U_Pointer(mpcNativeClipboard);
	Nes(M_Pointer(mpWindow));
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
// Replaces what is on the clipboard with szText (UTF8).  Returns false if
// the clipboard couldn't be written, as when another program has it open.
//
//////////////////////////////////////////////////////////////////////////
bool CClipboard::SetText(const char* szText)
{
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
	if (mpcNativeClipboard == NULL)
	{
		return false;
	}
	return mpcNativeClipboard->GetText(pszDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CWindow> CClipboard::GetWindow(void) { return mpWindow; }
CNativeClipboard* CClipboard::GetNativeClipboard(void) { return mpcNativeClipboard; }

