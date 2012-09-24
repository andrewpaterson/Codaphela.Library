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
#include "Object.h"
#include "HollowObject.h"
#include "ObjectDeserialiser.h"
#include "PointerObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject::CPointerObject()
{
	mpcObject = NULL;
	mpcEmbedding = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerObject::ClearObject(void)
{
	mpcObject = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerObject::Init(CObject* pcEmbedding)
{
	mpcEmbedding = pcEmbedding;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CPointerObject::UnsafePointTo(CBaseObject* pcNewObject)
{
	CBaseObject*	pcObject;

	pcObject = mpcObject;
	mpcObject = pcNewObject;
	return pcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerObject::PointTo(CBaseObject* pcNewObject)
{
	CBaseObject*	pcOldObject;

	if (mpcObject != pcNewObject)
	{
		pcOldObject = mpcObject;
		mpcObject = pcNewObject;

		if (mpcEmbedding)
		{			
			if (pcOldObject)
			{
				if (pcNewObject)
				{
					pcNewObject->AddFrom(mpcEmbedding);
				}
				pcOldObject->RemoveFrom(mpcEmbedding);
			}
			else if (mpcObject)
			{
				mpcObject->AddFrom(mpcEmbedding);
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerObject::operator = (CBaseObject* ptr)
{
	//This operator override exists only to allow NULL assignment.
	PointTo(ptr);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerObject::operator = (CPointerObject pcPointer)
{
	PointTo(pcPointer.mpcObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CPointerObject::operator -> ()
{
	Dehollow();
	return mpcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CPointerObject::operator & ()
{
	Dehollow();
	return mpcObject;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPointerObject::operator ! ()
{
	return mpcObject == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPointerObject::IsNotNull(void)
{
	return mpcObject != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPointerObject::IsNull(void)
{
	return mpcObject == NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPointerObject*	CPointerObject::This(void)
{
	//This method should only *ever* be called whilst in the Load method on a CObject
	return this;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject* CPointerObject::Object(void)
{
	return mpcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject** CPointerObject::ObjectPtr(void)
{
	return &mpcObject;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObject* CPointerObject::Embedding(void)
{
	return mpcEmbedding;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPointerObject::Dehollow(void)
{
	CHollowObject*	pcHollow;

	if (mpcObject)
	{
		if (mpcObject->IsHollow())
		{
			pcHollow = (CHollowObject*)mpcObject;
			mpcObject = pcHollow->Dehollow();
			pcHollow->Kill();
			if (mpcObject)
			{
				return TRUE;
			}
			else
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPointerObject::RemapFrom(CBaseObject* pcOld)
{
	int				iNumFroms;
	int				i;
	CBaseObject*	pvFrom;
	int				iCount;
	int				iNumKilled;

	iCount = 0;
	iNumFroms = pcOld->NumFroms();
	for (i = 0; i < iNumFroms; i++)
	{
		pvFrom = pcOld->GetFrom(i);
		iCount += pvFrom->RemapTos(pcOld, mpcObject);
		mpcObject->AddFrom(pvFrom);
	}

	mpcObject->CopyFroms(pcOld);
	mpcObject->SetDistToRoot(pcOld->DistToRoot());
	iNumKilled = pcOld->KillThisGraph();
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPointerObject::IsHollow(void)
{
	//IsHollow must be available without calling dehollow.
	if (mpcObject)
	{
		return mpcObject->IsHollow();
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPointerObject::Load(CObjectDeserialiser* pcFile)
{
	//Load must be available without calling dehollow.
	if (mpcObject)
	{
		if (!mpcObject->IsHollow())
		{
			return mpcObject->Load(pcFile);
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		return FALSE;
	}
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CPointerObject::DistToRoot(void)
{
	//DistToRoot must be available without calling dehollow.

	if (mpcObject)
	{
		return mpcObject->DistToRoot();
	}
	else
	{
		return -1;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CPointerObject::GetIndex(void)
{
	if (mpcObject)
	{
		return mpcObject->GetOI();
	}
	else
	{
		return NULL_O_INDEX;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPointerObject::GetName(void)
{
	if (mpcObject && mpcObject->IsNamed())
	{
		return mpcObject->GetName();
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
BOOL CPointerObject::IsNamed(void)
{
	if (mpcObject)
	{
		return mpcObject->IsNamed();
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPointerObject::ClassName(void)
{
	if (mpcObject)
	{
		return mpcObject->ClassName();
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
BOOL CPointerObject::IsDirty(void)
{
	if (mpcObject)
	{
		return mpcObject->IsDirty();
	}
	else
	{
		return FALSE;
	}
}

