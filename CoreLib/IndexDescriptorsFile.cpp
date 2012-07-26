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
#include "DurableFileController.h"
#include "IndexDescriptor.h"
#include "IndexDescriptorsFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexDescriptorsFile::Init(CDurableFileController* pcDurableFileControl, char* szFileName, char* szRewriteName)
{
	mpcDurableFileControl = pcDurableFileControl;
	mcIndexedDescriptorFile.Init(pcDurableFileControl->IsDurable(), szFileName, szRewriteName);
	pcDurableFileControl->AddFile(&mcIndexedDescriptorFile);
	mcIndexedDescriptorFile.Open();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIndexDescriptorsFile::Kill(void)
{
	mcIndexedDescriptorFile.Close();
	mcIndexedDescriptorFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CIndexDescriptorsFile::NumDescriptors(void)
{
	return mcIndexedDescriptorFile.Size() / sizeof(CIndexDescriptor);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CIndexDescriptorsFile::Read(CIndexDescriptor* pcDescriptor, int iPosition, int iNum)
{
	return mcIndexedDescriptorFile.Read(EFSO_SET, iPosition * sizeof(CIndexDescriptor), pcDescriptor, sizeof(CIndexDescriptor), iNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CIndexDescriptorsFile::Write(CIndexDescriptor* pcDescriptor, int iPosition, int iNum)
{
	return mcIndexedDescriptorFile.Write(iPosition * sizeof(CIndexDescriptor), pcDescriptor, sizeof(CIndexDescriptor), iNum);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDurableFile* CIndexDescriptorsFile::GetDurableFile(void)
{
	//This method only exists so the CIndexedHuge can work directly with the file.  It must not.
	return &mcIndexedDescriptorFile;
}


