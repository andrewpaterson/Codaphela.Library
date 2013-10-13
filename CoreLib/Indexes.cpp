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
#include "BaseLib/Logger.h"
#include "BaseLib/LogString.h"
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
void SIndexesIterator::Init(SIndexedLevel* psTop)
{
	int		i;

	apsLevels[0] = psTop;
	aiIndex[0] = 0;
	for (i = 1; i < MAX_INDEXED_LEVEL_DEPTH+1; i++)
	{
		apsLevels[i] = NULL;
		aiIndex[i] = -1;
	}

	oi = INVALID_O_INDEX;
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

	if (oi == INVALID_O_INDEX)
	{
		return NULL;
	}

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
	if (!psLevel)
	{
		gcLogger.Error("CIndexes::Add cannot create Level");
		return FALSE;
	}

	ucCurrent = *((unsigned char*)&oi);
	if (psLevel->apsLevels[ucCurrent] == NULL)
	{
		psLevel->apsLevels[ucCurrent] = (SIndexedLevel*)pvMemory;
		return TRUE;
	}
	else
	{
		CChars szOi;

		szOi.Init();
		szOi.Append(oi);
		gcLogger.Error2(__METHOD__, " Cannot add memory with index [", szOi.Text(), "].  It already exists.", NULL);
		szOi.Kill();
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

	apsLevel[1] = apsLevel[2] = apsLevel[3] = apsLevel[4] = apsLevel[5] = apsLevel[6] = apsLevel[7] = NULL;
	pvOI = (unsigned char*)&oi;  //if the index is one then pvOI[0] == 1 and pvOI[1..7] == 0.
	psLevel = &msTop;
	apsLevel[0] = &msTop;
	for (iCurrent = MAX_INDEXED_LEVEL_DEPTH-1; iCurrent >= 0; iCurrent--)
	{
		ucCurrent = pvOI[iCurrent+1];
		psLevel = psLevel->apsLevels[ucCurrent];
		apsLevel[MAX_INDEXED_LEVEL_DEPTH - iCurrent] = psLevel;
		if (!psLevel)
		{
			return FALSE;
		}
	}

	for (iCurrent = MAX_INDEXED_LEVEL_DEPTH; iCurrent >= 0; iCurrent--)
	{
		ucCurrent = pvOI[MAX_INDEXED_LEVEL_DEPTH - iCurrent];
		apsLevel[iCurrent]->apsLevels[ucCurrent] = NULL;

		if ((apsLevel[iCurrent]->IsEmpty() && iCurrent != 0))
		{
			if (!mcLevels.Remove(apsLevel[iCurrent]))
			{
				return FALSE;
			}
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
BOOL CIndexes::TestTopIsEmpty(void)
{
	return msTop.IsEmpty();
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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexes::StartIteration(SIndexesIterator* psIter)
{
	psIter->Init(&msTop);

	return BackIterate(0, psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexes::Iterate(SIndexesIterator* psIter)
{
	int				iLevel;
	int				iIndex;
	SIndexedLevel*	psLevel;
	unsigned char*	pvOI;
	OIndex			oi;

	pvOI = (unsigned char*)&psIter->oi;

	iLevel = MAX_INDEXED_LEVEL_DEPTH;
	psLevel = psIter->apsLevels[iLevel];
	for (iIndex = psIter->aiIndex[iLevel]+1; iIndex < INDEXED_LEVELS_IN_LEVEL; iIndex++)
	{
		psIter->aiIndex[iLevel] = iIndex;
		if (psLevel->apsLevels[iIndex] != NULL)
		{
			pvOI[MAX_INDEXED_LEVEL_DEPTH - iLevel] = (unsigned char)iIndex;
			return psIter->oi;
		}
	}
	psIter->aiIndex[iLevel] = iIndex;
	psIter->aiIndex[iLevel-1]++;

	for (iLevel = MAX_INDEXED_LEVEL_DEPTH; iLevel >= 0; iLevel--)
	{
		if (psIter->aiIndex[iLevel] >= INDEXED_LEVELS_IN_LEVEL)
		{
			pvOI[MAX_INDEXED_LEVEL_DEPTH - iLevel] = 0;
			psIter->apsLevels[iLevel] = NULL;
			psIter->aiIndex[iLevel] = -1;
		}
		else
		{
			oi = BackIterate(iLevel, psIter);
			if (oi != INVALID_O_INDEX)
			{
				return oi;
			}
			else
			{
				psIter->aiIndex[iLevel-1]++;
			}
		}
	}
	return INVALID_O_INDEX;	
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexes::BackIterate(int iInitialLevel, SIndexesIterator* psIter)
{
	int				iLevel;
	int				iIndex;
	SIndexedLevel*	psLevel;
	unsigned char*	pvOI;

	pvOI = (unsigned char*)&psIter->oi;

	for (iLevel = iInitialLevel; iLevel < MAX_INDEXED_LEVEL_DEPTH+1; iLevel++)
	{
		if (psIter->aiIndex[iLevel] == -1)
		{
			return INVALID_O_INDEX;
		}

		psLevel = psIter->apsLevels[iLevel];
		for (iIndex = psIter->aiIndex[iLevel]; iIndex < INDEXED_LEVELS_IN_LEVEL; iIndex++)
		{
			pvOI[MAX_INDEXED_LEVEL_DEPTH - iLevel] = (unsigned char)iIndex;
			psIter->aiIndex[iLevel] = iIndex;
			if (psLevel->apsLevels[iIndex] != NULL)
			{
				if (iLevel < MAX_INDEXED_LEVEL_DEPTH)
				{
					psIter->apsLevels[iLevel+1] = psLevel->apsLevels[iIndex];
					psIter->aiIndex[iLevel+1] = 0;
				}
				break;
			}
		}
	}
	return psIter->oi;
}

