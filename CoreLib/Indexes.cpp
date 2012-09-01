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

	for (i = 0; i < INDEXED_LEVELS_IN_LEVEL; i++)
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
void* CIndexes::Get(OIndex oi)
{
	unsigned char	ucCurrent;
	int				iCurrent;
	unsigned char*	pvOI;
	SIndexedLevel*	psLevel;

	pvOI = (unsigned char*)&oi;
	psLevel = &msTop;
	for (iCurrent = MAX_INDEXED_LEVEL_DEPTH; iCurrent > 0; iCurrent--)
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
SIndexedLevel* CIndexes::CreateLevels(OIndex oi)
{
	unsigned char	ucCurrent;
	int				iCurrent;
	unsigned char*	pvOI;
	SIndexedLevel*	psLevel;
	SIndexedLevel*	psNewLevel;

	pvOI = (unsigned char*)&oi;
	psLevel = &msTop;
	for (iCurrent = MAX_INDEXED_LEVEL_DEPTH; iCurrent > 0; iCurrent--)
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
	return psLevel;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexes::Add(OIndex oi, void* pvMemory)
{
	unsigned char	ucCurrent;
	SIndexedLevel*	psLevel;

	psLevel = CreateLevels(oi);
	ucCurrent = *((unsigned char*)&oi);
	if (psLevel->apsLevels[ucCurrent] == NULL)
	{
		psLevel->apsLevels[ucCurrent] = (SIndexedLevel*)pvMemory;
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexes::AddOverwriteExisting(OIndex oi, void* pvMemory, void** pvExisting)
{
	unsigned char	ucCurrent;
	SIndexedLevel*	psLevel;

	psLevel = CreateLevels(oi);
	ucCurrent = *((unsigned char*)&oi);
	if (psLevel->apsLevels[ucCurrent] == NULL)
	{
		psLevel->apsLevels[ucCurrent] = (SIndexedLevel*)pvMemory;
		return TRUE;
	}
	else
	{
		*pvExisting = psLevel->apsLevels[ucCurrent];
		psLevel->apsLevels[ucCurrent] = (SIndexedLevel*)pvMemory;
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexes::Remove(OIndex oi)
{
	unsigned char		ucCurrent;
	int					iCurrent;
	unsigned char*		pvOI;
	SIndexedLevel*		apsLevel[MAX_INDEXED_LEVEL_DEPTH+1];
	SIndexedLevel*		psLevel;

	pvOI = (unsigned char*)&oi;
	psLevel = &msTop;
	for (iCurrent = MAX_INDEXED_LEVEL_DEPTH; iCurrent >= 0; iCurrent--)
	{
		ucCurrent = pvOI[iCurrent];
		psLevel = psLevel->apsLevels[ucCurrent];
		apsLevel[iCurrent] = psLevel;
		if (!psLevel)
		{
			return FALSE;
		}
	}

	for (iCurrent = 1; iCurrent <= MAX_INDEXED_LEVEL_DEPTH; iCurrent++)
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexes::NumIndexed(void)
{
	return RecurseNumIndexed(&msTop, 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexes::RecurseNumIndexed(SIndexedLevel* psLevel, int iLevel)
{
	int		i;
	OIndex	iCount;

	iCount = 0;
	if (iLevel < MAX_INDEXED_LEVEL_DEPTH)
	{
		for (i = 0; i < INDEXED_LEVELS_IN_LEVEL; i++)
		{
			if (psLevel->apsLevels[i] != NULL)
			{
				iCount += RecurseNumIndexed(psLevel->apsLevels[i], iLevel+1);
			}
		}
	}
	else
	{
		for (i = 0; i < INDEXED_LEVELS_IN_LEVEL; i++)
		{
			if (psLevel->apsLevels[i] != NULL)
			{
				iCount++;
			}
		}
	}
	return iCount;
}

