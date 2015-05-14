/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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
#include "IndexedDataDescriptor.h"
#include "IndexDescriptorsFile.h"
#include "IndexAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexAccess::Init(CIndexDescriptorsFile* pcDescriptorsFile)
{
	mpcDescriptorsFile = pcDescriptorsFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::IsHuge(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::IsMemory(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::IsDisk(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexAccess::Begin(void)
{
	mpcDescriptorsFile->GetDurableFile()->Begin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::HasBegun(void)
{
	return mpcDescriptorsFile->mpcDurableFileControl->IsBegun();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexAccess::LoadAndBegin(void)
{
	Load();
	if (HasBegun())
	{
		Begin();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexAccess::RemoveFile(void)
{
	return mpcDescriptorsFile->Delete();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CIndexAccess::NumElements(void)
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
OIndex CIndexAccess::NumCachedDatas(void)
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
