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
#include "DebugOutput.h"
#include "ArrayInt.h"
#include "StringHelper.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayInt::AddRemap(size iElementPos, uint iRemapNum)
{
	uint*		pi;
	size		iIndex;
	size		iNumToAdd;
	size		iMinusOne;

	if (miUsedElements > iElementPos)
	{
		pi = CArrayTemplate<uint>::Get(iElementPos);
		(*pi) = iRemapNum;
	}
	else
	{
		iMinusOne = SIZE_MAX;
		iNumToAdd = (iElementPos - miUsedElements);

		for (iIndex = 0; iIndex < iNumToAdd; iIndex++)
		{
			Add(iMinusOne);
		}
		Add(iRemapNum);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayInt::RemoveRemap(size iElementPos)
{
	uint*	pi;
	size	iIndex;
	size	iCutDown;

	if (iElementPos < miUsedElements)
	{
		pi = CArrayTemplate<uint>::Get(iElementPos);
		(*pi) = -1;
	}

	iCutDown = miUsedElements;
	iIndex = miUsedElements;
	if (iIndex != 0)
	{
		do
		{
			iIndex--;
			pi = CArrayTemplate<uint>::Get(iElementPos);
			if (*pi == -1)
			{
				iCutDown = iIndex;
			}
			else
			{
				break;
			}
			if (iIndex == 0)
			{
				break;
			}
		}
		while (iIndex != 0);
	}
	if (iCutDown != miUsedElements)
	{
		SetUsedElements(iCutDown);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CArrayInt::FindUnusedInSorted(void)
{
	size	iIndex;
	uint	iValue;

	if (miUsedElements == 0)
	{
		return 0;
	}

	for (iIndex = 0; iIndex < miUsedElements; iIndex++)
	{
		iValue = *Get(iIndex);
		if (iValue != iIndex)
		{
			return iIndex;
		}
	}
	return miUsedElements;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayInt::AddList(uint iStop, ...)
{
	va_list		vaMarker;
	uint		iValue;

	va_start(vaMarker, iStop);
	iValue = va_arg(vaMarker, uint);
	while (iValue != iStop)
	{
		Add(iValue);
		iValue = va_arg(vaMarker, uint);
	}
	va_end(vaMarker);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CArrayInt::Dump(void)
{
	size	iIndex;
	uint	iValue;
	char	sz[32];


	EngineOutput("[");
	for (iIndex = 0; iIndex < miUsedElements; iIndex++)
	{
		iValue = GetValue(iIndex);
		IntToString(sz, 32, iValue, 10);
		EngineOutput(sz);
		EngineOutput(" ");
	}
	EngineOutput("]\n");
}

