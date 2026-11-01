/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela LogicimLib

Codaphela LogicimLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela LogicimLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela LogicimLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "CircuitTextMarkupParser.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircuitTextMarkupParser::ReadOpen(const char* szMarkup, STextStyleOverride* psOverride, int* piKind)
{
	if (szMarkup[1] != '{')
	{
		return 0;
	}

	*piKind = 0;
	if (szMarkup[0] == '~')
	{
		psOverride->SetOverline(true);
		return 2;
	}
	else if (szMarkup[0] == '_')
	{
		psOverride->SetPosition(TP_Subscript);
		return 2;
	}
	else if (szMarkup[0] == '^')
	{
		psOverride->SetPosition(TP_Superscript);
		return 2;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCircuitTextMarkupParser::ReadClose(const char* szMarkup, int iKind)
{
	if (szMarkup[0] == '}')
	{
		return 1;
	}
	return 0;
}


