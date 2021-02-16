/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#include "Logger.h"
#include "ConstructorCall.h"
#include "PackFiles.h"
#include "PackFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFile::Init(CPackFiles* pcPackFiles, CPackFileNode* psPackFileNode)
{
	mpcPackFiles = pcPackFiles;
	mpsPackFileNode = psPackFileNode;
	miPos = 0;
	meMode = EFM_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPackFile::Kill(void)
{
	//You need to free mpsPackFileNode
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFile::Open(EFileMode eFileMode)
{
	if (eFileMode == EFM_Write_Create)
	{
		if (mpcPackFiles->IsWriteMode())
		{
			meMode = EFM_Write_Create;
			return TRUE;
		}
	}
	else if (eFileMode == EFM_Read)
	{
		if (mpcPackFiles->IsReadMode())
		{
			meMode = EFM_Read;
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFile::Close(void)
{
	if (meMode != EFM_Unknown)
	{
		meMode = EFM_Unknown;
		return mpcPackFiles->Close(this);
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
filePos CPackFile::Read(void* pvBuffer, filePos iSize, filePos iCount)
{
	if (meMode == EFM_Read)
	{
		return mpcPackFiles->Read(mpsPackFileNode, pvBuffer, iSize, iCount);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFile::Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin)
{
	if (meMode != EFM_Unknown)
	{
		return mpcPackFiles->Seek(mpsPackFileNode, iOffset, iSeekOrigin);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CPackFile::Write(const void* pvBuffer, filePos iSize, filePos iCount)
{
	if (meMode == EFM_Write_Create)
	{
		return mpcPackFiles->Write(mpsPackFileNode, pvBuffer, iSize, iCount);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CPackFile::Tell(void)
{
	if (meMode != EFM_Unknown)
	{
		return mpcPackFiles->Tell(mpsPackFileNode);
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFile::Eof(void)
{
	if (meMode != EFM_Unknown)
	{
		return mpcPackFiles->Eof(mpsPackFileNode);
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFile::IsOpen(void)
{
	return meMode != EFM_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CPackFile::Size(void)
{
	return mpsPackFileNode->Size();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFile::Truncate(filePos iSize)
{
	return gcLogger.Error2(__METHOD__, " Cannot truncate Pack Files.", NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFile::Flush(void)
{
	if (meMode == EFM_Write_Create)
	{
		return mpcPackFiles->Flush();
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPackFile::Delete(void)
{
	if (IsOpen())
	{
		return FALSE;
	}

	//return mpcPackFiles->Remove(this);
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CPackFile::GetFileName(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CPackFile* PackFile(CPackFiles* pcPackFiles, CPackFileNode* psPackFile)
{
	CPackFile*	pcPackFile;

	pcPackFile = NewMalloc<CPackFile>();
	pcPackFile->Init(pcPackFiles, psPackFile);
	pcPackFile->mbBasicFileMustFree = TRUE;
	return pcPackFile;
}

