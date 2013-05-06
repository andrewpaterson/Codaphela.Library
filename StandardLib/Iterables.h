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
#ifndef __ITERABLES_H__
#define __ITERABLES_H__
#include "BaseLib/FreeList.h"
#include "BaseLib/Chars.h"
#include "BaseLib/LinkListTemplate.h"
#include "Unknown.h"


typedef CFreeList<CUnknown*>	CFreeListUnknownPtr;


class CIterableList;
template<class M>
class SIteratorTemplate
{
public:
	CIterableList*		pcList;
	SFreeListIterator	sIter;
};


class SIteratorUnknown
{
public:
	CIterableList*		pcList;
	SFreeListIterator	sIter;
};


class CIterableList
{
public:
	CFreeListUnknownPtr	mcFreeListPtrs;
	CChars				mszName;

	void		Init(char* szName);
	void		Kill(void);
	void		Add(CUnknown* pcUnknown);
	BOOL		Remove(CUnknown* pcUnknown);
	BOOL		Is(char* szName);
	template<class M>
	M*			StartIteration(SIteratorTemplate<M>* psIter);
	template<class M>
	M*			Iterate(SIteratorTemplate<M>* psIter);
	CUnknown*	StartIteration(SIteratorUnknown* psIter);
	CUnknown*	Iterate(SIteratorUnknown* psIter);
};


typedef CLinkListTemplate<CIterableList>	CLinkListIterableList;


class CIterables
{
public:
	CLinkListIterableList	mcIterableLists;

	void		Init(void);
	void		Kill(void);

	template<class M>
	M*			StartIteration(SIteratorTemplate<M>* psIter);
	template<class M>
	M*			Iterate(SIteratorTemplate<M>* psIter);
	CUnknown*	StartIteration(char* szName, SIteratorUnknown* psIter);
	CUnknown*	Iterate(SIteratorUnknown* psIter);
	void		Add(CUnknown* pcUnknown);
	BOOL		Remove(CUnknown* pcUnknown);
	template<class M>
	void		RemoveDuringIteration(SIteratorTemplate<M>* psIter);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIterableList::StartIteration(SIteratorTemplate<M>* psIter)
{
	M**		ppv;

	psIter->pcList = this;
	ppv = (M**)mcFreeListPtrs.StartIteration(&psIter->sIter);
	if (ppv)
	{
		return *ppv;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIterableList::Iterate(SIteratorTemplate<M>* psIter)
{
	M**		ppv;

	ppv = (M**)mcFreeListPtrs.Iterate(&psIter->sIter);
	if (ppv)
	{
		return *ppv;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIterables::StartIteration(SIteratorTemplate<M>* psIter)
{
	CIterableList*	pcList;
	M				vTemp;

	if (vTemp.Iterable())
	{
		pcList = mcIterableLists.GetHead();
		while (pcList)
		{
			if (pcList->Is(vTemp.ClassName()))
			{
				return pcList->StartIteration(psIter);
			}
			pcList = mcIterableLists.GetNext(pcList);
		}
		psIter->pcList = NULL;
		psIter->sIter.psCurrent = NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CIterables::Iterate(SIteratorTemplate<M>* psIter)
{
	M**		ppv;

	if (psIter->pcList)
	{
		ppv = (M**)psIter->pcList->mcFreeListPtrs.Iterate(&psIter->sIter);
		if (ppv)
		{
			return *ppv;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CIterables::RemoveDuringIteration(SIteratorTemplate<M>* psIter)
{
	M**	ppv;

	ppv = (M**)psIter->pcList->mcFreeListPtrs.GetElement(psIter->sIter.psCurrent, psIter->sIter.iPosition);
	psIter->pcList->mcFreeListPtrs.RemoveDuringIteration(&psIter->sIter);
	if (ppv)
	{
		(*ppv)->Kill();
	}
}




#endif // __ITERABLES_H__

