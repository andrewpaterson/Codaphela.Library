/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifndef __ARRAY_INT_AND_POINTER_H__
#define __ARRAY_INT_AND_POINTER_H__
#include "ArrayTemplate.h"
#include "IntAndPointer.h"
#include "Define.h"


class CArrayIntAndPointer : public CArrayTemplate<SIntAndPointer>
{
public:
	void 	Add(void* pvData, uint uiType);
	void 	AddIfUnique(void* pvData, uint uiType);
	bool 	Get(size iElementPos, void** pvData, uint* uiType);
	void*	GetPtr(size iElementPos);
	void*	SafeGetPtr(size iElementPos);
	uint	GetType(size iElementPos);
	uint	SafeGetType(size iElementPos);
	void	Set(size iElementPos, void* pvData, uint uiType);
	void	Remove(void* pv);
	void*	InsertIntoSorted(DataCompare fCompare, void* pvElement, uint uiType);
	bool	FindInSorted(void* pvElement, DataCompare fCompare, size* piIndex);
	bool	BinarySearch(void* pvData, size iLeft, size iRight, DataCompare fCompare, size* piIndex);
};


#endif // __ARRAY_INT_AND_POINTER_H__

