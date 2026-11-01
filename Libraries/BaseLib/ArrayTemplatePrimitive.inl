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
//No include guard: ArrayTemplatePrimitive.inl is included once per array size (ArraySizeNatural.inl and
//ArraySizeLarge.inl define the names and types it is written with).

template<class M>
class ARRAY_TEMPLATE_PRIMITIVE : public ARRAY_TEMPLATE<M>
{
protected:
	using ARRAY_TEMPLATE<M>::miUsedElements;
	using ARRAY_TEMPLATE<M>::mpvArray;

public:
	using ARRAY_TEMPLATE<M>::FindInSorted;
	using ARRAY_TEMPLATE<M>::Resize;
	using ARRAY_TEMPLATE<M>::SetUsedElements;

	M 		GetValue(ASIZE iIndex);
	M		SafeGetValue(ASIZE iIndex, M iNotFound);
	M 		operator[](ASIZE iIndex);
	void	SetValue(ASIZE iIndex, M iElement);
	void 	Add(M iElement);
	void 	Add(M* paElements, ASIZE iNumElements);
	void	InsertAt(M iElement, ASIZE iIndex);
	void 	SetArrayValues(M iValue);
	void	SetArrayValues(M iStart, M iIncrement);
	void	QuickSort(void);
	void	BubbleSort(void);
	ASIZE 	Find(M iValue);
	ASIZE 	AddIfUnique(M iValue);
	void	Swap(ASIZE iIndex1, ASIZE iIndex2);
	void	InsertIntoSorted(M iElement, bool bOverwriteExisting);
	bool	RemoveFromSorted(M iElement);
	bool	RemoveDuplicatesFromSorted(void);
	void	MakeUnique(void);
	void	Intersect(ARRAY_TEMPLATE_PRIMITIVE<M>* pcArray1, ARRAY_TEMPLATE_PRIMITIVE<M>* pcArray2);
	bool	IsSorted(void);
	ASIZE	FindFinalContiguousInSorted(void);
	M		Pop(void);
	void 	Push(M iElement);

	void	AddRemap(ASIZE iElementPos, M iRemapNum);
	void 	AddList(M iStop, ...);
	void	RemoveRemap(ASIZE iElementPos);
};




//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M ARRAY_TEMPLATE_PRIMITIVE<M>::GetValue(ASIZE iIndex)
{
	return (*(ARRAY_TEMPLATE<M>::Get(iIndex)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M ARRAY_TEMPLATE_PRIMITIVE<M>::operator[](ASIZE iIndex)
{
	return (*(ARRAY_TEMPLATE<M>::Get(iIndex)));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
M ARRAY_TEMPLATE_PRIMITIVE<M>::SafeGetValue(ASIZE iIndex, M iNotFound)
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
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::SetValue(ASIZE iIndex, M iElement)
{
	((M*)mpvArray)[iIndex] = iElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::Add(M iElement)
{
	M*	pvTemp;

	pvTemp = ARRAY_TEMPLATE<M>::Add();
	*pvTemp = iElement;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::Add(M* paElements, ASIZE iNumElements)
{
	ASIZE	i;
	M*		pvTemp;

	for (i = 0; i < iNumElements; i++)
	{
		pvTemp = ARRAY_TEMPLATE<M>::Add();
		*pvTemp = paElements[i];
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::InsertAt(M iElement, ASIZE iIndex)
{
	M*	pvTemp;

	pvTemp = ARRAY_TEMPLATE<M>::InsertAt(iIndex);
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
void ARRAY_TEMPLATE_PRIMITIVE<M>::SetArrayValues(M iValue)
{
	ASIZE	i;

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
void ARRAY_TEMPLATE_PRIMITIVE<M>::SetArrayValues(M iStartValue, M iIncrement)
{
	ASIZE	i;

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
void ARRAY_TEMPLATE_PRIMITIVE<M>::QuickSort(void)
{
	ARRAY_TEMPLATE<M>::QuickSort(&ComparePrimitive<M>);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::BubbleSort(void)
{
	ARRAY_TEMPLATE<M>::BubbleSort(&ComparePrimitive<M>);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE_PRIMITIVE<M>::Find(M iValue)
{
	ASIZE	i;

	for (i = 0; i < miUsedElements; i++)
	{
		if (((M*)mpvArray)[i] == iValue)
		{
			return i;
		}
	}
	return ARRAY_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
ASIZE ARRAY_TEMPLATE_PRIMITIVE<M>::AddIfUnique(M iValue)
{
	ASIZE iPos;

	iPos = Find(iValue);
	if (iPos == ARRAY_NOT_FOUND)
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
void ARRAY_TEMPLATE_PRIMITIVE<M>::Swap(ASIZE iIndex1, ASIZE iIndex2)
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
void ARRAY_TEMPLATE_PRIMITIVE<M>::InsertIntoSorted(M iElement, bool bOverwriteExisting)
{
	ASIZE	iPos;
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
				ARRAY_TEMPLATE<M>::Set(iPos, &iElement);
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
bool ARRAY_TEMPLATE_PRIMITIVE<M>::RemoveFromSorted(M iElement)
{
	ASIZE	iPos;
	bool	bExists;

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
template<class M>
bool ARRAY_TEMPLATE_PRIMITIVE<M>::RemoveDuplicatesFromSorted(void)
{
	ASIZE	i;
	M		iValue;
	M		iCurrent;
	bool	bAnyRemoved;
	ASIZE  iWrite;

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
void ARRAY_TEMPLATE_PRIMITIVE<M>::MakeUnique(void)
{
	QuickSort();
	RemoveDuplicatesFromSorted();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::Intersect(ARRAY_TEMPLATE_PRIMITIVE<M>* pcArray1, ARRAY_TEMPLATE_PRIMITIVE<M>* pcArray2)
{
	ASIZE	i;
	ASIZE	j;
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
bool ARRAY_TEMPLATE_PRIMITIVE<M>::IsSorted(void)
{
	ASIZE	i;
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
ASIZE ARRAY_TEMPLATE_PRIMITIVE<M>::FindFinalContiguousInSorted(void)
{
	ASIZE	i;
	M		iValue;

	if (miUsedElements == 0)
	{
		return ARRAY_NOT_FOUND;
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
				return ARRAY_NOT_FOUND;
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
M ARRAY_TEMPLATE_PRIMITIVE<M>::Pop(void)
{
	M	i;

	//Returns zero if empty.
	ARRAY_TEMPLATE<M>::Pop(&i);
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::Push(M iElement)
{
	Add(iElement);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::AddRemap(ASIZE iElementPos, M iRemapNum)
{
	M*		pi;
	ASIZE	iIndex;
	ASIZE	iNumToAdd;
	ASIZE	iMinusOne;

	if (miUsedElements > iElementPos)
	{
		pi = ARRAY_TEMPLATE<M>::Get(iElementPos);
		(*pi) = iRemapNum;
	}
	else
	{
		iMinusOne = ARRAY_SIZE_MAX;
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
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::RemoveRemap(ASIZE iElementPos)
{
	M*		pi;
	ASIZE	iIndex;
	ASIZE	iCutDown;

	if (iElementPos < miUsedElements)
	{
		pi = ARRAY_TEMPLATE<M>::Get(iElementPos);
		(*pi) = -1;
	}

	iCutDown = miUsedElements;
	iIndex = miUsedElements;
	if (iIndex != 0)
	{
		do
		{
			iIndex--;
			pi = ARRAY_TEMPLATE<M>::Get(iIndex);
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
template<class M>
void ARRAY_TEMPLATE_PRIMITIVE<M>::AddList(M iStop, ...)
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
