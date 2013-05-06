/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CoreLib

Codaphela CoreLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CoreLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CoreLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "BaseLib/FastFunctions.h"
#include "BaseLib/ConstructorCall.h"
#include "IndexDiskDirectAccess.h"
#include "IndexMemoryAccess.h"
#include "IndexHugeAccess.h"
#include "IndexedDescriptors.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptors::Init(CIndexAccess* pcAccess)
{
	mpcAccess = pcAccess;
	Load();

	if (pcAccess->HasBegun())
	{
		mpcAccess->Begin();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptors::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptors::Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	return mpcAccess->Get(pcDescriptor, oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptors::Set(CIndexedDataDescriptor* pcDescriptor)
{
	return mpcAccess->Set(pcDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptors::Remove(OIndex oi)
{
	return mpcAccess->Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptors::Save(void)
{
	mpcAccess->Save();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptors::Load(void)
{
	mpcAccess->Load();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexedDescriptors::UpdateFile(void)
{
	mpcAccess->UpdateFile();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedDescriptors::Length(void)
{
	return mpcAccess->Length();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexedDescriptors::RemoveFile(void)
{
	return mpcAccess->RemoveFile();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedDescriptors::NumElements(void)
{
	OIndex						i;
	OIndex						iLength;
	CIndexedDataDescriptor		cCurrent;
	OIndex						iTotal;

	iTotal = 0;
	iLength = Length();
	for (i = 0; i < iLength; i++)
	{
		if (Get(&cCurrent, i))
		{
			iTotal++;
		}
	}
	return iTotal;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexedDescriptors::NumCachedDatas(void)
{
	OIndex				i;
	OIndex				iLength;
	CIndexedDataDescriptor	cCurrent;
	OIndex				iTotal;

	iTotal = 0;
	iLength = Length();
	for (i = 0; i < iLength; i++)
	{
		if (Get(&cCurrent, i))
		{
			if (cCurrent.IsCached())
			{
				iTotal++;
			}
		}
	}
	return iTotal;
}
