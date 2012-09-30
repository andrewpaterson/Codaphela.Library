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
#include "Pointer.h"
#include "ObjectSerialiser.h"
#include "ObjectDeserialiser.h"
#include "ArrayCommonObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Init(BOOL bUnique, BOOL bIgnoreNull, BOOL bPreserveOrder, int iChunkSize)
{
	mcArray.Init(FALSE, FALSE, bUnique, bIgnoreNull, bPreserveOrder, iChunkSize);
	mbSubRoot = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Kill(void)
{
	CCollection::Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::KillToPointers(void)
{
	mcArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::KillChildGraph(void)
{
	CArrayBaseObjectPtr	apcKilled;
	CBaseObject*		pcPointedTo;
	int					i;

	apcKilled.Init(1024);

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			pcPointedTo->CollectThoseToBeKilled(&apcKilled);
		}
	}

	KillCollected(&apcKilled);

	apcKilled.Kill();

	mcArray.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveAll(void)
{
	KillChildGraph();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::KillData(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCommonObject::Save(CObjectSerialiser* pcFile)
{
	int				i;
	int				iNumElements;
	CBaseObject*	pcPointedTo;

	ReturnOnFalse(mcArray.SaveArrayHeader(pcFile));

	ReturnOnFalse(pcFile->WriteBool(mbSubRoot));

	iNumElements = mcArray.NumElements();
	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		ReturnOnFalse(pcFile->WriteDependent(pcPointedTo));
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCommonObject::Load(CObjectDeserialiser* pcFile)
{
	int				i;
	int				iFlags;
	int				iNumElements;
	CBaseObject**	pcPointedTo;

	//Note: This function has never been called.
	ReturnOnFalse(mcArray.LoadArrayHeader(pcFile, &iFlags, &iNumElements));

	ReturnOnFalse(pcFile->ReadBool(&mbSubRoot));

	for (i = 0; i < iNumElements; i++)
	{
		pcPointedTo = (CBaseObject**)mcArray.UnsafeGetPointer(i);
		ReturnOnFalse(pcFile->ReadDependent(pcPointedTo, this));
	}

	mcArray.PostLoad(iFlags);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Add(CPointerObject pObject)
{
	mcArray.Add(pObject.mpcObject);
	if (pObject.IsNotNull())
	{
		pObject->AddFrom(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Set(int iIndex, CPointerObject pObject)
{
	CBaseObject*		pcPointedTo;

	if ((iIndex < 0) || (iIndex >= mcArray.UnsafeNumElements()))
	{
		return;
	}

	pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(iIndex);
	mcArray.Set(iIndex, pObject.mpcObject);
	if (pcPointedTo)
	{
		pcPointedTo->RemoveFrom(this);
	}

	pObject->AddFrom(this);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::Remove(CPointerObject pObject)
{
	if (pObject.IsNotNull())
	{
		mcArray.Remove(pObject.mpcObject);
		pObject->RemoveFrom(this);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::NumElements(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::NumTos(void)
{
	CBaseObject*	pcPointedTo;
	int				i;
	int				iCount;

	iCount = 0;
	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			iCount++;
		}
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::CollectThoseToBeKilled(CArrayBaseObjectPtr* papcKilled)
{
	CBaseObject*			pcPointedTo;
	int						i;

	MarkForKilling(papcKilled);

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			if (pcPointedTo->miDistToRoot >= ROOT_DIST_TO_ROOT)
			{
				if (!pcPointedTo->CanFindRoot())
				{
					pcPointedTo->CollectThoseToBeKilled(papcKilled);
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveAllTos(CArrayEmbeddedBaseObjectPtr* papcFromsChanged)
{
	CBaseObject*			pcPointedTo;
	int						i;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		RemoveToFrom(pcPointedTo, papcFromsChanged);
	}
	mcArray.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::RemoveTo(CBaseObject* pcTo)
{
	mcArray.Remove((CUnknown*)pcTo);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::MakeSubRoot(void)
{
	mbSubRoot = TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CArrayCommonObject::IsSubRoot(void)
{
	return mbSubRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::SetDistToRoot(int iDistToRoot)
{
	CBaseObject*			pcPointedTo;
	int						i;

	miDistToRoot = iDistToRoot;

	for (i = 0; i < mcArray.UnsafeNumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			PotentiallySetDistToRoot(pcPointedTo, iDistToRoot+1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CArrayCommonObject::GetTos(CArrayBaseObjectPtr* papcTos)
{
	CBaseObject*	pcPointedTo;
	int				i;

	for (i = 0; i < mcArray.NumElements(); i++)
	{
		pcPointedTo = (CBaseObject*)mcArray.UnsafeGet(i);
		if (pcPointedTo)
		{
			papcTos->Add(&pcPointedTo);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CArrayCommonObject::UnsafeGet(int iIndex)
{
	return (CBaseObject*)mcArray.UnsafeGet(iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CArrayCommonObject::RemapTos(CBaseObject* pcOld, CBaseObject* pcNew)
{
	int				iCount;
	CBaseObject**	ppcPointedTo;
	int				i;

	iCount = 0;
	for (i = 0; i < mcArray.NumElements(); i++)
	{
		ppcPointedTo = (CBaseObject**)mcArray.UnsafeGetPointer(i);
		if (*ppcPointedTo)
		{
			if (*ppcPointedTo == pcOld)
			{
				*ppcPointedTo = pcNew;
				iCount++;
			}
		}
	}
	return iCount;
}
