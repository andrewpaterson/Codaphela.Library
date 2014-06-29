/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __ADDITIONAL_TYPES_H__
#define __ADDITIONAL_TYPES_H__
#include "ArraySimple.h"
#include "ArrayTemplate.h"
#include "ArrayInt.h"
#include "DataTypes.h"
#include "GeometricTypes.h"


//Array of simple int.
class CArraySimpleInt : public CArraySimple<int>
{
public:
	void	Add(int i);
	int		AddIfUnique(int i);
	int		Find(int i);
	int		GetValue(int iElementPos);
	void	QuickSort(void);
};


//Array of array of simple int.
class CArrayOfArraySimpleInt : public CArrayTemplate<CArraySimpleInt>
{
public:
	void				Allocate(int iChunkSize);
	void				Allocate(int iChunkSize, int iNumElements);
	void				InitArrays(int iStartIndex, int iEndIndex);
	void				Kill(void);
	CArraySimpleInt*	Add(void);
};

//Array of Array of int.
class CArrayOfArrayInt : public CArrayTemplate<CArrayInt>
{
public:
	void		Kill(void);
	CArrayInt*	Add(void);
	CArrayInt*	Add(int iChunkSize);
	int			AddIfUnique(CArrayInt* pcArrayInt);
	int			Find(CArrayInt* pacArrayInt);
};


//Typed array of int.
struct CTypedArrayInt : public CArrayInt
{
	int		miType;

	void	Init(void);
	void	Init(int iChunkSize);
	void	SetType(int iType);
	int		GetType(void);
};

//Array of typed array of int.
class CArrayOfTypedArrayInt : public CArrayTemplate<CTypedArrayInt>
{
public:
	int					miDefaultChunkSize;

	void				Init(void);
	void				Init(int iChunkSize);
	void				Init(int iChunkSizeMajor, int iChunkSizeMinor);
	void				Kill(void);
	CTypedArrayInt*		AddArray(int iType);
	void				AddElement(int iType, int iValue);
	CTypedArrayInt*		GetArray(int iType);
};


#endif //__ADDITIONAL_TYPES_H__

