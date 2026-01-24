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
#ifndef __ARRAY_UNKNOWN_TEMPLATE_H__
#define __ARRAY_UNKNOWN_TEMPLATE_H__
#include "Unknowns.h"
#include "ArrayUnknown.h"


template<class M>
class CArrayUnknownTemplate : public CArrayUnknown
{
CONSTRUCTABLE(CArrayUnknownTemplate);
public:
	void 	Init(void);
	void 	Init(CUnknowns* pcUnknownsAllocatingFrom);
	void 	Kill(void);

	void	Add(M* pcUnknown);
	M*		Add(void);

	void	Insert(int iIndex, M* pcUnknown);
	M*		Insert(int iIndex);

	void	Remove(int iIndex);
	bool	Remove(M* pcUnknown);

	M*		Get(int iIndex);

	M*		Last(void);
	M*		StartIteration(SSetIterator* psIter);
	M*		Iterate(SSetIterator* psIter);

protected:
	bool	LoadElement(CFileReader* pcFile, CUnknown** ppcUnknown);
	bool	SaveElement(CFileWriter* pcFile, CUnknown* pcUnknown);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayUnknownTemplate<M>::Init(void)
{
	CArrayUnknown::Init();
	TypeKnown(true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayUnknownTemplate<M>::Init(CUnknowns* pcUnknownsAllocatingFrom)
{
	CArrayUnknown::Init(pcUnknownsAllocatingFrom, iChunkSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayUnknownTemplate<M>::Kill(void)
{
	CArrayUnknown::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayUnknownTemplate<M>::Add(M* pcUnknown)
{
	CArrayUnknown::Add(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayUnknownTemplate<M>::Add(void)
{
	return (M*)CArrayUnknown::Add<M>();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayUnknownTemplate<M>::Insert(int iIndex, M* pcUnknown)
{
	CArrayUnknown::Insert(iIndex, pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayUnknownTemplate<M>::Insert(int iIndex)
{
	return (M*)CArrayUnknown::Insert<M>(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CArrayUnknownTemplate<M>::Remove(int iIndex)
{
	CArrayUnknown::Remove(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayUnknownTemplate<M>::Remove(M* pcUnknown)
{
	return CArrayUnknown::Remove(pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayUnknownTemplate<M>::Get(int iIndex)
{
	return (M*)CArrayUnknown::Get(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayUnknownTemplate<M>::Last(void)
{
	return (M*)CArrayUnknown::Last();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayUnknownTemplate<M>::StartIteration(SSetIterator* psIter)
{
	return (M*)CArrayUnknown::StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayUnknownTemplate<M>::Iterate(SSetIterator* psIter)
{
	return (M*)CArrayUnknown::Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayUnknownTemplate<M>::LoadElement(CFileReader* pcFile, CUnknown** ppcUnknown)
{
	M*	pcUnknown;

	pcUnknown = gcUnknowns.Add<M>();
	ReturnOnFalse(pcUnknown->Load(pcFile));
	*ppcUnknown = pcUnknown;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayUnknownTemplate<M>::SaveElement(CFileWriter* pcFile, CUnknown* pcUnknown)
{
	return pcUnknown->Save(pcFile);
}


#endif // __ARRAY_UNKNOWN_TEMPLATE_H__

