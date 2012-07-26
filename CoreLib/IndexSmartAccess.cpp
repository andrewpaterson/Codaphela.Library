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
#include "IndexSmartAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexSmartAccess::Init(CIndexDescriptorsFile* pcDescriptorsFile, CIndexedData* pcIndexedData, BOOL bDirtyTesting, int iSecondLevelWidth, int iThirdLevelWidth, int iNumSecondLevelChunks, int iNumThirdLevelChunks)
{
	long long int		iNumDescriptors;

	CIndexAccess::Init(pcDescriptorsFile);
	mpcIndexedData = pcIndexedData;
	miSecondLevelWidth = iSecondLevelWidth;
	miThirdLevelWidth = iThirdLevelWidth;
	miNumSecondLevelChunks = iNumSecondLevelChunks; 
	miNumThirdLevelChunks = iNumThirdLevelChunks;

	iNumDescriptors = mpcDescriptorsFile->NumDescriptors();

	if (iNumDescriptors > iNumThirdLevelChunks)
	{
		mcHuge.Init(mpcDescriptorsFile, mpcIndexedData, bDirtyTesting, iSecondLevelWidth, iThirdLevelWidth, iNumSecondLevelChunks, iNumThirdLevelChunks);
		pcAccess = &mcHuge;
	}
	else
	{
		mcMemory.Init(mpcDescriptorsFile, bDirtyTesting, iNumThirdLevelChunks);
		pcAccess = &mcMemory;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexSmartAccess::Kill(void)
{
	pcAccess->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexSmartAccess::Get(CIndexDescriptor* pcDescriptor, OIndex oi)
{
	return pcAccess->Get(pcDescriptor, oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexSmartAccess::Set(CIndexDescriptor* pcDescriptor)
{
	if (pcAccess->IsMemory())
	{
		if (pcDescriptor->GetIndex() >= miThirdLevelWidth)
		{
			SwitchToHuge();
		}
	}
	return pcAccess->Set(pcDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexSmartAccess::Remove(OIndex oi)
{
	return pcAccess->Remove(oi);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CIndexSmartAccess::Length(void)
{
	return pcAccess->Length();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexSmartAccess::Load(void)
{
	pcAccess->Load();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexSmartAccess::Save(void)
{
	pcAccess->Save();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexSmartAccess::UpdateFile(void)
{
	return pcAccess->UpdateFile();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CIndexAccess* CIndexSmartAccess::GetAccess(void)
{
	return pcAccess;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexSmartAccess::SwitchToHuge(void)
{
	if (pcAccess->IsMemory())
	{
		mcHuge.Init(mpcDescriptorsFile, mpcIndexedData, mcMemory.mbDirtyTesting, miSecondLevelWidth, miThirdLevelWidth, miNumSecondLevelChunks, miNumThirdLevelChunks);
		mcHuge.Set(mcMemory.maMemoryArray.GetData(), mcMemory.maMemoryArray.NumElements());

		pcAccess = &mcHuge;
		mcMemory.Kill();
	}
}
