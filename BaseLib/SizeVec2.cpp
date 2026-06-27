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
#include "SizeVec2.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SSizeVec2::Print(CChars* psx, size iWholeNumbers)
{
	CChars		szzz;
	size			iWidth;

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
size SSizeVec2::WholeNumbers(void)
{
	size i[2];

	i[0] = CountDigits((int)x);
	i[1] = CountDigits((int)y);

	return LargestInt((int*)i, 2);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size SizeVec2LengthSq(const SSizeVec2 *pV)
{
	return pV->x * pV->x + pV->y * pV->y;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SSizeVec2* SizeVec2Add(SSizeVec2 *pOut, const SSizeVec2* pV1, const SSizeVec2* pV2)
{
	pOut->x = pV1->x + pV2->x;
	pOut->y = pV1->y + pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SSizeVec2* SizeVec2Subtract(SSizeVec2 *pOut, const SSizeVec2* pV1, const SSizeVec2* pV2)
{
	pOut->x = pV1->x - pV2->x;
	pOut->y = pV1->y - pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SSizeVec2* SizeVec2Minimize(SSizeVec2 *pOut, const SSizeVec2* pV1, const SSizeVec2* pV2)
{
	pOut->x = pV1->x < pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y < pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SSizeVec2* SizeVec2Maximize(SSizeVec2 *pOut, const SSizeVec2* pV1, const SSizeVec2* pV2)
{
	pOut->x = pV1->x > pV2->x ? pV1->x : pV2->x;
	pOut->y = pV1->y > pV2->y ? pV1->y : pV2->y;
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SSizeVec2* SizeVec2Scale(SSizeVec2 *pOut, const SSizeVec2 *pV, float32 s)
{
	pOut->x = (size)(pV->x * s);
	pOut->y = (size)(pV->y * s);
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SSizeVec2* SizeVec2Lerp(SSizeVec2 *pOut, const SSizeVec2* pV1, const SSizeVec2* pV2, float32 s)
{
	pOut->x = pV1->x + (size)(s * (pV2->x - pV1->x));
	pOut->y = pV1->y + (size)(s * (pV2->y - pV1->y));
	return pOut;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SizeVec2Swap(SSizeVec2* ps1, SSizeVec2* ps2)
{
	SSizeVec2 temp;

	temp = *ps2;
	*ps2 = *ps1;
	*ps1 = temp;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SizeVec2MinMax(SSizeVec2* psMin, SSizeVec2* psMax, SSizeVec2* asIn, size iInStride, size iNumPoints)
{
	size			i;
	SSizeVec2*	psIn;

	if (iNumPoints > 0)
	{
		psIn = asIn;
		*psMin = *psIn;
		*psMax = *psIn;

		for (i = 1; i < iNumPoints; i++)
		{
			psIn = (SSizeVec2*)RemapSinglePointer(asIn, i * iInStride);

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
void SizeVec2InterpolatePosition(SSizeVec2* psVecDest, const SSizeVec2* psVec1, const SSizeVec2* psVec2, float32 fWeight)
{
	SSizeVec2		sVec1;
	SSizeVec2		sVec2;

	SizeVec2Scale(&sVec1, psVec1, fWeight);
	SizeVec2Scale(&sVec2, psVec2, 1.0f - fWeight);
	SizeVec2Add(psVecDest, &sVec1, &sVec2);
}

