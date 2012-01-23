/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LanguageLib

Codaphela LanguageLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LanguageLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LanguageLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "Tokens.h"
#include "BaseLib/PointerRemapper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetTokenSize(int* piCurrent)
{
	switch(*piCurrent)
	{
	case ET_Error:
	case ET_Unknown: 
	case ET_Node:
		return ((STokenSingle*)piCurrent)->Size();

	case ET_Identifier:
		return ((STokenIdentifier*)piCurrent)->Size();

	case ET_Text:
		return ((STokenKeywordText*)piCurrent)->Size();

	case ET_Char:
		return ((STokenKeywordChar*)piCurrent)->Size();

	case PT_IntegerType:
		return ((STokenInteger*)piCurrent)->Size();

	case PT_RealType:
		return ((STokenReal*)piCurrent)->Size();

	case PT_StringType:
		return ((STokenUnknownText*)piCurrent)->Size();

	case PT_CharType:
		return ((STokenUnknownChar*)piCurrent)->Size();

	default:
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int* GetNextToken(int* piCurrent)
{
	int		iSize;

	iSize = GetTokenSize(piCurrent);
	if (iSize == 0)
	{
		return NULL;
	}
	return (int*)RemapSinglePointer(piCurrent, iSize*4);
}
