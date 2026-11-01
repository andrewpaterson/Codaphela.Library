/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "ObjectWriter.h"
#include "ObjectReader.h"
#include "OrdererHolder.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrdererHolder::Init(void)
{
	Init(NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrdererHolder::Init(Ptr<CBaseObject> pObject)
{
	PreInit();
	mpObject = pObject;
	maszFulfilments.Init();
	maszRequirements.Init();
	PostInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrdererHolder::Free(void)
{
	maszRequirements.Kill();
	maszFulfilments.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void COrdererHolder::Class(void)
{
	M_Pointer(mpObject);
	U_Data(CArrayChars, maszFulfilments);
	U_Data(CArrayChars, maszRequirements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrdererHolder::Save(CObjectWriter* pcFile)
{
	ReturnOnFalse(SaveNames(pcFile, &maszFulfilments));
	return SaveNames(pcFile, &maszRequirements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrdererHolder::Load(CObjectReader* pcFile)
{
	ReturnOnFalse(LoadNames(pcFile, &maszFulfilments));
	return LoadNames(pcFile, &maszRequirements);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrdererHolder::SaveNames(CObjectWriter* pcFile, CArrayChars* paszNames)
{
	size	i;
	size	uiNumElements;

	ReturnOnFalse(pcFile->WriteSize(paszNames->NumElements()));
	uiNumElements = paszNames->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		ReturnOnFalse(paszNames->Get(i)->WriteString(pcFile));
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool COrdererHolder::LoadNames(CObjectReader* pcFile, CArrayChars* paszNames)
{
	size	uiNumNames;
	size	i;
	CChars	szName;

	paszNames->Init();
	ReturnOnFalse(pcFile->ReadSize(&uiNumNames));
	for (i = 0; i < uiNumNames; i++)
	{
		ReturnOnFalse(szName.ReadString(pcFile));
		paszNames->Add(&szName);
		szName.Kill();
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A name is only kept once.
//
//////////////////////////////////////////////////////////////////////////
void COrdererHolder::AddFulfilment(const char* szName)
{
	if (maszFulfilments.GetIndex(szName) == ARRAY_ELEMENT_NOT_FOUND)
	{
		maszFulfilments.Add(szName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A name is only kept once.
//
//////////////////////////////////////////////////////////////////////////
void COrdererHolder::AddRequirement(const char* szName)
{
	if (maszRequirements.GetIndex(szName) == ARRAY_ELEMENT_NOT_FOUND)
	{
		maszRequirements.Add(szName);
	}
}


Ptr<CBaseObject> COrdererHolder::GetObject(void) { return mpObject; }
CArrayChars* COrdererHolder::GetFulfilments(void) { return &maszFulfilments; }
CArrayChars* COrdererHolder::GetRequirements(void) { return &maszRequirements; }

