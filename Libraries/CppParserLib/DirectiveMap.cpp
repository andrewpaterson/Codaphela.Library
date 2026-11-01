/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "DirectiveMap.h"


//////////////////////////////////////////////////////////////////////////
//
// Adds the directive called pcName.  Returns NULL if it is already in the
// map.
//
//////////////////////////////////////////////////////////////////////////
CPreprocessorDirective* CDirectiveMap::AddDirective(CExternalString* pcName, EPreprocessorDirective eDirective)
{
	CPreprocessorDirective*		pcDirective;
	int64						lliID;

	pcDirective = AddEntry(pcName, &lliID);
	if (pcDirective)
	{
		pcDirective->Init(pcName, eDirective, lliID, this);
	}
	return pcDirective;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds the directive called szName.  Returns NULL if it is already in the
// map.
//
//////////////////////////////////////////////////////////////////////////
CPreprocessorDirective* CDirectiveMap::AddDirective(char* szName, EPreprocessorDirective eDirective)
{
	CExternalString		cExternalString;

	cExternalString.Init(szName, StrLen(szName));
	return AddDirective(&cExternalString, eDirective);
}


//////////////////////////////////////////////////////////////////////////
//
// Returns the directive called szName, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CPreprocessorDirective* CDirectiveMap::GetDirective(char* szName, bool bExact)
{
	return Get(szName, bExact);
}


//////////////////////////////////////////////////////////////////////////
//
// Removes the directive called szName, if it is in the map.
//
//////////////////////////////////////////////////////////////////////////
void CDirectiveMap::RemoveDirective(char* szName)
{
	Remove(szName);
}

