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
#include "Int16Vec2.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SInt16Vec2::Print(CChars* psx, int iWholeNumbers)
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
int SInt16Vec2::WholeNumbers(void)
{
	int i[2];

	i[0] = CountDigits(x);
	i[1] = CountDigits(y);

	return LargestInt(i, 3);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int16 Int16Vec2LengthSq(const SInt16Vec2 *pV)
{
	return pV->x * pV->x + pV->y * pV->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int16 Int16Vec2Dot(const SInt16Vec2* pV1, const SInt16Vec2* pV2)
{
	return pV1->x * pV2->x + pV1->y * pV2->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int16 Int16Vec2Cross(const SInt16Vec2* pV1, const SInt16Vec2* pV2)
{
	return pV1->x * pV2->y - pV1->y * pV2->x;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInt16Vec2* Int16Vec2Add(SInt16Vec2 *pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInt16Vec2* Int16Vec2Subtract(SInt16Vec2 *pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInt16Vec2* Int16Vec2Minimize(SInt16Vec2 *pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInt16Vec2* Int16Vec2Maximize(SInt16Vec2 *pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInt16Vec2* Int16Vec2Scale(SInt16Vec2 *pOut, const SInt16Vec2 *pV, float32 s)
{
	pOut->x = (int16)(pV->x * s);
	pOut->y = (int16)(pV->y * s);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SInt16Vec2* Int16Vec2Lerp(SInt16Vec2 *pOut, const SInt16Vec2* pV1, const SInt16Vec2* pV2, float32 s)
{
	pOut->x = pV1->x + (int16)(s * (pV2->x - pV1->x));
	pOut->y = pV1->y + (int16)(s * (pV2->y - pV1->y));
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int16Vec2Swap(SInt16Vec2* ps1, SInt16Vec2* ps2)
{
	SInt16Vec2 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void Int16Vec2MinMax(SInt16Vec2* psMin, SInt16Vec2* psMax, SInt16Vec2* asIn, int iInStride, int iNumPoints)
{
	int			i;
	SInt16Vec2*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SInt16Vec2*)RemapSinglePointer(asIn, i * iInStride);

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
void Int16Vec2InterpolatePosition(SInt16Vec2* psVecDest, const SInt16Vec2* psVec1, const SInt16Vec2* psVec2, float32 fWeight)
{
	SInt16Vec2		sVec1;
	SInt16Vec2		sVec2;

	Int16Vec2Scale(&sVec1, psVec1, fWeight);
	Int16Vec2Scale(&sVec2, psVec2, 1.0f - fWeight);
	Int16Vec2Add(psVecDest, &sVec1, &sVec2);
}

