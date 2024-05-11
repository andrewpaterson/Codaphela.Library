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
#ifndef __ARRAY_PRIMITIVE_TEMPLATE_H__
#define __ARRAY_PRIMITIVE_TEMPLATE_H__
#include <stdarg.h>
#include "ArrayTemplate.h"


template<class M>
class CArrayTemplatePrimitive : public CArrayTemplate<M>
{
public:
	M 		GetValue(size iElementPos);
	M		SafeGetValue(size iElementPos);
	M 		operator[](size iElementPos);
	void	SetValue(size iElementPos, M iElement);
	void 	Add(M iElement);
	void 	Add(M* paElements, size iNumElements);
	void	InsertAt(M iElement, size iIndex);
	void 	SetArrayValues(M iValue);
	void	SetArrayValues(M iStart, M iIncrement);
	void	QuickSort(void);
	void	BubbleSort(void);
	size 	Find(M iValue);
	size 	AddIfUnique(M iValue);
	void	Swap(size iIndex1, size iIndex2);
	void	InsertIntoSorted(M iElement, bool bOverwriteExisting);
	bool	RemoveFromSorted(M iElement);
	bool	RemoveDuplicatesFromSorted(void);
	void	MakeUnique(void);
	void	Intersect(CArrayTemplatePrimitive<M>* pcArray1, CArrayTemplatePrimitive<M>* pcArray2);
	bool	IsSorted(void);
	size	FindFinalContiguousInSorted(void);
	M		Pop(void);
	void 	Push(M iElement);
};


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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M CArrayTemplatePrimitive<M>::GetValue(size iElementPos)
{
	return (*(CArrayTemplate<M>::Get(iElementPos)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M CArrayTemplatePrimitive<M>::operator[](size iElementPos)
{
	return (*(CArrayTemplate<M>::Get(iElementPos)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M CArrayTemplatePrimitive<M>::SafeGetValue(size iElementPos)
{
	if (iElementPos < miUsedElements)
	{
		return ((M*)mpvArray)[iElementPos];
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::SetValue(size iElementPos, M iElement)
{
	((M*)mpvArray)[iElementPos] = iElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::Add(M iElement)
{
	M*	pvTemp;

	pvTemp = CArrayTemplate<M>::Add();
	*pvTemp = iElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::Add(M* paElements, size iNumElements)
{
	size	i;
	M*		pvTemp;

	for (i = 0; i < iNumElements; i++)
	{
		pvTemp = CArrayTemplate<M>::Add();
		*pvTemp = paElements[i];
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::InsertAt(M iElement, size iIndex)
{
	M*	pvTemp;

	pvTemp = CArrayTemplate<M>::InsertAt(iIndex);
	if (pvTemp)
	{
		*pvTemp = iElement;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::SetArrayValues(M iValue)
{
	size	i;

	for (i = 0; i < miUsedElements; i++)
	{
		((M*)mpvArray)[i] = iValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::SetArrayValues(M iStartValue, M iIncrement)
{
	size	i;

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
template<class M>
void CArrayTemplatePrimitive<M>::QuickSort(void)
{
	CArrayTemplate<M>::QuickSort(&ComparePrimitive<M>);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::BubbleSort(void)
{
	CArrayTemplate<M>::BubbleSort(&ComparePrimitive<M>);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplatePrimitive<M>::Find(M iValue)
{
	size	i;

	for (i = 0; i < miUsedElements; i++)
	{
		if (((M*)mpvArray)[i] == iValue)
		{
			return i;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
size CArrayTemplatePrimitive<M>::AddIfUnique(M iValue)
{
	size iPos;

	iPos = Find(iValue);
	if (iPos == ARRAY_ELEMENT_NOT_FOUND)
	{
		Add(iValue);
	}
	return iPos;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::Swap(size iIndex1, size iIndex2)
{
	M*		piElement1;
	M*		piElement2;
	M		iTemp;

	piElement1 = Get(iIndex1);
	piElement2 = Get(iIndex2);
	iTemp = (*piElement1);
	(*piElement1) = (*piElement2);
	(*piElement2) = iTemp;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::InsertIntoSorted(M iElement, bool bOverwriteExisting)
{
	size	iPos;
	bool	bExists;

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
				CArrayTemplate<M>::Set(iPos, &iElement);
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
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplatePrimitive<M>::RemoveFromSorted(M iElement)
{
	size	iPos;
	bool	bExists;

	bExists = FindInSorted(&iElement, ComparePrimitive<M>, &iPos);
	if (bExists)
	{
		RemoveAt(iPos, true);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayTemplatePrimitive<M>::RemoveDuplicatesFromSorted(void)
{
	size	i;
	M		iValue;
	M		iCurrent;
	bool	bAnyRemoved;
	size  iWrite;

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
template<class M>
void CArrayTemplatePrimitive<M>::MakeUnique(void)
{
	QuickSort();
	RemoveDuplicatesFromSorted();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::Intersect(CArrayTemplatePrimitive<M>* pcArray1, CArrayTemplatePrimitive<M>* pcArray2)
{
	size	i;
	size	j;
	M		x;
	M		y;

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
template<class M>
bool CArrayTemplatePrimitive<M>::IsSorted(void)
{
	size	i;
	M		iValue;
	M		iLast;

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
template<class M>
size CArrayTemplatePrimitive<M>::FindFinalContiguousInSorted(void)
{
	size	i;
	M		iValue;

	if (miUsedElements == 0)
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}

	for (i = 0; i < miUsedElements; i++)
	{
		iValue = GetValue(i);
		if (i != iValue)
		{
			if (i > 1)
			{
				return i - 1;
			}
			else
			{
				return ARRAY_ELEMENT_NOT_FOUND;
			}
		}
	}
	return miUsedElements - 1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M CArrayTemplatePrimitive<M>::Pop(void)
{
	M	i;

	//Returns zero if empty.
	CArrayTemplate<M>::Pop(&i);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::Push(M iElement)
{
	Add(iElement);
}


#endif // __ARRAY_PRIMITIVE_TEMPLATE_H__

