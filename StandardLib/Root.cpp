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
#include "ClassDefines.h"
#include "Objects.h"
#include "ObjectWriter.h"
#include "ObjectReader.h"
#include "Root.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CRoot::CRoot()
{
	SetDistToRoot(ROOT_DIST_TO_ROOT);
	SetPointerTosExpectedDistToRoot(ROOT_DIST_TO_ROOT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CRoot> CRoot::Init(void)
{
	PreInit();

	mpcObjectsAllocatingFrom = mpcObjectsThisIn;
	CreateSet();

	PostInit();
	return Ptr<CRoot>(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::KillInternal(bool bHeapFromChanged)
{
	mpObjects->Kill();
	CBaseObject::KillInternal(bHeapFromChanged);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::Free(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::Class(void)
{
	M_Pointer(mpObjects);
	U_Pointer(mpcObjectsAllocatingFrom);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::CreateSet(void)
{
	mpObjects = mpcObjectsAllocatingFrom->Malloc<CSetObject>()->Init();
	mpObjects->MakeSubRoot();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::NullifySet(void)
{
	mpObjects = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::Add(CPointer& pObject)
{
	if (pObject.IsNotNull())
	{
		mpObjects->Add(pObject);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoot::Remove(CPointer& pObject)
{
	if (pObject.IsNotNull())
	{
		return mpObjects->Remove(pObject);
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoot::Remove(CBaseObject* pcObject)
{
	return mpObjects->Remove(pcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::RemoveAll(void)
{
	mpObjects->RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoot::IsRoot(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoot::IsSubRoot(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoot::Load(CObjectReader* pcFile)
{
	//This smells.
	mpcObjectsAllocatingFrom = &gcObjects;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CRoot::Get(int iIndex)
{
	mpObjects.Dereference();
	CPointer pObject = mpObjects->Get(iIndex);
	return pObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointer CRoot::Get(char* szObjectName)
{
	SSetIterator	sIter;
	char*			szName;

	if (StrEmpty(szObjectName))
	{
		return Null();
	}

	CPointer pObject = mpObjects->StartIteration(&sIter);
	while (pObject.IsNotNull())
	{
		szName = pObject.GetName();
		if (szName != NULL)
		{
			if (strcmp(szObjectName, szName) == 0)
			{
				return pObject;
			}
		}
		pObject = mpObjects->Iterate(&sIter);
	}

	return Null();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
Ptr<CSetObject> CRoot::GetAll(void)
{
	return mpObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CSetObject* CRoot::TestGetSet(void)
{
	return	&mpObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CRoot::IsSetHollow(void)
{
	return mpObjects.IsHollow();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CRoot::NumObjects(void)
{
	return mpObjects->NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::TouchAll(void)
{
	mpObjects->TouchAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::KillAll(void)
{
	mpObjects->KillAll();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::Add(CPointer& pO1, CPointer& pO2)
{
	Add(pO1);
	Add(pO2);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CRoot::Add(CPointer& pO1, CPointer& pO2, CPointer& pO3)
{
	Add(pO1);
	Add(pO2);
	Add(pO3);
}


