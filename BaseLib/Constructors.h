/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#ifndef __CONSTRUCTORS_H__
#define __CONSTRUCTORS_H__
#include "StackMemory.h"
#include "ConstructorCall.h"
#include "MapStringBlock.h"


class CConstructors
{
protected:
	CMapStringBlock		mcConstructors;

public:
						void	Init(void);
						void	Kill(void);

	template<class M>	M*		Add(char* szClassName);
	template<class M>	M*		Add(const char* szClassName);
	template<class M>	M*		Add(void);

	template<class M>	M*		Get(void);

						void*	Construct(const char* szName, CMallocator* pcMalloc, char(**pacDebugName)[4] = NULL);
						int		NumConstructors(void);

						BOOL	Contains(const char* szName);

						BOOL	ValidateMemoryInitialised(void);

protected:
						BOOL	ValidateNotAdded(const char* szClassName);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* StackConstruct(CStackMemory<>* pcStack)
{
	M*		pvM;
	size_t	iSize;

	iSize = sizeof(M);
	pvM = (M*)pcStack->Init(iSize);
	memset(pvM, 0, iSize);
	new(pvM) M();
	return pvM;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CConstructors::Add(char* szClassName)
{
	M*		pvM;
	int		iSize;

	if (!ValidateNotAdded(szClassName))
	{
		return NULL;
	}

	iSize = sizeof(M);
	pvM = (M*)mcConstructors.Put(szClassName, iSize);
	memset(pvM, 0, iSize);
	new(pvM) M();
	return pvM;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CConstructors::Add(const char* szClassName)
{
	M*		pvM;
	int		iSize;

	if (!ValidateNotAdded(szClassName))
	{
		return NULL;
	}

	iSize = sizeof(M);
	pvM = (M*)mcConstructors.Put(szClassName, iSize);
	memset(pvM, 0, iSize);
	new(pvM) M();
	return pvM;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CConstructors::Add(void)
{
	M*				pvM;
	int				iSize;
	CStackMemory<>	cStack;  //Stop the destructor being called on M so we can't just declare M on the stack.
	const char*		szClassName;
	BOOL			bResult;

	pvM = StackConstruct<M>(&cStack);
	szClassName = pvM->ClassName();
	if (!ValidateNotAdded(szClassName))
	{
		cStack.Kill();
		return NULL;
	}

	iSize = pvM->ClassSize();
	bResult = mcConstructors.Put(szClassName, pvM, iSize);
	if (!bResult)
	{
		cStack.Kill();
		return NULL;
	}
	pvM = (M*)mcConstructors.Get(szClassName);
	cStack.Kill(); 
	return pvM;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M>
M* CConstructors::Get(void)
{
	M*				pvM;
	CStackMemory<>	cStack;
	const char*		szClassName;

	pvM = StackConstruct<M>(&cStack);
	szClassName = pvM->ClassName();
	pvM = (M*)mcConstructors.Get(szClassName);
	if (pvM)
	{
		if (StringCompare(pvM->ClassName(), szClassName) == 0)
		{
			return pvM;
		}
	}
	return NULL;
}


#endif // !__CONSTRUCTORS_H__

