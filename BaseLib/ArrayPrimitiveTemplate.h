/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
class CArrayPrimitiveTemplate : public CArrayTemplate<M>
{
public:
	M 		GetValue(int iElementPos);
	M		SafeGetValue(int iElementPos);
	M 		operator[](int iElementPos);
	void	SetValue(int iElementPos, M iElement);
	void 	Add(M iElement);
	void	InsertAt(M iElement, int iIndex);
	void	AddList(M iStop, ...);
	void 	SetArrayValues(M iValue);
	void	SetArrayValues(M iStart, M iIncrement);
	void	QuickSort(void);
	void	BubbleSort(void);
	int 	Find(M iValue);
	int 	AddIfUnique(M iValue);
	void	Swap(int iIndex1, int iIndex2);
	void	InsertIntoSorted(M iElement, BOOL bOverwriteExisting);
	BOOL	RemoveFromSorted(M iElement);
	BOOL	RemoveDuplicatesFromSorted(void);
	void	MakeUnique(void);
	void	Intersect(CArrayPrimitiveTemplate<M>* pcArray1, CArrayPrimitiveTemplate<M>* pcArray2);
	BOOL	IsSorted(void);
	int		FindFinalContiguousInSorted(void);
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
M CArrayPrimitiveTemplate<M>::GetValue(int iElementPos)
{
	return (*(CArrayTemplate<M>::Get(iElementPos)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M CArrayPrimitiveTemplate<M>::operator[](int iElementPos)
{
	return (*(CArrayTemplate<M>::Get(iElementPos)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M CArrayPrimitiveTemplate<M>::SafeGetValue(int iElementPos)
{
	if ((iElementPos >= 0) && (iElementPos < this->miUsedElements))
	{
		return mpvArray[iElementPos];
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayPrimitiveTemplate<M>::SetValue(int iElementPos, M iElement)
{
	mpvArray[iElementPos] = iElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayPrimitiveTemplate<M>::Add(M iElement)
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
void CArrayPrimitiveTemplate<M>::InsertAt(M iElement, int iIndex)
{
	M*	pvTemp;

	pvTemp = CArrayTemplate<M>::InsertAt(iIndex);
	*pvTemp = iElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayPrimitiveTemplate<M>::AddList(M iStop, ...)
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


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayPrimitiveTemplate<M>::SetArrayValues(M iValue)
{
	int	i;

	for (i = 0; i < this->miUsedElements; i++)
	{
		this->mpvArray[i] = iValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayPrimitiveTemplate<M>::SetArrayValues(M iStartValue, M iIncrement)
{
	int	i;

	for (i = 0; i < this->miUsedElements; i++)
	{
		this->mpvArray[i] = iStartValue;
		iStartValue += iIncrement;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayPrimitiveTemplate<M>::QuickSort(void)
{
	CArrayTemplate::QuickSort(&ComparePrimitive<M>);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayPrimitiveTemplate<M>::BubbleSort(void)
{
	CArrayTemplate::BubbleSort(&ComparePrimitive<M>);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayPrimitiveTemplate<M>::Find(M iValue)
{
	int	i;

	for (i = 0; i < this->miUsedElements; i++)
	{
		if (this->mpvArray[i] == iValue)
		{
			return i;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayPrimitiveTemplate<M>::AddIfUnique(M iValue)
{
	int iPos;

	iPos = Find(iValue);
	if (iPos == -1)
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
void CArrayPrimitiveTemplate<M>::Swap(int iIndex1, int iIndex2)
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
void CArrayPrimitiveTemplate<M>::InsertIntoSorted(M iElement, BOOL bOverwriteExisting)
{
	int		iPos;
	BOOL	bExists;

	bExists = FindInSorted(&iElement, ComparePrimitive<M>, &iPos);
	if (iPos < this->miUsedElements)
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
BOOL CArrayPrimitiveTemplate<M>::RemoveFromSorted(M iElement)
{
	int		iPos;
	BOOL	bExists;

	bExists = FindInSorted(&iElement, ComparePrimitive<M>, &iPos);
	if (bExists)
	{
		this->RemoveAt(iPos, TRUE);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayPrimitiveTemplate<M>::RemoveDuplicatesFromSorted(void)
{
	int		i;
	M		iValue;
	M		iCurrent;
	BOOL	bAnyRemoved;
	int     iWrite;

	bAnyRemoved = FALSE;
    if (this->miUsedElements > 0)
    {
        iCurrent = GetValue(0);
        iWrite = 1;
        for (i = 1; i < this->miUsedElements; i++)
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
                bAnyRemoved = TRUE;
            }
        }

        if (bAnyRemoved)
        {
            this->GrowToNumElements(iWrite);
        }
    }
	return bAnyRemoved;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayPrimitiveTemplate<M>::MakeUnique(void)
{
	QuickSort();
	RemoveDuplicatesFromSorted();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayPrimitiveTemplate<M>::Intersect(CArrayPrimitiveTemplate<M>* pcArray1, CArrayPrimitiveTemplate<M>* pcArray2)
{
	int	i;
	int	j;
	M	x;
	M	y;

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
BOOL CArrayPrimitiveTemplate<M>::IsSorted(void)
{
	int		i;
	M		iValue;
	M		iLast;


	for (i = 0; i < this->miUsedElements; i++)
	{
		iValue = GetValue(i);
		if (i != 0)
		{
			if (iValue < iLast)
			{
				return FALSE;
			}
		}
		iLast = iValue;
	}
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayPrimitiveTemplate<M>::FindFinalContiguousInSorted(void)
{
	int		i;
	M		iValue;

	if (this->miUsedElements == 0)
	{
		return -1;
	}

	for (i = 0; i < this->miUsedElements; i++)
	{
		iValue = GetValue(i);
		if (i != iValue)
		{
			return i-1;
		}
	}
	return this->miUsedElements-1;
}


#endif // __ARRAY_PRIMITIVE_TEMPLATE_H__

