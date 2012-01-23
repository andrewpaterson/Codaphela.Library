/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "Indexes.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void SIndexedLevel::Init(void)
{
	memset(this, 0, sizeof(SIndexedLevel));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL SIndexedLevel::IsEmpty(void)
{
	int		i;

	for (i = 0; i < 256; i++)
	{
		if (apsLevels[i] != NULL)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexes::Init(int iChunkSize)
{
	mcLevels.Init(iChunkSize);
	msTop.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexes::Kill(void)
{
	mcLevels.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CIndexes::Get(OIndex OI)
{
	unsigned char	ucCurrent;
	int				iCurrent;
	unsigned char*	pvOI;
	SIndexedLevel*	psLevel;

	pvOI = (unsigned char*)&OI;
	psLevel = &msTop;
	for (iCurrent = 7; iCurrent > 0; iCurrent--)
	{
		ucCurrent = pvOI[iCurrent];
		psLevel = psLevel->apsLevels[ucCurrent];
		if (!psLevel)
		{
			return NULL;
		}
	}
	ucCurrent = pvOI[iCurrent];
	return psLevel->apsLevels[ucCurrent];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexes::Add(OIndex OI, void* pvMemory)
{
	unsigned char	ucCurrent;
	int				iCurrent;
	unsigned char*	pvOI;
	SIndexedLevel*	psLevel;
	SIndexedLevel*	psNewLevel;

	pvOI = (unsigned char*)&OI;
	psLevel = &msTop;
	for (iCurrent = 7; iCurrent > 0; iCurrent--)
	{
		ucCurrent = pvOI[iCurrent];
		psNewLevel = psLevel->apsLevels[ucCurrent];
		if (!psNewLevel)
		{
			psNewLevel = mcLevels.Add();
			psNewLevel->Init();
			psLevel->apsLevels[ucCurrent] = psNewLevel;
		}
		psLevel = psNewLevel;
	}
	ucCurrent = pvOI[iCurrent];
	psLevel->apsLevels[ucCurrent] = (SIndexedLevel*)pvMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexes::Remove(OIndex OI)
{
	unsigned char		ucCurrent;
	int					iCurrent;
	unsigned char*		pvOI;
	SIndexedLevel*		apsLevel[8];
	SIndexedLevel*		psLevel;

	pvOI = (unsigned char*)&OI;
	psLevel = &msTop;
	for (iCurrent = 7; iCurrent >= 0; iCurrent--)
	{
		ucCurrent = pvOI[iCurrent];
		psLevel = psLevel->apsLevels[ucCurrent];
		apsLevel[iCurrent] = psLevel;
		if (!psLevel)
		{
			return FALSE;
		}
	}

	for (iCurrent = 1; iCurrent <= 7; iCurrent++)
	{
		ucCurrent = pvOI[iCurrent-1];
		apsLevel[iCurrent]->apsLevels[ucCurrent] = NULL;

		if (apsLevel[iCurrent]->IsEmpty())
		{
			mcLevels.Remove(apsLevel[iCurrent]);
		}
		else
		{
			return TRUE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CIndexes::TestNumLevels(void)
{
	return mcLevels.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CIndexes::TestByteSize(void)
{
	return mcLevels.ByteSize();
}

