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
#ifndef __SET_TYPE_H__
#define __SET_TYPE_H__
#include "SetUnknown.h"


template<class M>
class CSetType : public CSetUnknown
{
BASE_FUNCTIONS(CSetType);
public:
	void 	Init(int iChunkSize = ARRAY_COMMOM_CHUNK_SIZE);
	void 	Init(CUnknowns* pcUnknownsAllocatingFrom, int iChunkSize = ARRAY_COMMOM_CHUNK_SIZE);
	void 	Kill(void);

	void	Add(M* pcUnknown);
	M*		Add(void);
	BOOL	AddAll(CArrayCommonUnknown* pcSource);

	BOOL	Remove(M* pcUnknown);
	void	RemoveDuringIteration(SSetIterator* psIter);

	M*		StartIteration(SSetIterator* psIter);
	M*		Iterate(SSetIterator* psIter);

	BOOL	Contains(M* pcUnknown);

protected:
	BOOL	LoadElement(CFileReader* pcFile, CUnknown** ppcUnknown);
	BOOL	SaveElement(CFileWriter* pcFile, CUnknown* pcUnknown);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CSetType<M>::Init(int iChunkSize)
{
	CSetUnknown::Init(iChunkSize);
	TypeKnown(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CSetType<M>::Init(CUnknowns* pcUnknownsAllocatingFrom, int iChunkSize)
{
	CSetUnknown::Init(pcUnknownsAllocatingFrom, iChunkSize);
	TypeKnown(TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CSetType<M>::Kill(void)
{
	CSetUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CSetType<M>::Add(M* pcUnknown)
{
	CSetUnknown::Add(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CSetType<M>::Add(void)
{
	return (M*)CSetUnknown::Add<M>();
}


/////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CSetType<M>::AddAll(CArrayCommonUnknown* pcSource)
{
	return CArrayCommonUnknown::AddAll(pcSource);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CSetType<M>::Remove(M* pcUnknown)
{
	return CSetUnknown::Remove(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CSetType<M>::RemoveDuringIteration(SSetIterator* psIter)
{
	CSetUnknown::RemoveDuringIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CSetType<M>::StartIteration(SSetIterator* psIter)
{
	return (M*)CSetUnknown::StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CSetType<M>::Iterate(SSetIterator* psIter)
{
	return (M*)CSetUnknown::Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CSetType<M>::Contains(M* pcUnknown)
{
	return CArrayCommonUnknown::Contains(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
BOOL CSetType<M>::LoadElement(CFileReader* pcFile, CUnknown** ppcUnknown)
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
BOOL CSetType<M>::SaveElement(CFileWriter* pcFile, CUnknown* pcUnknown)
{
	return pcUnknown->Save(pcFile);
}


#endif // __SET_TYPE_H__

