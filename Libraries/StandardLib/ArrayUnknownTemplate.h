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
CONSTRUCTABLE(CArrayUnknownTemplate)
public:
	void 	Init(void);
	void 	Kill(void);

	void	Add(M* pcUnknown);
	M*		Add(void);

	bool	Insert(size iIndex, M* pcUnknown);
	M*		Insert(size iIndex);

	bool	Remove(size iIndex);
	bool	Remove(M* pcUnknown);

	M*		Get(size iIndex);

	M*		Last(void);
	bool	StartIteration(SSetIterator* psIter, M** ppcUnknown);
	bool	Iterate(SSetIterator* psIter, M** ppcUnknown);

protected:
	bool	ReadElement(CFileReader* pcFile, CUnknown** ppcUnknown) override;
	bool	WriteElement(CFileWriter* pcFile, CUnknown* pcUnknown) override;
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
bool CArrayUnknownTemplate<M>::Insert(size iIndex, M* pcUnknown)
{
	return CArrayUnknown::Insert(iIndex, pcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CArrayUnknownTemplate<M>::Insert(size iIndex)
{
	return (M*)CArrayUnknown::Insert<M>(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayUnknownTemplate<M>::Remove(size iIndex)
{
	return CArrayUnknown::Remove(iIndex);
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
M* CArrayUnknownTemplate<M>::Get(size iIndex)
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
bool CArrayUnknownTemplate<M>::StartIteration(SSetIterator* psIter, M** ppcUnknown)
{
	return CArrayUnknown::StartIteration(psIter, (CUnknown**)ppcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayUnknownTemplate<M>::Iterate(SSetIterator* psIter, M** ppcUnknown)
{
	return CArrayUnknown::Iterate(psIter, (CUnknown**)ppcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayUnknownTemplate<M>::ReadElement(CFileReader* pcFile, CUnknown** ppcUnknown)
{
	return gcUnknowns.LoadKnownType<M>(pcFile, ppcUnknown);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
bool CArrayUnknownTemplate<M>::WriteElement(CFileWriter* pcFile, CUnknown* pcUnknown)
{
	return gcUnknowns.SaveKnownType(pcFile, pcUnknown);
}


#endif // __ARRAY_UNKNOWN_TEMPLATE_H__

