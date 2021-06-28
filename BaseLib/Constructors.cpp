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
#include "Logger.h"
#include "Constructors.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstructors::Init(void)
{
	mcConstructors.Init(32);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CConstructors::Kill(void)
{
	mcConstructors.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CConstructors::Construct(const char* szConstructorName, CMallocator* pcMalloc, char(**pacDebugName)[4])
{
	return Construct(szConstructorName, pcMalloc, 0, pacDebugName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CConstructors::Construct(const char* szConstructorName, CMallocator* pcMalloc, size_t uiAdditionalSize, char(**pacDebugName)[4])
{
	void*		pcConstructor;
	size_t		uiSize;
	void*		pcObject;

	pcConstructor = mcConstructors.Get(szConstructorName, (int*)&uiSize);
	if (!pcConstructor)
	{
		gcLogger.Error2(__METHOD__, " Could not find a Constructor named [", szConstructorName, "].", NULL);
		return NULL;
	}

	pcObject = pcMalloc->Malloc(uiSize + uiAdditionalSize, pacDebugName);
	memcpy(pcObject, pcConstructor, uiSize);
	return pcObject;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CConstructors::NumConstructors(void)
{
	return mcConstructors.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConstructors::ValidateMemoryInitialised(void)
{
	if (!mcConstructors.IsMallocInitialised())
	{
		return gcLogger.Error2(__METHOD__, " Constructor's Malloc is not initialised.  Call MemoryInit()", NULL);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConstructors::ValidateNotAdded(const char* szClassName)
{
	BOOL	bResult;

	bResult = mcConstructors.HasKey(szClassName);
	if (bResult)
	{
		return gcLogger.Error2(__METHOD__, " Constructor [", szClassName, "] has already been added.", NULL);
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CConstructors::Contains(const char* szName)
{
	void*	pcConstructor;
	int		iSize;

	pcConstructor = mcConstructors.Get(szName, &iSize);
	return pcConstructor != NULL;
}


