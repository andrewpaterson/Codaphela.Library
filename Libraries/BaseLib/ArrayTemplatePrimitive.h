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
#ifndef __ARRAY_TEMPLATE_PRIMITIVE_H__
#define __ARRAY_TEMPLATE_PRIMITIVE_H__
#include <stdarg.h>
#include "ArrayTemplate.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int ComparePrimitive(const void* arg1, const void* arg2)
{
	if ((*((M*)arg1)) < (*((M*)arg2)))
	{
		return -1;
	}
	if ((*((M*)arg1)) > (*((M*)arg2)))
	{
		return 1;
	}
	return 0;
}


//CArrayTemplatePrimitive<M> (an array of numbers counted in size, a CArrayTemplate<M>) and
//CArrayLargeTemplatePrimitive<M> (counted in lsize, a CArrayLargeTemplate<M>) are this, written once: TEMPLATE is the
//class each derives from.
template<class M, class TEMPLATE>
class CArrayTemplatePrimitiveBase : public TEMPLATE
{
protected:
	using TEMPLATE::miUsedElements;
	using TEMPLATE::mpvArray;

public:
	typedef typename TEMPLATE::Count	Count;

	using TEMPLATE::FindInSorted;
	using TEMPLATE::Resize;
	using TEMPLATE::SetUsedElements;

	M		GetValue(Count iIndex);
	M		SafeGetValue(Count iIndex, M iNotFound);
	M		operator[](Count iIndex);
	void	SetValue(Count iIndex, M iElement);
	void	Add(M iElement);
	void	Add(M* paElements, Count iNumElements);
	void	InsertAt(M iElement, Count iIndex);
	void	SetArrayValues(M iValue);
	void	SetArrayValues(M iStart, M iIncrement);
	void	QuickSort(void);
	void	BubbleSort(void);
	Count	Find(M iValue);
	Count	AddIfUnique(M iValue);
	void	Swap(Count iIndex1, Count iIndex2);
	void	InsertIntoSorted(M iElement, bool bOverwriteExisting);
	template<class F>
	void	InsertInOrder(M iElement, F* psOrder, Count iFirst = 0);
	bool	RemoveFromSorted(M iElement);
	bool	RemoveDuplicatesFromSorted(void);
	void	MakeUnique(void);
	void	Intersect(CArrayTemplatePrimitiveBase<M, TEMPLATE>* pcArray1, CArrayTemplatePrimitiveBase<M, TEMPLATE>* pcArray2);
	bool	IsSorted(void);
	Count	FindFinalContiguousInSorted(void);
	M		Pop(void);
	void	Push(M iElement);

	void	AddRemap(Count iElementPos, M iRemapNum);
	void	AddList(M iStop, ...);
	void	RemoveRemap(Count iElementPos);
};




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
M CArrayTemplatePrimitiveBase<M, TEMPLATE>::GetValue(Count iIndex)
{
	return (*(TEMPLATE::Get(iIndex)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
M CArrayTemplatePrimitiveBase<M, TEMPLATE>::operator[](typename TEMPLATE::Count iIndex)
{
	return (*(TEMPLATE::Get(iIndex)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
M CArrayTemplatePrimitiveBase<M, TEMPLATE>::SafeGetValue(Count iIndex, M iNotFound)
{
	if (iIndex < miUsedElements)
	{
		return ((M*)mpvArray)[iIndex];
	}
	return iNotFound;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::SetValue(Count iIndex, M iElement)
{
	((M*)mpvArray)[iIndex] = iElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::Add(M iElement)
{
	M*	pvTemp;

	pvTemp = TEMPLATE::Add();
	*pvTemp = iElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::Add(M* paElements, Count iNumElements)
{
	typename TEMPLATE::Count	i;
	M*							pvTemp;

	for (i = 0; i < iNumElements; i++)
	{
		pvTemp = TEMPLATE::Add();
		*pvTemp = paElements[i];
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::InsertAt(M iElement, Count iIndex)
{
	M*	pvTemp;

	pvTemp = TEMPLATE::InsertAt(iIndex);
	if (pvTemp)
	{
		*pvTemp = iElement;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::SetArrayValues(M iValue)
{
	typename TEMPLATE::Count	i;

	for (i = 0; i < miUsedElements; i++)
	{
		((M*)mpvArray)[i] = iValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::SetArrayValues(M iStartValue, M iIncrement)
{
	typename TEMPLATE::Count	i;

	for (i = 0; i < miUsedElements; i++)
	{
		((M*)mpvArray)[i] = iStartValue;
		iStartValue += iIncrement;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::QuickSort(void)
{
	TEMPLATE::QuickSort(&ComparePrimitive<M>);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::BubbleSort(void)
{
	TEMPLATE::BubbleSort(&ComparePrimitive<M>);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
typename TEMPLATE::Count CArrayTemplatePrimitiveBase<M, TEMPLATE>::Find(M iValue)
{
	typename TEMPLATE::Count	i;

	for (i = 0; i < miUsedElements; i++)
	{
		if (((M*)mpvArray)[i] == iValue)
		{
			return i;
		}
	}
	return TEMPLATE::SizePolicy::NotFound;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
typename TEMPLATE::Count CArrayTemplatePrimitiveBase<M, TEMPLATE>::AddIfUnique(M iValue)
{
	typename TEMPLATE::Count iPos;

	iPos = Find(iValue);
	if (iPos == TEMPLATE::SizePolicy::NotFound)
	{
		Add(iValue);
	}
	return iPos;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::Swap(Count iIndex1, Count iIndex2)
{
	M*		piElement1;
	M*		piElement2;
	M		iTemp;

	piElement1 = this->Get(iIndex1);
	piElement2 = this->Get(iIndex2);
	iTemp = (*piElement1);
	(*piElement1) = (*piElement2);
	(*piElement2) = iTemp;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::InsertIntoSorted(M iElement, bool bOverwriteExisting)
{
	typename TEMPLATE::Count	iPos;
	bool						bExists;

	bExists = FindInSorted(&iElement, ComparePrimitive<M>, &iPos);
	if (iPos < miUsedElements)
	{
		if (!bExists)
		{
			InsertAt(iElement, iPos);
		}
		else
		{
			if (bOverwriteExisting)
			{
				TEMPLATE::Set(iPos, &iElement);
			}
			else
			{
				InsertAt(iElement, iPos);
			}
		}
	}
	else
	{
		Add(iElement);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
// Inserts iElement before the first element from iFirst on that it		//
// goes before (psOrder->IsBefore(iElement, iOther) is true), or at the	//
// end.  It goes after the elements it isn't before, its equals among	//
// them, so elements inserted one by one keep their order.				//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
template<class F>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::InsertInOrder(M iElement, F* psOrder, Count iFirst)
{
	typename TEMPLATE::Count	i;

	for (i = iFirst; i < miUsedElements; i++)
	{
		if (psOrder->IsBefore(iElement, GetValue(i)))
		{
			break;
		}
	}
	InsertAt(iElement, i);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
bool CArrayTemplatePrimitiveBase<M, TEMPLATE>::RemoveFromSorted(M iElement)
{
	typename TEMPLATE::Count	iPos;
	bool						bExists;

	bExists = FindInSorted(&iElement, ComparePrimitive<M>, &iPos);
	if (bExists)
	{
		this->RemoveAt(iPos, true);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
bool CArrayTemplatePrimitiveBase<M, TEMPLATE>::RemoveDuplicatesFromSorted(void)
{
	typename TEMPLATE::Count	i;
	M							iValue;
	M							iCurrent;
	bool						bAnyRemoved;
	typename TEMPLATE::Count	iWrite;

	bAnyRemoved = false;
	if (miUsedElements > 0)
	{
		iCurrent = GetValue(0);
		iWrite = 1;
		for (i = 1; i < miUsedElements; i++)
		{
			iValue = GetValue(i);
			if (iValue != iCurrent)
			{
				SetValue(iWrite, iValue);
				iCurrent = iValue;
				iWrite++;
			}
			else
			{
				bAnyRemoved = true;
			}
		}

		if (bAnyRemoved)
		{
			Resize(iWrite);
		}
	}
	return bAnyRemoved;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::MakeUnique(void)
{
	QuickSort();
	RemoveDuplicatesFromSorted();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::Intersect(CArrayTemplatePrimitiveBase<M, TEMPLATE>* pcArray1, CArrayTemplatePrimitiveBase<M, TEMPLATE>* pcArray2)
{
	typename TEMPLATE::Count	i;
	typename TEMPLATE::Count	j;
	M							x;
	M							y;

	//This assumes that both arrays contain only unique values.
	for (i = 0; i < pcArray1->NumElements(); i++)
	{
		x = pcArray1->GetValue(i);

		for (j = 0; j < pcArray2->NumElements(); j++)
		{
			y = pcArray2->GetValue(j);

			if (x == y)
			{
				Add(x);
				break;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
bool CArrayTemplatePrimitiveBase<M, TEMPLATE>::IsSorted(void)
{
	typename TEMPLATE::Count	i;
	M							iValue;
	M							iLast;

	if (miUsedElements > 1)
	{
		iLast = GetValue(0);

		for (i = 1; i < miUsedElements; i++)
		{
			iValue = GetValue(i);
			if (i != 0)
			{
				if (iValue < iLast)
				{
					return false;
				}
			}
			iLast = iValue;
		}
		return true;
	}
	else
	{
		return true;
	}
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
typename TEMPLATE::Count CArrayTemplatePrimitiveBase<M, TEMPLATE>::FindFinalContiguousInSorted(void)
{
	typename TEMPLATE::Count	i;
	M							iValue;

	if (miUsedElements == 0)
	{
		return TEMPLATE::SizePolicy::NotFound;
	}

	for (i = 0; i < miUsedElements; i++)
	{
		iValue = GetValue(i);
		if ((M)i != iValue)
		{
			if (i > 0)
			{
				return i - 1;
			}
			else
			{
				return TEMPLATE::SizePolicy::NotFound;
			}
		}
	}
	return miUsedElements - 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
M CArrayTemplatePrimitiveBase<M, TEMPLATE>::Pop(void)
{
	M	i;

	//Returns zero if empty.
	TEMPLATE::Pop(&i);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::Push(M iElement)
{
	Add(iElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::AddRemap(Count iElementPos, M iRemapNum)
{
	M*							pi;
	typename TEMPLATE::Count	iIndex;
	typename TEMPLATE::Count	iNumToAdd;
	typename TEMPLATE::Count	iMinusOne;

	if (miUsedElements > iElementPos)
	{
		pi = TEMPLATE::Get(iElementPos);
		(*pi) = iRemapNum;
	}
	else
	{
		iMinusOne = TEMPLATE::SizePolicy::Max;
		iNumToAdd = (iElementPos - miUsedElements);

		for (iIndex = 0; iIndex < iNumToAdd; iIndex++)
		{
			Add((M)iMinusOne);
		}
		Add(iRemapNum);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::RemoveRemap(Count iElementPos)
{
	M*							pi;
	typename TEMPLATE::Count	iIndex;
	typename TEMPLATE::Count	iCutDown;

	if (iElementPos < miUsedElements)
	{
		pi = TEMPLATE::Get(iElementPos);
		(*pi) = -1;
	}

	iCutDown = miUsedElements;
	iIndex = miUsedElements;
	if (iIndex != 0)
	{
		do
		{
			iIndex--;
			pi = TEMPLATE::Get(iIndex);
			if (*pi == (M)-1)
			{
				iCutDown = iIndex;
			}
			else
			{
				break;
			}
			if (iIndex == 0)
			{
				break;
			}
		}
		while (iIndex != 0);
	}
	if (iCutDown != miUsedElements)
	{
		SetUsedElements(iCutDown);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M, class TEMPLATE>
void CArrayTemplatePrimitiveBase<M, TEMPLATE>::AddList(M iStop, ...)
{
	va_list		vaMarker;
	M			iValue;

	va_start(vaMarker, iStop);
	iValue = va_arg(vaMarker, M);
	while (iValue != iStop)
	{
		Add(iValue);
		iValue = va_arg(vaMarker, M);
	}
	va_end(vaMarker);
}


//The names the arrays are known by: a CArrayTemplatePrimitive<M> is a CArrayTemplate<M>.
template<class M> class CArrayTemplatePrimitive : public CArrayTemplatePrimitiveBase<M, CArrayTemplate<M> > {};
template<class M> class CArrayLargeTemplatePrimitive : public CArrayTemplatePrimitiveBase<M, CArrayLargeTemplate<M> > {};


#endif // __ARRAY_TEMPLATE_PRIMITIVE_H__

