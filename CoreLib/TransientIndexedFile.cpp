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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/Chars.h"
#include "TransientIndexedFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransientIndexedFile::Init(char* szDirectory, unsigned int uiCacheSize)
{
	mcFiles.Init(128);
	mszDirectory.Init(szDirectory);
	mcCache.Init(uiCacheSize, sizeof(SOIndexIndexCacheDescriptor));
	mcPointers.Init(8192);
	mbCreatedDirectory = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransientIndexedFile::Kill(void)
{
	CFileUtil							cFileUtil;
	CTransientIndexedFileDescriptor*	pcFile;
	int									i;

	mcPointers.Kill();
	mcCache.Kill();

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcFile = mcFiles.Get(i);
		pcFile->Kill();
	}
	mcFiles.Kill();

	if (mbCreatedDirectory)
	{
		cFileUtil.RemoveDir(mszDirectory.Text());
	}

	mszDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::Add(OIndex oi, void* pvData, unsigned int uiSize)
{
	int							iIndex;
	STransientIndexedPointer*	psTransientIndexedPointer;
	BOOL						bExists;
	void*						pvCache;
	BOOL						bResult;

	bExists = mcPointers.Get(oi, &psTransientIndexedPointer, &iIndex);
	if (!bExists)
	{
		bResult = PrivateAdd(oi, uiSize, &pvCache, iIndex);
		if (bResult)
		{
			memcpy_fast(pvCache, pvData, uiSize);
		}
		return bResult;
	}
	else
	{
		if (psTransientIndexedPointer->IsRemoved())
		{
			return Set(oi, pvData, uiSize);
		}
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::PrivateAdd(OIndex oi, unsigned int uiSize, void** ppvData, int iIndex)
{
	STransientIndexedPointer*	psTransientIndexedPointer;
	BOOL						bResult;

	
	psTransientIndexedPointer = mcPointers.Add(oi, iIndex);
	psTransientIndexedPointer->Init(oi, uiSize);
	RemapCacheOIndexIndices(iIndex);

	bResult = Allocate(psTransientIndexedPointer, iIndex);
	if (!bResult)
	{
		return FALSE;
	}
	*ppvData = psTransientIndexedPointer->pvCache;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::GetDetail(OIndex oi, void** ppvData, unsigned int* puiSize)
{
	int		iIndex;

	return PrivateGetDetail(oi, ppvData, puiSize, &iIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::PrivateGetDetail(OIndex oi, void** ppvData, unsigned int* puiSize, int* piIndex)
{
	STransientIndexedPointer*	psPointer;
	BOOL						bResult;

	bResult = mcPointers.Get(oi, &psPointer, piIndex);
	if (!bResult)
	{
		return FALSE;
	}

	if (psPointer->IsRemoved())
	{
		*puiSize = 0;
		*ppvData = NULL;
		return TRUE;
	}

	*puiSize = psPointer->sIndexedMemory.uiSize;
	*ppvData = PrivateGet(*piIndex);
	if (*ppvData != NULL)
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CTransientIndexedFile::PrivateGet(int iIndex)
{
	STransientIndexedPointer*	psPointer;
	BOOL						bResult;

	mcPointers.GetDirect(iIndex, &psPointer);

	if (psPointer->pvCache)
	{
		return psPointer->pvCache;
	}
	else
	{
		bResult = Read(psPointer, iIndex);
		if (bResult)
		{
			return psPointer->pvCache;
		}
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::Get(OIndex oi, void* pvDest)
{
	void*			pvData;
	unsigned int	uiSize;
	BOOL			bResult;

	bResult = GetDetail(oi, &pvData, &uiSize);
	if (bResult)
	{
		memcpy_fast(pvDest, pvData, uiSize);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned int CTransientIndexedFile::Size(OIndex oi)
{
	void*			pvData;
	unsigned int	uiSize;
	BOOL			bResult;

	bResult = GetDetail(oi, &pvData, &uiSize);
	if (bResult)
	{
		return uiSize;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::Remove(OIndex oi)
{
	STransientIndexedPointer*		psPointer;
	int								iIndex;
	BOOL							bExists;
	SOIndexIndexCacheDescriptor*	psCacheDesc;

	//This method looks wrong.
	bExists = mcPointers.Get(oi, &psPointer, &iIndex);
	if (bExists)
	{
		if (psPointer->IsRemoved())
		{
			return FALSE;
		}
		else
		{
			if (psPointer->pvCache)
			{
				psCacheDesc = (SOIndexIndexCacheDescriptor*)RemapSinglePointer(psPointer->pvCache, -(int)sizeof(SOIndexIndexCacheDescriptor));
				mcCache.Invalidate(psCacheDesc);
			}
			psPointer->sIndexedMemory.uiSize = 0;
			return Allocate(psPointer, iIndex);
		}
	}
	else
	{
		return AddRemoved(oi);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::AddRemoved(OIndex oi)
{
	int								iIndex;
	STransientIndexedPointer*		psTransientIndexedPointer;
	BOOL							bExists;
	void*							pvCache;

	bExists = mcPointers.Get(oi, &psTransientIndexedPointer, &iIndex);
	if (!bExists)
	{
		return PrivateAdd(oi, 0, &pvCache, iIndex);
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
BOOL CTransientIndexedFile::Set(OIndex oi, void* pvData, unsigned int uiSize)
{
	STransientIndexedPointer*		psPointer;
	SOIndexIndexCacheDescriptor*	psCacheDesc;
	void*							pvCache;
	int								iPointerIndex;
	BOOL							bExists;
	int								iIndex;
	BOOL							bResult;

	bExists = mcPointers.Get(oi, &psPointer, &iIndex);
	if (!bExists)
	{
		return FALSE;
	}

	if (uiSize > 0)
	{
		if (uiSize != psPointer->sIndexedMemory.uiSize)
		{
			if (psPointer->pvCache)
			{
				psCacheDesc = (SOIndexIndexCacheDescriptor*)RemapSinglePointer(psPointer->pvCache, -(int)sizeof(SOIndexIndexCacheDescriptor));
				mcCache.Invalidate(psCacheDesc);
			}
			psPointer->sIndexedMemory.uiSize = uiSize;
			bResult = Allocate(psPointer, iIndex);
			if (bResult)
			{
				pvCache = psPointer->pvCache;
				memcpy_fast(pvCache, pvData, uiSize);
				return TRUE;
			}
			return FALSE;
		}
		else
		{
			if (psPointer->pvCache)
			{
				pvCache = psPointer->pvCache;
			}
			else
			{
				bResult = Allocate(psPointer, iIndex);
				if (!bResult)
				{
					return FALSE;
				}
				pvCache = psPointer->pvCache;
			}
		}

		iPointerIndex = mcPointers.GetIndex(psPointer);

		if (pvCache)
		{
			psCacheDesc = (SOIndexIndexCacheDescriptor*)RemapSinglePointer(pvCache, -(int)(sizeof(SOIndexIndexCacheDescriptor)));
			psCacheDesc->sIndex.iIndex = iPointerIndex;
			psCacheDesc->sIndex.oi = psPointer->sIndexedMemory.oi;

			memcpy_fast(pvCache, pvData, uiSize);

			psPointer->Init(oi, uiSize);
			psPointer->pvCache = pvCache;
			return TRUE;
		}
		return FALSE;
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
BOOL CTransientIndexedFile::Allocate(STransientIndexedPointer* psPointer, int iPointerIndex)
{
	SOIndexIndexCacheDescriptor*	psOIndexIndex;
	CMemoryCacheAllocation			cPreAllocated;
	int								i;
	void*							pvData;

	cPreAllocated.Init(psPointer->sIndexedMemory.uiSize);
	if (!mcCache.PreAllocate(&cPreAllocated))
	{
		cPreAllocated.Kill();
		return FALSE;
	}

	for (i = 0; i < cPreAllocated.NumElements(); i++)
	{
		psOIndexIndex = (SOIndexIndexCacheDescriptor*)cPreAllocated.Get(i);
		pvData = RemapSinglePointer(psOIndexIndex, sizeof(SOIndexIndexCacheDescriptor));
		Write(psOIndexIndex->sIndex.iIndex, pvData);
	}

	pvData = mcCache.Allocate(&cPreAllocated);

	if (pvData)
	{
		psOIndexIndex = (SOIndexIndexCacheDescriptor*)RemapSinglePointer(pvData, -(int)(sizeof(SOIndexIndexCacheDescriptor)));
		psOIndexIndex->sIndex.iIndex = iPointerIndex;
		psOIndexIndex->sIndex.oi = psPointer->sIndexedMemory.oi;
		psPointer->pvCache = pvData;
		cPreAllocated.Kill();
		return TRUE;
	}
	else
	{
		cPreAllocated.Kill();
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::Read(STransientIndexedPointer* psPointer, int iPointerIndex)
{
	CTransientIndexedFileDescriptor*	pcFileDesc;

	if (psPointer->iFileIndex == -1)
	{
		return FALSE;
	}

	if (!Allocate(psPointer, iPointerIndex))
	{
		return FALSE;
	}

	pcFileDesc = mcFiles.Get(psPointer->iFileIndex);
	return pcFileDesc->Read(psPointer->iIndexInFile, psPointer->pvCache);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::WriteNew(STransientIndexedPointer* psPointer, void* pvData)
{
	CTransientIndexedFileDescriptor*	pcFile;
	filePos								iIndex;

	pcFile = GetOrCreateFile(psPointer->sIndexedMemory.uiSize);

	iIndex = pcFile->Write(pvData);
	if (iIndex == -1)
	{
		return FALSE;
	}

	psPointer->iFileIndex = pcFile->miFileIndex;
	psPointer->iIndexInFile = iIndex;
	return TRUE;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::WriteExisting(STransientIndexedPointer* psPointer, void* pvData)
{
	CTransientIndexedFileDescriptor*	pcFile;

	pcFile = GetFile(psPointer->iFileIndex);
	if (pcFile)
	{
		return pcFile->Write(psPointer->iIndexInFile, pvData);
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
BOOL CTransientIndexedFile::Write(int iPointerIndex, void* pvData)
{
	STransientIndexedPointer*	psPointer;
	CFileUtil					cFileUtil;

	if (!mbCreatedDirectory)
	{
		cFileUtil.MakeDir(mszDirectory.Text());
		mbCreatedDirectory = TRUE;
	}

	mcPointers.GetDirect(iPointerIndex, &psPointer);
	psPointer->pvCache = NULL;
	if (psPointer->iFileIndex == -1)
	{
		return WriteNew(psPointer, pvData);
	}
	else
	{
		return WriteExisting(psPointer, pvData);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransientIndexedFile::RemapCacheOIndexIndices(int iInsertedIndex)
{
	SOIndexIndexCacheDescriptor*	psCache;
	SOIndexIndexCacheDescriptor*	psInitial;

	psCache = (SOIndexIndexCacheDescriptor*)mcCache.GetFirst();
	psInitial = psCache;
	if (psInitial)
	{
		for (;;)
		{
			if (psCache->sIndex.iIndex >= iInsertedIndex)
			{
				psCache->sIndex.iIndex++;
			}
			psCache = (SOIndexIndexCacheDescriptor*)mcCache.GetNext(psCache);

			if (psCache == psInitial)
			{
				break;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::TestOrder(void)
{
	int							i;
	OIndex						iLast;
	STransientIndexedPointer*	psTransientIndexedPointer;

	iLast = -1;

	for (i = 0; i < mcPointers.NumElements(); i++)
	{
		mcPointers.GetDirect(i, &psTransientIndexedPointer);
		if (psTransientIndexedPointer->sIndexedMemory.oi <= iLast)
		{
			return FALSE;
		}
		iLast = psTransientIndexedPointer->sIndexedMemory.oi;
	}
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTransientIndexedFileDescriptor* CTransientIndexedFile::GetOrCreateFile(int iDataSize)
{
	int									i;
	CTransientIndexedFileDescriptor*	pcFile;
	int									iNumFiles;
	char								szFileName[65536];

	iNumFiles = 0;
	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcFile = mcFiles.Get(i);
		if (pcFile->miDataSize == iDataSize)
		{
			if (!pcFile->IsFull())
			{
				return pcFile;
			}
			else
			{
				iNumFiles++;
			}
		}
	}

	pcFile = mcFiles.Add();
	new (pcFile) CTransientIndexedFileDescriptor();
	DataFileName(szFileName, iDataSize, iNumFiles);
	pcFile->Init(mcFiles.NumElements()-1, szFileName, iDataSize, iNumFiles);
	return pcFile;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTransientIndexedFileDescriptor* CTransientIndexedFile::GetFile(int iDataSize, int iFileNum)
{
	int									i;
	CTransientIndexedFileDescriptor*	pcFile;

	for (i = 0; i < mcFiles.NumElements(); i++)
	{
		pcFile = mcFiles.Get(i);
		if (pcFile->miDataSize == iDataSize)
		{
			if (pcFile->miFileNumber == iFileNum)
			{
				return pcFile;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTransientIndexedFileDescriptor* CTransientIndexedFile::GetFile(int iFileIndex)
{
	CTransientIndexedFileDescriptor*	pcFile;

	pcFile = mcFiles.SafeGet(iFileIndex);
	if (!pcFile)
	{
		return NULL;
	}
	if (pcFile->miFileIndex != iFileIndex)
	{
		return NULL;
	}
	return pcFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CTransientIndexedFile::DataFileName(char* szFile1, int iDataSize, int iFileNum)
{
	CChars	szFileName;

	szFileName.Init(mszDirectory);
	szFileName.Append(FILE_SEPARATOR);
	szFileName.Append(iDataSize);
	szFileName.Append("_");
	szFileName.Append(iFileNum);
	szFileName.Append(".DAT");

	if (szFileName.Length() < 65536)
	{
		strcpy(szFile1, szFileName.Text());
		szFileName.Kill();
		return TRUE;
	}

	szFileName.Kill();
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SIndexedMemory* CTransientIndexedFile::GetIndexedData(int iIndexedDataIndex)
{
	STransientIndexedPointer*	psPointer;

	mcPointers.GetDirect(iIndexedDataIndex, &psPointer);
	return &psPointer->sIndexedMemory;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CTransientIndexedFile::GetData(SIndexedMemory* psIndexedMemory)
{
	STransientIndexedPointer*	psPointer;
	int							iIndex;

	psPointer = (STransientIndexedPointer*)psIndexedMemory;
	if (psPointer->pvCache != NULL)
	{
		return psPointer->pvCache;
	}
	else
	{
		if (psPointer->IsRemoved())
		{
			return NULL;
		}
		iIndex = mcPointers.GetIndex(psPointer);
		return PrivateGet(iIndex);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CTransientIndexedFile::NumDatas(void)
{
	return mcPointers.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CTransientIndexedFile::Dump(void)
{
	int					iNumElements;
	int					i;
	SIndexedMemory*	psIndexedMemory;
	void*				pvDest;
	CChars				sz;

	sz.Init();
	iNumElements = NumDatas();

	sz.Append("Transaction Data (");
	sz.Append(iNumElements);
	sz.Append(") (Huge)\n-----------------\n");

	for (i = 0; i < iNumElements; i++)
	{
		psIndexedMemory = GetIndexedData(i);
		pvDest = GetData(psIndexedMemory);

		sz.Append("Index[");
		sz.Append((int)psIndexedMemory->oi);
		sz.Append("] Size[");
		sz.Append((int)psIndexedMemory->uiSize);
		sz.Append("] ");
		if (psIndexedMemory->uiSize > 0)
		{
			sz.Append("\"");
			sz.AppendData((char*)pvDest, 80);
			sz.Append("\" ");
		}
		if (psIndexedMemory->IsRemoved())
		{
			sz.Append("(Removed)");
		}
		sz.Append("\n");
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}
