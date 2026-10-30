/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include <stdlib.h>
#include <string.h>
#include "PointerFunctions.h"
#include "Newline.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CountCarriageReturns(const char* szString, lsize uiLen)
{
	lsize	i;
	lsize	uiNumCarriageReturns;

	uiNumCarriageReturns = 0;
	for (i = 0; i < uiLen; i++)
	{
		if (szString[i] == '\r')
		{
			uiNumCarriageReturns++;
		}
	}
	return uiNumCarriageReturns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CountLineFeeds(char* szString, lsize uiLen)
{
	lsize	i;
	lsize	iNumLineFeeds;

	iNumLineFeeds = 0;
	for (i = 0; i < uiLen; i++)
	{
		if (szString[i] == '\n')
		{
			iNumLineFeeds++;
		}
	}
	return iNumLineFeeds;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize CountNewLines(char* szString, lsize uiLen)
{
	lsize iCarriageReturns;
	lsize iLineFeeds;

	iCarriageReturns = CountCarriageReturns(szString, uiLen);
	iLineFeeds = CountLineFeeds(szString, uiLen);
	
	if (iLineFeeds == 0)
	{
		iLineFeeds = iCarriageReturns;
	}
	if (iCarriageReturns == 0)
	{
		iCarriageReturns = iLineFeeds;
	}

	if (iLineFeeds != iCarriageReturns)
	{
		//There is no file format where /r and /n are not balanced.
		return MAX_LSIZE;
	}
	return iCarriageReturns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
lsize PassifyNewlines(char *szString)
{
	lsize	uiLen;
	lsize	i;
	lsize	uiNumCarriageReturns;
	char*	szNewString;  //Should use StackMemory.
	lsize	uiPos;

	uiLen = strlen(szString);
	uiNumCarriageReturns = CountCarriageReturns(szString, uiLen);

	if (uiNumCarriageReturns == 0)
	{
		return MAX_LSIZE;
	}

	szNewString	= (char*)malloc(uiLen-uiNumCarriageReturns+1);

	uiPos = 0;
	for (i = 0; i <= uiLen; i++)
	{
		if (szString[i] != '\r')
		{
			szNewString[uiPos] = szString[i];
			uiPos++;
		}
	}

	memcpy(szString, szNewString, uiPos);
	SafeFree(szNewString);
	return uiPos - 1;
}

