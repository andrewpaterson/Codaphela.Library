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


struct SIndexedLevel
{
	(SIndexedLevel*)	apsLevels[256];

	void Init(void);
	BOOL IsEmpty(void);
};


typedef CFreeList<SIndexedLevel>	CFreeListIndexedObjectsLevel;


class CIndexes
{
	CFreeListIndexedObjectsLevel	mcLevels;
	SIndexedLevel					msTop;

public:
	void			Init(int iChunkSize);
	void			Kill(void);
	void*			Get(OIndex oi);
	void			Add(OIndex oi, void* pvMemory);
	BOOL			Remove(OIndex oi);

	int				TestNumLevels(void);
	unsigned int	TestByteSize(void);
};


#endif // __INDEXES_H__

