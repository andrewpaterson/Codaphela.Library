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
#ifndef __INDEXES_H__
#define __INDEXES_H__
#include "BaseLib/FreeList.h"
#include "IndexedGeneral.h"

//OIndex is 64 bits equals 8 chars (of combinations each).
#define INDEXED_LEVELS_IN_LEVEL	256
#define MAX_INDEXED_LEVEL_DEPTH	7


struct SIndexedLevel
{
	(SIndexedLevel*)	apsLevels[INDEXED_LEVELS_IN_LEVEL];  //Points

	void Init(void);
	BOOL IsEmpty(void);
};


typedef CFreeList<SIndexedLevel>	CFreeListIndexedObjectsLevel;


struct SIndexesIterator
{
	(SIndexedLevel*)	apsLevels[MAX_INDEXED_LEVEL_DEPTH+1];
	int					aiIndex[MAX_INDEXED_LEVEL_DEPTH+1];
	OIndex				oi;

	void Init(SIndexedLevel* psTop);
};


class CIndexes
{
protected:
	CFreeListIndexedObjectsLevel	mcLevels;
	SIndexedLevel					msTop;

public:
	void			Init(int iChunkSize);
	void			Kill(void);
	void*			Get(OIndex oi);
	BOOL			Add(OIndex oi, void* pvMemory);
	BOOL			AddOverwriteExisting(OIndex oi, void* pvMemory, void** pvExisting);
	BOOL			Remove(OIndex oi);

	int				TestNumLevels(void);
	unsigned int	TestByteSize(void);
	BOOL			TestTopIsEmpty(void);

	OIndex			NumIndexed(void);
	OIndex			StartIteration(SIndexesIterator* psIter);
	OIndex			Iterate(SIndexesIterator* psIter);

protected:
	OIndex			RecurseNumIndexed(SIndexedLevel* psLevel, int iLevel);
	SIndexedLevel*	CreateLevels(OIndex oi);
	OIndex			BackIterate(int iInitialLevel, SIndexesIterator* psIter);
};


#endif // __INDEXES_H__

