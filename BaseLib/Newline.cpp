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
int CountCarriageReturns(char* szString, int iLen)
{
	int		i;
	int		iNumCarriageReturns;

	iNumCarriageReturns = 0;
	for (i = 0; i < iLen; i++)
	{
		if (szString[i] == '\r')
		{
			iNumCarriageReturns++;
		}
	}
	return iNumCarriageReturns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CountLineFeeds(char* szString, int iLen)
{
	int		i;
	int		iNumLineFeeds;

	iNumLineFeeds = 0;
	for (i = 0; i < iLen; i++)
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
int CountNewLines(char* szString, int iLen)
{
	int iCarriageReturns;
	int iLineFeeds;

	iCarriageReturns = CountCarriageReturns(szString, iLen);
	iLineFeeds = CountLineFeeds(szString, iLen);
	
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
		return -1;
	}
	return iCarriageReturns;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int PassifyNewlines(char *szString)
{
	int		iLen;
	int		i;
	int		iNumCarriageReturns;
	char*	szNewString;  //Should use StackMemory.
	int		iPos;

	iLen = (int)strlen(szString);
	iNumCarriageReturns = CountCarriageReturns(szString, iLen);

	if (iNumCarriageReturns == 0)
	{
		return -1;
	}

	szNewString	= (char*)malloc(iLen-iNumCarriageReturns+1);

	iPos = 0;
	for (i = 0; i <= iLen; i++)
	{
		if (szString[i] != '\r')
		{
			szNewString[iPos] = szString[i];
			iPos++;
		}
	}

	memcpy(szString, szNewString, iPos);
	SafeFree(szNewString);
	return iPos-1;
}

