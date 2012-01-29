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
#include "IndexMemoryAccess.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexMemoryAccess::Init(CIndexDescriptorsFile* pcDescriptorsFile, BOOL bDirtyTesting, int iChunkSize)
{
	CIndexAccess::Init(pcDescriptorsFile);
	mbDirtyTesting = bDirtyTesting;
	maMemoryArray.Init(iChunkSize);
	miChunkSize = iChunkSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexMemoryAccess::Kill(void)
{
	maMemoryArray.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
long long int CIndexMemoryAccess::Length(void)
{
	return (long long int)(maMemoryArray.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexMemoryAccess::Get(CIndexDescriptor* pcDescriptor, OIndex oi)
{
	CIndexDescriptor*	pcTemp;
	unsigned int		loInt;

	loInt = (unsigned int)oi;
	pcTemp = maMemoryArray.SafeGet(loInt);
	if (!pcTemp)
	{
		return FALSE;
	}
	memcpy_fast(pcDescriptor, pcTemp, sizeof(CIndexDescriptor));

	return pcDescriptor->IsAllocated();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexMemoryAccess::Set(CIndexDescriptor* pcDescriptor)
{
	unsigned int		loInt;
	CIndexDescriptor*	pcExistingDescriptor;

	loInt = (unsigned int)pcDescriptor->GetIndex();
	if (mbDirtyTesting)
	{
		pcExistingDescriptor = maMemoryArray.SafeGet(loInt);
		if ((!pcExistingDescriptor) || (!pcExistingDescriptor->IsAllocated()))
		{
			pcDescriptor->Dirty(TRUE);
		}
		else
		{
			if (pcExistingDescriptor->IsDirty())
			{
				pcDescriptor->Dirty(TRUE);
			}
			else if (!pcDescriptor->IsDirty())
			{
				if (memcmp(pcDescriptor, pcExistingDescriptor, sizeof(CIndexDescriptor)) != 0)
				{
					pcDescriptor->Dirty(TRUE);
				}
			}
		}

		if (pcDescriptor->IsDirty())
		{
			maMemoryArray.SafeSet(loInt, pcDescriptor);
			return TRUE;
		}
		else
		{
			return TRUE;
		}
	}
	else
	{
		maMemoryArray.SafeSet(loInt, pcDescriptor);
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexMemoryAccess::Remove(OIndex oi)
{
	unsigned int		loInt;
	CIndexDescriptor	cNullDescriptor;

	memset(&cNullDescriptor, 0, sizeof(CIndexDescriptor));
	cNullDescriptor.Init(oi, 0);

	loInt = (unsigned int)oi;
	if (maMemoryArray.NumElements() > (int)loInt)
	{
		maMemoryArray.Set(loInt, &cNullDescriptor);
	}		

	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexMemoryAccess::Load(void)
{
	int		iNumDescriptors;

	iNumDescriptors = (int)mpcDescriptorsFile->NumDescriptors();

	maMemoryArray.Kill();
	maMemoryArray.Allocate(miChunkSize, iNumDescriptors);

	mpcDescriptorsFile->Read(maMemoryArray.GetData(), 0, iNumDescriptors);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexMemoryAccess::Save(void)
{
	mpcDescriptorsFile->Write(maMemoryArray.GetData(), 0, maMemoryArray.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexMemoryAccess::UpdateFile(void)
{
	int					i;
	CIndexDescriptor*	pcDescriptor;
	BOOL				bResult;

	for (i = 0; i < maMemoryArray.NumElements(); i++)
	{
		pcDescriptor = maMemoryArray.Get(i);
		if (pcDescriptor->IsAllocated())
		{
			if ((!mbDirtyTesting) || (mbDirtyTesting && pcDescriptor->IsDirty()))
			{
				pcDescriptor->Dirty(FALSE);
				bResult = mpcDescriptorsFile->Write(pcDescriptor, i);
				if (!bResult)
				{
					return;
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CIndexMemoryAccess::IsMemory(void)
{
	return TRUE;
}

