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
#ifndef __UNKNOWNS_H__
#define __UNKNOWNS_H__
#include "BaseLib/Memory.h"
#include "BaseLib/ConstructorCall.h"
#include "ArrayUnknownPtr.h"
#include "Iterables.h"
#include "ConstructorUnknown.h"


#define UMalloc(classtype)	((classtype*)gcUnknowns.Add<classtype>());


class CUnknowns
{
friend class CConstructorUnknown;
friend class CMapStringUnknown;

protected:
	CMemory					mcMemory;
	CIterables				mcIterables;
	CChars					mszName;
	CConstructorUnknown		mcConstructors;

public:
						void			Init(char* szName);
						void			Kill(void);
						void			Remove(CUnknown* pcUnknown);
	template<class M>	M*				Add(void);
	template<class M>	M*				AddUnsafe(void);
						CUnknown*		Add(char* szClassName);
						CUnknown*		AddFromHeader(CFileReader* pcFile);
						BOOL			LoadUnknown(CFileReader* pcFile, CUnknown** ppcUnknown);
						BOOL			SaveUnknown(CFileWriter* pcFile, CUnknown* pcUnknown);

						//Iterable stuff.
	template<class M>	M*				StartIteration(SIteratorTemplate<M>* psIter);
	template<class M>	M*				Iterate(SIteratorTemplate<M>* psIter);
						CUnknown*		StartIteration(char* szClassName, SIteratorUnknown* psIter);
						CUnknown*		Iterate(SIteratorUnknown* psIter);
	template<class M>	void			RemoveDuringIteration(SIteratorTemplate<M>* psIter);

						//Debug stuff
						int				NumElements(void);
						void			DumpAddDetail(CUnknown* pcUnknown);

	template<class M>	void			AddConstructor(void);
						int				GetIterableListsHeadNumElements(void);
						CFreeListBlock*	GetFreeList(unsigned int iElementSize);

						void			RemoveInKill(CUnknown* pcUnknown);
						void			RemoveInKill(CArrayUnknownPtr* papcObjectPts);
protected:
	CUnknown*	AddExisting(CUnknown* pcUnknown);
	void		DebugName(CUnknown* pcUnknown, char (*pszDebug)[4]);
	void		BreakOnAdd(unsigned int uiAllocCount);
	BOOL		IsFreed(CUnknown* pcUnknown);
};


extern CUnknowns gcUnknowns;


void UnknownsInit(void);
void UnknownsKill(void);


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CUnknowns::Add(void)
{
	M*		pv;

	//You can only add Unknown class types to Unknowns.
	//Practically this excludes anything that extends CObject.
	pv = AddUnsafe<M>();
	if (pv->IsUnknown())
	{
		return pv;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CUnknowns::AddUnsafe(void)
{
	M*		pv;
	char	szDebug[4];
	int		iSize;

	iSize = sizeof(M);
	pv = (M*)mcMemory.Add(iSize);
	if (pv)
	{
		CONSTRUCT(pv, M);

		DebugName(pv, &szDebug);
		mcMemory.SetDebugName(pv, &szDebug);

		pv->CUnknown::PreInit(this);
		if (pv->Iterable())
		{
			mcIterables.Add(pv);
		}
		return pv;
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CUnknowns::StartIteration(SIteratorTemplate<M>* psIter)
{
	return mcIterables.StartIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CUnknowns::Iterate(SIteratorTemplate<M>* psIter)
{
	return mcIterables.Iterate(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CUnknowns::RemoveDuringIteration(SIteratorTemplate<M>* psIter)
{
	mcIterables.RemoveDuringIteration(psIter);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void CUnknowns::AddConstructor(void)
{
	mcConstructors.Add<M>();
}


#endif // __UNKNOWNS_H__

