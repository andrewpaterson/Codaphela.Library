/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela MicroprocessorLib

Codaphela MicroprocessorLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela MicroprocessorLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela MicroprocessorLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/StringHelper.h"
#include "SchmittThresholds.h"


//In each part's rows VCC rises.
static SSchmittThreshold gasSchmittThresholds[] =
{
	//74LVC14 (Nexperia): only minimums and maximums, so the middle of each.
	{ "LVC",	"14",	1.65f,	0.85f,	0.50f },
	{ "LVC",	"14",	2.3f,	1.25f,	0.75f },
	{ "LVC",	"14",	2.7f,	1.55f,	1.10f },
	{ "LVC",	"14",	3.0f,	1.60f,	1.15f },
	{ "LVC",	"14",	3.6f,	1.60f,	1.15f },
	//74LVC1G17 (Nexperia): typical.
	{ "LVC",	"17",	1.8f,	1.0f,	0.6f },
	{ "LVC",	"17",	2.3f,	1.2f,	0.8f },
	{ "LVC",	"17",	3.0f,	1.5f,	1.0f },
	{ "LVC",	"17",	4.5f,	2.1f,	1.5f },
	{ "LVC",	"17",	5.5f,	2.5f,	1.8f },
	//74VHCT14 (Fairchild): VT+ at most 1.9V (2.1V), VT- at least 0.5V (0.6V), the hysteresis at least 0.4V, so VT+
	//from 0.9V to 1.9V and VT- from 0.5V to 1.5V; the middle of each.
	{ "VHCT",	"14",	4.5f,	1.40f,	1.00f },
	{ "VHCT",	"14",	5.5f,	1.55f,	1.15f },
};


//////////////////////////////////////////////////////////////////////////
//
// The part's rows: its first and how many.
//
//////////////////////////////////////////////////////////////////////////
static size FindSchmittRows(const char* szFamily, const char* szPart, SSchmittThreshold** ppsFirst)
{
	size	i;
	size	uiCount;

	uiCount = 0;
	*ppsFirst = NULL;
	for (i = 0; i < sizeof(gasSchmittThresholds) / sizeof(SSchmittThreshold); i++)
	{
		if ((StringCompare(gasSchmittThresholds[i].szFamily, szFamily) == 0) && (StringCompare(gasSchmittThresholds[i].szPart, szPart) == 0))
		{
			if (*ppsFirst == NULL)
			{
				*ppsFirst = &gasSchmittThresholds[i];
			}
			uiCount++;
		}
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool IsSchmittPart(const char* szFamily, const char* szPart)
{
	SSchmittThreshold*	psFirst;

	return FindSchmittRows(szFamily, szPart, &psFirst) > 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The part's VT+ and VT- at fVcc.  False if the part isn't a Schmitt
// trigger the table has.
//
//////////////////////////////////////////////////////////////////////////
bool GetSchmittThresholds(const char* szFamily, const char* szPart, float32 fVcc, float32* pfPositive, float32* pfNegative)
{
	SSchmittThreshold*	psFirst;
	SSchmittThreshold*	psLower;
	SSchmittThreshold*	psHigher;
	size				uiCount;
	size				i;
	float32				fFraction;

	uiCount = FindSchmittRows(szFamily, szPart, &psFirst);
	if (uiCount == 0)
	{
		return false;
	}
	if (fVcc <= psFirst[0].fVcc)
	{
		*pfPositive = psFirst[0].fPositive * fVcc / psFirst[0].fVcc;
		*pfNegative = psFirst[0].fNegative * fVcc / psFirst[0].fVcc;
		return true;
	}
	if (fVcc >= psFirst[uiCount - 1].fVcc)
	{
		*pfPositive = psFirst[uiCount - 1].fPositive * fVcc / psFirst[uiCount - 1].fVcc;
		*pfNegative = psFirst[uiCount - 1].fNegative * fVcc / psFirst[uiCount - 1].fVcc;
		return true;
	}
	for (i = 1; i < uiCount; i++)
	{
		if (fVcc <= psFirst[i].fVcc)
		{
			break;
		}
	}
	psLower = &psFirst[i - 1];
	psHigher = &psFirst[i];
	fFraction = (fVcc - psLower->fVcc) / (psHigher->fVcc - psLower->fVcc);
	*pfPositive = psLower->fPositive + (psHigher->fPositive - psLower->fPositive) * fFraction;
	*pfNegative = psLower->fNegative + (psHigher->fNegative - psLower->fNegative) * fFraction;
	return true;
}

