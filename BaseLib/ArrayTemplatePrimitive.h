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
class CArrayTemplatePrimitive : public CArrayTemplate<M>
{
public:
	M 		GetValue(int iElementPos);
	M		SafeGetValue(int iElementPos);
	M 		operator[](int iElementPos);
	void	SetValue(int iElementPos, M iElement);
	void 	Add(M iElement);
	void 	Add(M* paElements, int iNumElements);
	void	InsertAt(M iElement, int iIndex);
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
	void	Intersect(CArrayTemplatePrimitive<M>* pcArray1, CArrayTemplatePrimitive<M>* pcArray2);
	BOOL	IsSorted(void);
	int		FindFinalContiguousInSorted(void);
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
M CArrayTemplatePrimitive<M>::GetValue(int iElementPos)
{
	return (*(CArrayTemplate<M>::Get(iElementPos)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M CArrayTemplatePrimitive<M>::operator[](int iElementPos)
{
	return (*(CArrayTemplate<M>::Get(iElementPos)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M CArrayTemplatePrimitive<M>::SafeGetValue(int iElementPos)
{
	if ((iElementPos >= 0) && (iElementPos < this->miUsedElements))
	{
		return ((M*)this->mpvArray)[iElementPos];
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::SetValue(int iElementPos, M iElement)
{
	((M*)this->mpvArray)[iElementPos] = iElement;
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
void CArrayTemplatePrimitive<M>::Add(M* paElements, int iNumElements)
{
	int		i;
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
void CArrayTemplatePrimitive<M>::InsertAt(M iElement, int iIndex)
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
	int	i;

	for (i = 0; i < this->miUsedElements; i++)
	{
		((M*)this->mpvArray)[i] = iValue;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayTemplatePrimitive<M>::SetArrayValues(M iStartValue, M iIncrement)
{
	int	i;

	for (i = 0; i < this->miUsedElements; i++)
	{
		((M*)this->mpvArray)[i] = iStartValue;
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
int CArrayTemplatePrimitive<M>::Find(M iValue)
{
	int	i;

	for (i = 0; i < this->miUsedElements; i++)
	{
		if (((M*)this->mpvArray)[i] == iValue)
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
int CArrayTemplatePrimitive<M>::AddIfUnique(M iValue)
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
void CArrayTemplatePrimitive<M>::Swap(int iIndex1, int iIndex2)
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
template<class M>
void CArrayTemplatePrimitive<M>::InsertIntoSorted(M iElement, BOOL bOverwriteExisting)
{
	int		iPos;
	BOOL	bExists;

	bExists = this->FindInSorted(&iElement, ComparePrimitive<M>, &iPos);
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
BOOL CArrayTemplatePrimitive<M>::RemoveFromSorted(M iElement)
{
	int		iPos;
	BOOL	bExists;

	bExists = this->FindInSorted(&iElement, ComparePrimitive<M>, &iPos);
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
BOOL CArrayTemplatePrimitive<M>::RemoveDuplicatesFromSorted(void)
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
            this->Resize(iWrite);
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
BOOL CArrayTemplatePrimitive<M>::IsSorted(void)
{
	int		i;
	M		iValue;
	M		iLast;

	if (this->miUsedElements > 1)
	{
		iLast = GetValue(0);

		for (i = 1; i < this->miUsedElements; i++)
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
	else
	{
		return TRUE;
	}
}



//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
int CArrayTemplatePrimitive<M>::FindFinalContiguousInSorted(void)
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

