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
#include "WinRefWindowFactory.h"
#include "WinRefClipboard.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefClipboard::SetText(const char* szText)
{
	CWinRefWindowFactory*	pcFactory;

	pcFactory = (CWinRefWindowFactory*)mpcWindowFactory;
	pcFactory->GetClipboardText()->Set(szText);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Nothing has been copied until something is put on it.
//
//////////////////////////////////////////////////////////////////////////
bool CWinRefClipboard::GetText(CChars* pszDest)
{
	CWinRefWindowFactory*	pcFactory;
	CChars*					pszText;

	pcFactory = (CWinRefWindowFactory*)mpcWindowFactory;
	pszText = pcFactory->GetClipboardText();
	if (pszText->Empty())
	{
		return false;
	}
	pszDest->Append(pszText);
	return true;
}

