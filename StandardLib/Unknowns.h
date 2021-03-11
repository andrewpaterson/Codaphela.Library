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
#include "BaseLib/MemoryAllocator.h"
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/Constructors.h"
#include "BaseLib/Log.h"
#include "BaseLib/LifeCycle.h"
#include "ArrayUnknownPtr.h"
#include "Iterables.h"


#define UMalloc(classtype)	((classtype*)gcUnknowns.Add<classtype>());


class CUnknowns
{
protected:
	CIterables				mcIterables;
	CChars					mszName;
	CConstructors*			mpcConstructors;
	CLife<CMallocator>		mcMallocLife;
	CMallocator*			mpcMalloc;
	size_t					miNumElements;

public:
						void			Init(char* szName, CConstructors* pcConstructors);
						void			Init(CLifeInit<CMallocator> cMalloc, char* szName, CConstructors* pcConstructors);
						void			Kill(void);

						void			Remove(CUnknown* pcUnknown);

	template<class M>	M*				Add(void);
	template<class M>	M*				AddUnsafe(void);
	template<class M>	M*				AddUnsafe(int iAdditionalSize);
						CUnknown*		Add(char* szClassName);
						CUnknown*		AddFromHeader(CFileReader* pcFile);


	template<class M>	void			AddConstructor(void);

						BOOL			LoadUnknown(CFileReader* pcFile, CUnknown** ppcUnknown);
						BOOL			SaveUnknown(CFileWriter* pcFile, CUnknown* pcUnknown);

						//Iterable stuff.
	template<class M>	M*				StartIteration(SIteratorTemplate<M>* psIter);
	template<class M>	M*				Iterate(SIteratorTemplate<M>* psIter);
						CUnknown*		StartIteration(char* szClassName, SIteratorUnknown* psIter);
						CUnknown*		Iterate(SIteratorUnknown* psIter);
	template<class M>	void			RemoveDuringIteration(SIteratorTemplate<M>* psIter);

						//Debug stuff
						void			DumpAddDetail(CUnknown* pcUnknown);
						size_t			NumElements(void);

						int				GetIterableListsHeadNumElements(void);

						void			RemoveInKill(CUnknown* pcUnknown);
						void			RemoveInKill(CArrayUnknownPtr* papcObjectPts);

protected:
						CUnknown*		AddExisting(CUnknown* pcExisting);
						void			DebugName(CUnknown* pcUnknown, char (*pszDebug)[4]);
						BOOL			IsFreed(CUnknown* pcUnknown);
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
		RemoveInKill(pv);
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
	return AddUnsafe<M>(0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CUnknowns::AddUnsafe(int iAdditionalSize)
{
	M*			pv;
	char(*		acDebug)[4];
	int			iSize;

	iSize = sizeof(M);
	pv = (M*)mpcMalloc->Malloc(iSize + iAdditionalSize, &acDebug);
	if (pv)
	{
		memset(pv, 0, iSize + iAdditionalSize);
		new(pv) M();

		DebugName(pv, (char(*)[4])acDebug);
		pv->CUnknown::SetUnknowns(this);
		if (pv->Iterable())
		{
			mcIterables.Add(pv);
		}

		miNumElements++;
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
	const char*	szClassName;
	M*			pvM;

	pvM = NewMalloc<M>();
	if (pvM)
	{
		szClassName = pvM->ClassName();
		if (mpcConstructors)
		{
			mpcConstructors->Add<M>(szClassName);
		}
		else
		{
			gcLogger.Error2(__METHOD__, " Constructors for Unknowns is NULL.", NULL);
		}
		free (pvM);
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Couldn't get class name whilst adding constructor for class.", NULL);
	}
}


#endif // __UNKNOWNS_H__

