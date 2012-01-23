/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WorldLib

Codaphela WorldLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WorldLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WorldLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft DirectX is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __INTERPOLATOR_ARRAY_H__
#define __INTERPOLATOR_ARRAY_H__


//This looks really dodgy and I think all it was for was interpolating colours.


//////////////////////////////////////////////////////////////////////////
//	Inertpolator Array Template
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class T>
class CInertpolatorArrayTemplate
{
public:
	T*		mpaStartArray;
	T*		mpaEndArray;
	T*		mpaOutputArray;
	int		miArraySize;

	void Init(int iArraySize);
	void Kill(void);

	void SetStartElement(int iElementNum, T fValue);
	void SetStartArray(T* pafArray);  //Uses a memcpy to set the array, does not just set the pointer.
	void SetEndElement(int iElementNum, T fValue);
	void SetEndArray(T* pafArray);  //Uses a memcpy to set the array, does not just set the pointer.
	void Update(float fPos);
};


typedef CInertpolatorArrayTemplate<float>	__CInertpolatorArrayFloat;
typedef CInertpolatorArrayTemplate<int>		__CInertpolatorArrayInt;


//////////////////////////////////////////////////////////////////////////
//	Inertpolator Array Float
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CInertpolatorArrayFloat : public __CInertpolatorArrayFloat
{
public:
};


//////////////////////////////////////////////////////////////////////////
//	Inertpolator Array Int
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
class CInertpolatorArrayInt : public __CInertpolatorArrayInt
{
public:
};



//////////////////////////////////////////////////////////////////////////
//	Inertpolator Array Template Implementation
//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class T>
void CInertpolatorArrayTemplate<T>::Init(int iArraySize)
{
	mpaStartArray = malloc(sizeof(float)*iArraySize);
	mpaEndArray = malloc(sizeof(float)*iArraySize);
	mpaOutputArray = malloc(sizeof(float)*iArraySize);
	miArraySize = iArraySize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class T>
void CInertpolatorArrayTemplate<T>::Kill(void)
{
	if (mpaStartArray != NULL)
	{
		free(mpaStartArray);
	}
	if (mpaEndArray != NULL)
	{
		free(mpaEndArray)
	}
	if (mpaOutputArray != NULL)
	{
		free(mpaOutputArray)
	}
	mpaStartArray = NULL;
	mpaEndArray = NULL;
	mpaOutputArray = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class T>
void CInertpolatorArrayTemplate<T>::SetStartElement(int iElementNum, T fValue)
{
	mpaStartArray[iElementNum] = T;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class T>
void CInertpolatorArrayTemplate<T>::SetStartArray(T* pafArray)  //Uses a memcpy to set the array, does not just set the pointer.
{
	memcpy(mpaStartArray, pafArray, sizeof(T)*miArraySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class T>
void CInertpolatorArrayTemplate<T>::SetEndElement(int iElementNum, T fValue)
{
	mpaEndArray[iElementNum] = T;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class T>
void CInertpolatorArrayTemplate<T>::SetEndArray(T* pafArray)  //Uses a memcpy to set the array, does not just set the pointer.
{
	memcpy(mpaEndArray, pafArray, sizeof(T)*miArraySize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class T>
void CInertpolatorArrayTemplate<T>::Update(float fPos)
{
	int		i;

	for (i = 0; i < miArraySize; i++)
	{
		mpaOutputArray[i] = (T)(mpaStartArray[i] * fPos) + (T)(mpaEndArray[i] * (1.0f - fPos));
	}
}


#endif //__INTERPOLATOR_ARRAY_H__


