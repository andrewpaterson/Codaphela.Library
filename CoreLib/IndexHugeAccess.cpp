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
#include "IndexHugeAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexHugeAccess::Init(CIndexDescriptorsFile* pcDescriptorsFile, CIndexedData* pcIndexedData, BOOL bDirtyTesting, int iSecondLevelWidth, int iThirdLevelWidth, int iNumSecondLevelChunks, int iNumThirdLevelChunks)
{
	CIndexAccess::Init(pcDescriptorsFile);
	mcHugeSupport.Init(pcDescriptorsFile->GetDurableFile(), bDirtyTesting, pcIndexedData, iSecondLevelWidth, iThirdLevelWidth, iNumSecondLevelChunks, iNumThirdLevelChunks);

	LoadAndBegin();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexHugeAccess::Kill(void)
{
	mcHugeSupport.Kill();
}		


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CIndexHugeAccess::Length(void)
{
	return mcHugeSupport.Length();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexHugeAccess::Get(CIndexedDataDescriptor* pcDescriptor, OIndex oi)
{
	return mcHugeSupport.Get(pcDescriptor, oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexHugeAccess::Set(CIndexedDataDescriptor* pcDescriptor)
{
	return mcHugeSupport.Set(pcDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexHugeAccess::Remove(OIndex oi)
{
	return mcHugeSupport.Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexHugeAccess::Load(void)
{
	mcHugeSupport.Load();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexHugeAccess::Save(void)
{
	mcHugeSupport.Save();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexHugeAccess::UpdateFile(void)
{
	return mcHugeSupport.UpdateFile();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexHugeAccess::Set(CIndexedDataDescriptor* pacDescriptors, int iNumDescriptors)
{
	return mcHugeSupport.Set(pacDescriptors, iNumDescriptors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexHugeAccess::IsHuge(void)
{
	return TRUE;
}

