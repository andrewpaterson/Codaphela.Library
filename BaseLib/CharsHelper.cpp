/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "Numbers.h"
#include "IntegerHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCharsHelper::GenerateRandomNumbersCharList(int iNum)
{
	int		i;
	int		r;
	CChars	sz;

	sz.Init("	unsigned char		uc[] = {");
	for (i = 0; i < iNum; i++)
	{
		r = Random(0, 255);
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
void CCharsHelper::GenerateBitStream(unsigned char* ucBytes, int iByteCount)
{
	BOOL	b;
	CChars	sz;

	sz.Init();
	for (int i = 0; i < iByteCount; i++)
	{
		for (int j = 0; j < 8; j++)
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
	int				i;
	int				iNumElements;
	CChars*			psz;
	int				iIndex;

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
		if (iIndex != -1)
		{
			paszDest->InsertIntoSorted(psz);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCharsHelper::InverseIntersect(CArrayChars* paszDest, CArrayChars* paszLeft, CArrayChars* paszRight)
{
	CArrayChars aszIntersection;
	int			i;
	CChars*		psz;
	int			iIndex;

	aszIntersection.Init();
	Intersect(&aszIntersection, paszLeft, paszRight);

	for (i = 0; i < paszLeft->NumElements(); i++)
	{
		psz = paszLeft->Get(i);
		iIndex = aszIntersection.FindInSorted(psz);
		if (iIndex == -1)
		{
			paszDest->Add(psz);
		}
	}

	for (i = 0; i < paszRight->NumElements(); i++)
	{
		psz = paszRight->Get(i);
		iIndex = aszIntersection.FindInSorted(psz);
		if (iIndex == -1)
		{
			paszDest->Add(psz);
		}
	}

	aszIntersection.Kill();
}

