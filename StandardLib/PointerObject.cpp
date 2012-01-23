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
void CPointerObject::Init(CObject* pcEmbedding)
{
	mpcEmbedding = pcEmbedding;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerObject::PointTo(CBaseObject* pcObject)
{
	if (mpcObject != pcObject)
	{
		if (mpcObject)
		{
			if (mpcEmbedding)
			{
				mpcEmbedding->RemoveTo(mpcObject);
				mpcObject->RemoveEmbeddedFrom(mpcEmbedding);
			}
		}
		mpcObject = pcObject;
		if (mpcObject)
		{
			mpcObject->AddFrom(mpcEmbedding);
			if (mpcEmbedding)
			{
				mpcEmbedding->AddTo(mpcObject);
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
BOOL CPointerObject::Dehollow(void)
{
	CHollowObject*	pcHollow;

	if(mpcObject)
	{
		if (mpcObject->IsHollow())
		{
			pcHollow = (CHollowObject*)mpcObject;
			mpcObject = pcHollow->Load();
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

