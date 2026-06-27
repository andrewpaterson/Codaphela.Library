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
#include "Numbers.h"
#include "IntegerHelper.h"
#include "IntVec2.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SIntVec2::Print(CChars* psx, int iWholeNumbers)
{
	CChars		szzz;
	int			iWidth;

	if (iWholeNumbers == -1)
	{
		iWholeNumbers = WholeNumbers();
		iWholeNumbers++;
		if (iWholeNumbers < 2)
		{
			iWholeNumbers = 2;
		}
	}

	iWidth = iWholeNumbers + 1;

	psx->Append('[');
	szzz.Init();
	szzz.Append(x);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(',');
	szzz.Init();
	szzz.Append(y);
	szzz.RightAlign(' ', iWidth);
	psx->Append(szzz);
	szzz.Kill();
	psx->Append(']');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int SIntVec2::WholeNumbers(void)
{
	int i[2];

	i[0] = CountDigits(x);
	i[1] = CountDigits(y);

	return LargestInt(i, 2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int IntVec2LengthSq(const SIntVec2 *pV)
{
	return pV->x * pV->x + pV->y * pV->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int IntVec2Dot(const SIntVec2* pV1, const SIntVec2* pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int IntVec2Cross(const SIntVec2* pV1, const SIntVec2* pV2)
{
	return pV1->x * pV2->y - pV1->y * pV2->x;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SIntVec2* IntVec2Add(SIntVec2 *pOut, const SIntVec2* pV1, const SIntVec2* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SIntVec2* IntVec2Subtract(SIntVec2 *pOut, const SIntVec2* pV1, const SIntVec2* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SIntVec2* IntVec2Minimize(SIntVec2 *pOut, const SIntVec2* pV1, const SIntVec2* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SIntVec2* IntVec2Maximize(SIntVec2 *pOut, const SIntVec2* pV1, const SIntVec2* pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SIntVec2* IntVec2Scale(SIntVec2 *pOut, const SIntVec2 *pV, float32 s)
{
	pOut->x = (int)(pV->x * s);
	pOut->y = (int)(pV->y * s);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SIntVec2* IntVec2Lerp(SIntVec2 *pOut, const SIntVec2* pV1, const SIntVec2* pV2, float32 s)
{
	pOut->x = pV1->x + (int)(s * (pV2->x - pV1->x));
	pOut->y = pV1->y + (int)(s * (pV2->y - pV1->y));
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IntVec2Swap(SIntVec2* ps1, SIntVec2* ps2)
{
	SIntVec2 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IntVec2MinMax(SIntVec2* psMin, SIntVec2* psMax, SIntVec2* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SIntVec2*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SIntVec2*)RemapSinglePointer(asIn, i * iInStride);

			if (psIn->x < psMin->x)
			{
				psMin->x = psIn->x;
			}
			if (psIn->y < psMin->y)
			{
				psMin->y = psIn->y;
			}

			if (psIn->x > psMax->x)
			{
				psMax->x = psIn->x;
			}
			if (psIn->y > psMax->y)
			{
				psMax->y = psIn->y;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void IntVec2InterpolatePosition(SIntVec2* psVecDest, const SIntVec2* psVec1, const SIntVec2* psVec2, float32 fWeight)
{
	SIntVec2		sVec1;
	SIntVec2		sVec2;

	IntVec2Scale(&sVec1, psVec1, fWeight);
	IntVec2Scale(&sVec2, psVec2, 1.0f - fWeight);
	IntVec2Add(psVecDest, &sVec1, &sVec2);
}

