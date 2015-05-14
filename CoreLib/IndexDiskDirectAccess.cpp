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
#include "IndexDescriptorsFile.h"
#include "IndexDiskDirectAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexDiskDirectAccess::Init(CIndexDescriptorsFile* pcDescriptorsFile)
{
	CIndexAccess::Init(pcDescriptorsFile);

	LoadAndBegin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexDiskDirectAccess::Kill(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexDiskDirectAccess::Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	filePos	iResult;
	unsigned int iNumDescriptors;
	unsigned int loInt;

	loInt = (unsigned int)oi;
	iNumDescriptors = (unsigned int)mpcDescriptorsFile->NumDescriptors();
	if (loInt >= iNumDescriptors)
	{
		return FALSE;
	}

	iResult = mpcDescriptorsFile->Read(pcDescriptor, loInt);

	if (iResult == 0)
	{
		return FALSE;
	}
	if (!pcDescriptor->IsAllocated())
	{
		return FALSE;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexDiskDirectAccess::Set(CIndexedDataDescriptor* pcDescriptor)
{
	filePos			iResult;
	unsigned int	loInt;

	loInt = (unsigned int)pcDescriptor->GetIndex();
	iResult = mpcDescriptorsFile->Write(pcDescriptor, loInt);

	return iResult == 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexDiskDirectAccess::Remove(OIndex oi)
{
	CIndexedDataDescriptor	cNullDescriptor;
	filePos				iResult;
	unsigned int		loInt;

	memset(&cNullDescriptor, 0, sizeof(CIndexedDataDescriptor));
	cNullDescriptor.Init(oi, 0);

	loInt = (unsigned int)oi;
	iResult = mpcDescriptorsFile->Write(&cNullDescriptor, loInt);

	return iResult == 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CIndexDiskDirectAccess::Length(void)
{
	return mpcDescriptorsFile->NumDescriptors();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexDiskDirectAccess::Load(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexDiskDirectAccess::Save(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexDiskDirectAccess::UpdateFile(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexDiskDirectAccess::IsDisk(void)
{
	return TRUE;
}

