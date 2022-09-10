/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#ifndef __ARRAY_TYPED_POINTER_H__
#define __ARRAY_TYPED_POINTER_H__
#include "ArrayTemplate.h"
#include "Define.h"


struct SIntAndPointer
{
	void*	pvData;
	int		iValue;  //Arbitrary type values... no need to be a registered type.
};


class CArrayIntAndPointer : public CArrayTemplate<SIntAndPointer>
{
public:
	void 	Add(void* pvData, int iInt);
	void 	AddIfUnique(void* pvData, int iInt);
	bool 	Get(int iElementPos, void** pvData, int* iType);
	void*	GetPtr(int iElementPos);
	void*	SafeGetPtr(int iElementPos);
	int		GetType(int iElementPos);
	int		SafeGetType(int iElementPos);
	void	Set(int iElementPos, void* pvData, int iInt);
	void	Remove(void* pv);
	void*	InsertIntoSorted(DataCompare fCompare, void* pvElement, int iInt);
	bool	FindInSorted(void* pvElement, DataCompare fCompare, int* piIndex);
	bool	BinarySearch(void* pvData, int iLeft, int iRight, DataCompare fCompare, int* piIndex);
};


#endif // !__ARRAY_TYPED_POINTER_H__

