/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __ARRAY_TYPE_H__
#define __ARRAY_TYPE_H__
#include "ArrayUnknown.h"


template<class M>
class CArrayType : public CArrayUnknown
{
BASE_FUNCTIONS(CArrayType);
public:
	void 	Init(int iChunkSize = ARRAY_COMMOM_CHUNK_SIZE);
	void 	Init(CUnknowns* pcUnknownsAllocatingFrom, int iChunkSize = ARRAY_COMMOM_CHUNK_SIZE);
	void 	Kill(void);

	void	Add(M* pcUnknown);
	M*		Add(void);

	void	Insert(int iIndex, M* pcUnknown);
	M*		Insert(int iIndex);

	void	Remove(int iIndex);
	BOOL	Remove(M* pcUnknown);

	M*		Get(int iIndex);

	M*		Last(void);
	M*		StartIteration(SSetIterator* psIter);
	M*		Iterate(SSetIterator* psIter);

protected:
	BOOL	LoadElement(CFileReader* pcFile, CUnknown** ppcUnknown);
	BOOL	SaveElement(CFileWriter* pcFile, CUnknown* pcUnknown);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayType<M>::Init(int iChunkSize)
{
	CArrayUnknown::Init(iChunkSize);
	TypeKnown(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayType<M>::Init(CUnknowns* pcUnknownsAllocatingFrom, int iChunkSize)
{
	CArrayUnknown::Init(pcUnknownsAllocatingFrom, iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayType<M>::Kill(void)
{
	CArrayUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayType<M>::Add(M* pcUnknown)
{
	CArrayUnknown::Add(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayType<M>::Add(void)
{
	return (M*)CArrayUnknown::Add<M>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayType<M>::Insert(int iIndex, M* pcUnknown)
{
	CArrayUnknown::Insert(iIndex, pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayType<M>::Insert(int iIndex)
{
	return (M*)CArrayUnknown::Insert<M>(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayType<M>::Remove(int iIndex)
{
	CArrayUnknown::Remove(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayType<M>::Remove(M* pcUnknown)
{
	return CArrayUnknown::Remove(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayType<M>::Get(int iIndex)
{
	return (M*)CArrayUnknown::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayType<M>::Last(void)
{
	return (M*)CArrayUnknown::Last();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayType<M>::StartIteration(SSetIterator* psIter)
{
	return (M*)CArrayUnknown::StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayType<M>::Iterate(SSetIterator* psIter)
{
	return (M*)CArrayUnknown::Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayType<M>::LoadElement(CFileReader* pcFile, CUnknown** ppcUnknown)
{
	M*	pcUnknown;

	pcUnknown = gcUnknowns.Add<M>();
	ReturnOnFalse(pcUnknown->Load(pcFile));
	*ppcUnknown = pcUnknown;
	return TRUE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CArrayType<M>::SaveElement(CFileWriter* pcFile, CUnknown* pcUnknown)
{
	return pcUnknown->Save(pcFile);
}


#endif // __ARRAY_TYPE_H__

