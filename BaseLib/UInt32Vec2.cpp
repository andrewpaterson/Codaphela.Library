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
#include "UInt32Vec2.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SUInt32Vec2::Print(CChars* psx, uint32 iWholeNumbers)
{
	CChars		szzz;
	uint32			iWidth;

	if (iWholeNumbers == MAX_UINT32)
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
uint32 SUInt32Vec2::WholeNumbers(void)
{
	int32 i[2];

	i[0] = (int32)CountDigits((int64)x);
	i[1] = (int32)CountDigits((int64)y);

	return LargestInt(i, 2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
uint32 UInt32Vec2LengthSq(const SUInt32Vec2 *pV)
{
	return pV->x * pV->x + pV->y * pV->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SUInt32Vec2* UInt32Vec2Add(SUInt32Vec2 *pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SUInt32Vec2* UInt32Vec2Subtract(SUInt32Vec2 *pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SUInt32Vec2* UInt32Vec2Minimize(SUInt32Vec2 *pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SUInt32Vec2* UInt32Vec2Maximize(SUInt32Vec2 *pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SUInt32Vec2* UInt32Vec2Scale(SUInt32Vec2 *pOut, const SUInt32Vec2 *pV, float32 s)
{
	pOut->x = (uint32)(pV->x * s);
	pOut->y = (uint32)(pV->y * s);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SUInt32Vec2* UInt32Vec2Lerp(SUInt32Vec2 *pOut, const SUInt32Vec2* pV1, const SUInt32Vec2* pV2, float32 s)
{
	pOut->x = pV1->x + (uint32)(s * (pV2->x - pV1->x));
	pOut->y = pV1->y + (uint32)(s * (pV2->y - pV1->y));
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void UInt32Vec2Swap(SUInt32Vec2* ps1, SUInt32Vec2* ps2)
{
	SUInt32Vec2 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void UInt32Vec2MinMax(SUInt32Vec2* psMin, SUInt32Vec2* psMax, SUInt32Vec2* asIn, uint32 iInStride, uint32 iNumPoints)
{
	uint32			i;
	SUInt32Vec2*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SUInt32Vec2*)RemapSinglePointer(asIn, i * iInStride);

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
void UInt32Vec2InterpolatePosition(SUInt32Vec2* psVecDest, const SUInt32Vec2* psVec1, const SUInt32Vec2* psVec2, float32 fWeight)
{
	SUInt32Vec2		sVec1;
	SUInt32Vec2		sVec2;

	UInt32Vec2Scale(&sVec1, psVec1, fWeight);
	UInt32Vec2Scale(&sVec2, psVec2, 1.0f - fWeight);
	UInt32Vec2Add(psVecDest, &sVec1, &sVec2);
}

