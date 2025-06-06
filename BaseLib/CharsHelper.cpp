/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "CharsHelper.h"
#include "StdRandom.h"
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCharsHelper::GenerateRandomNumbersCharList(size iNum)
{
	size		i;
	int			r;
	CChars		sz;
	CRandom		cRandom;

	cRandom.Init();

	sz.Init("	uint8		uc[] = {");
	for (i = 0; i < iNum; i++)
	{
		r = cRandom.Next(0, 255);
		sz.Append("0x");
		sz.AppendHexHiLo(&r, 1);
		if (i != iNum-1)
		{
			sz.Append(", ");
		}
	}
	sz.Append("};\n");
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCharsHelper::GenerateBitStream(uint8* ucBytes, size iByteCount)
{
	bool	b;
	CChars	sz;
	size	i;
	size	j;

	sz.Init();
	for (i = 0; i < iByteCount; i++)
	{
		for (j = 0; j < 8; j++)
		{
			b = GetBit(j, &ucBytes[i]);
			if (b)
			{
				sz.Insert(0, "1");
			}
			else
			{
				sz.Insert(0, "0");
			}
		}
		sz.Insert(0, " ");
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCharsHelper::Intersect(CArrayChars* paszDest, CArrayChars* paszLeft, CArrayChars* paszRight)
{
	CArrayChars*	paszIterate;
	CArrayChars		aszSorted;
	size			i;
	size			iNumElements;
	CChars*			psz;
	size			iIndex;

	if (paszLeft->NumElements() > paszRight->NumElements())
	{
		aszSorted.Init(paszLeft);
		paszIterate = paszRight;
	}
	else
	{
		aszSorted.Init(paszRight);
		paszIterate = paszLeft;
	}
	aszSorted.QuickSort();

	iNumElements = paszIterate->NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		psz = paszIterate->Get(i);
		iIndex = aszSorted.FindInSorted(psz);
		if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
		{
			paszDest->InsertIntoSorted(psz);
		}
	}
	aszSorted.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCharsHelper::InverseIntersect(CArrayChars* paszDest, CArrayChars* paszLeft, CArrayChars* paszRight)
{
	CArrayChars aszIntersection;
	size		i;
	CChars*		psz;
	size		iIndex;

	aszIntersection.Init();
	Intersect(&aszIntersection, paszLeft, paszRight);

	for (i = 0; i < paszLeft->NumElements(); i++)
	{
		psz = paszLeft->Get(i);
		iIndex = aszIntersection.FindInSorted(psz);
		if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
		{
			paszDest->Add(psz);
		}
	}

	for (i = 0; i < paszRight->NumElements(); i++)
	{
		psz = paszRight->Get(i);
		iIndex = aszIntersection.FindInSorted(psz);
		if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
		{
			paszDest->Add(psz);
		}
	}

	aszIntersection.Kill();
}

