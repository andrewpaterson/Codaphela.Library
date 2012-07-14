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
#include "BaseLib/DiskFile.h"
#include "TransientIndexedFileDescriptor.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransientIndexedFileDescriptor::Init(int iFileIndex, char* szFileName, int iDataSize, int iFileNum)
{
	int		iFileLengh;

	miFileIndex = iFileIndex;

	mszFileName.Init(szFileName);
	miDataSize = iDataSize;
	miFileNumber = iFileNum;

	mcFile.Init(DiskFile(mszFileName.Text()));
	mcFile.Open(EFM_ReadWrite_Create);

	iFileLengh = 0;
	miNumDatas = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransientIndexedFileDescriptor::Kill(void)
{
	CFileUtil	cFileUtil;

	miNumDatas = 0;
	mcFile.Close();
	mcFile.Kill();
	cFileUtil.Delete(mszFileName.Text());
	mszFileName.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFileDescriptor::IsFull(void)
{
	if (miNumDatas >= 0xffffff)  //16 million.
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTransientIndexedFileDescriptor::Write(void* pvData)
{
	filePos	iIndex;
	int		iWritten;

	iIndex = mcFile.GetFileSize();
	if ((iIndex % miDataSize) != 0)
	{
		return -1;
	}
	iWritten = mcFile.Write(pvData, miDataSize, 1);
	miNumDatas++;

	if (iWritten != 1)
	{
		return -1;
	}

	return (int)(iIndex / miDataSize);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFileDescriptor::Write(int iIndex, void* pvData)
{
	int				iWritten;
	filePos			iFileLengh;
	unsigned int	iPosition;

	iPosition = iIndex * miDataSize;
	mcFile.Seek(iPosition);
	iWritten = mcFile.Write(pvData, miDataSize, 1);
	if (iWritten != 1)
	{
		return FALSE;
	}

	if (iIndex > miNumDatas)
	{
		iFileLengh = mcFile.GetFileSize();
		miNumDatas = (int)(iFileLengh / miDataSize);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFileDescriptor::Read(int iIndex, void* pvData)
{
	int				iRead;
	unsigned int	iPosition;

	if (iIndex > miNumDatas)
	{
		return FALSE;
	}

	iPosition = iIndex*miDataSize;
	mcFile.Seek(iPosition);
	iRead = mcFile.Read(pvData, miDataSize, 1);
	if (iRead != 1)
	{
		return FALSE;
	}
	return TRUE;
}

