/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
size CountCarriageReturns(char* szString, size uiLen)
{
	size	i;
	size	uiNumCarriageReturns;

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
size CountLineFeeds(char* szString, size uiLen)
{
	size	i;
	size	iNumLineFeeds;

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
size CountNewLines(char* szString, size uiLen)
{
	size iCarriageReturns;
	size iLineFeeds;

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
		return SIZE_MAX;
	}
	return iCarriageReturns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size PassifyNewlines(char *szString)
{
	size	uiLen;
	size	i;
	size	uiNumCarriageReturns;
	char*	szNewString;  //Should use StackMemory.
	size	uiPos;

	uiLen = strlen(szString);
	uiNumCarriageReturns = CountCarriageReturns(szString, uiLen);

	if (uiNumCarriageReturns == 0)
	{
		return SIZE_MAX;
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

