/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <string.h>
#include "EscapeCodes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char GetEscapeCode(char cCurrent)
{
	if (cCurrent == 'n')
	{
		return '\n';
	}
	else if (cCurrent == '\\')
	{
		return '\\';
	}
	else if (cCurrent == '\"')
	{
		return '\"';
	}
	else if (cCurrent == '\'')
	{
		return '\'';
	}
	else if (cCurrent == '?')
	{
		return '\?';
	}
	else if (cCurrent == '0')
	{
		return '\0';
	}
	else if (cCurrent == 'a')
	{
		return '\a';
	}				
	else if (cCurrent == 'b')
	{
		return '\b';
	}				
	else if (cCurrent == 'f')
	{
		return '\f';
	}				
	else if (cCurrent == 'r')
	{
		return '\r';
	}
	else if (cCurrent == 't')
	{
		return '\t';
	}
	else if (cCurrent == 'v')
	{
		return '\v';
	}

	//If @ is returned there was an error.
	return '@';
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int GetEscapeString(char cCurrent, char* szDest)
{
	if (cCurrent == '\\')
	{
		strcpy(szDest, "\\\\");
		return 1;
	}

	if ((cCurrent >= 32) && (cCurrent <= 127))
	{
		szDest[0] = cCurrent;
		szDest[1] = '\0';
		return 1;
	}
	
	if ((cCurrent >= 0) && (cCurrent <= 31))
	{
		if (cCurrent == '\n')
		{
			strcpy(szDest, "\\n");
			return 1;
		}
		else if (cCurrent == '\"')
		{
			strcpy(szDest, "\\\"");
			return 1;
		}
		else if (cCurrent == '\'')
		{
			strcpy(szDest, "\\'");
			return 1;
		}
		else if (cCurrent == '\?')
		{
			strcpy(szDest, "\\?");
			return 1;
		}
		else if (cCurrent == '\0')
		{
			strcpy(szDest, "\\0");
			return 1;
		}
		else if (cCurrent == '\a')
		{
			strcpy(szDest, "\\a");
			return 1;
		}				
		else if (cCurrent == '\b')
		{
			strcpy(szDest, "\\b");
			return 1;
		}				
		else if (cCurrent == '\f')
		{
			strcpy(szDest, "\\f");
			return 1;
		}				
		else if (cCurrent == '\r')
		{
			strcpy(szDest, "\\r");
			return 1;
		}
		else if (cCurrent == '\t')
		{
			strcpy(szDest, "\\t");
			return 1;
		}
		else if (cCurrent == '\v')
		{
			strcpy(szDest, "\\v");
			return 1;
		}
	}
	return 0;
}

