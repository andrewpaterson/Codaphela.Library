/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __ARRAY_TRANSIENT_INDEXED_POINTER_H__
#define __ARRAY_TRANSIENT_INDEXED_POINTER_H__
#include "BaseLib/ArrayTemplate.h"
#include "TransactionIndexedData.h"


struct STransientIndexedPointer
{
	SIndexedMemory	sIndexedMemory;
	void*			pvCache;
	int				iFileIndex;
	int				iIndexInFile;

	void	Init(OIndex oi, unsigned int uiSize);
	BOOL	IsRemoved(void);
};


typedef CArrayTemplate<STransientIndexedPointer> __CArrayTransientIndexedPointer;


class CArrayTransientIndexedPointer : public __CArrayTransientIndexedPointer
{
private:
	SOIndexIndex	asIndexCache[4];
	int				miNext;

public:
	void						Init(int iChunkSize);
	void						Kill(void);

	STransientIndexedPointer*	Add(OIndex oi, int iIndex);
	BOOL						Get(OIndex oi, STransientIndexedPointer** ppsPointer, int* piIndex);
	
	BOOL						GetNoCacheTest(OIndex oi, STransientIndexedPointer** ppsPointer, int* piIndex);
	BOOL						GetDirect(int iIndex, STransientIndexedPointer** ppsPointer);
	void						ClearIndexCache(void);
	void						SetIndexCache(OIndex oi, int iIndex);
};


int CompareOIndexToTransientIndexedPointer(const void* pv1, const void* pv2);


#endif // __ARRAY_TRANSIENT_INDEXED_POINTER_H__

