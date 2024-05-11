/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
#include <stdio.h>
#include "BufferedFile.h"
#include "IntegerHelper.h"
#include "FileUtil.h"
#include "ConstructorCall.h"
#include "PointerFunctions.h"


//////////////////////////////////////////////////////////////////////////
//
// 
//////////////////////////////////////////////////////////////////////////
void CBufferedFile::Init(CAbstractFile* pcFile)
{
	Init(pcFile, BUFFERED_FILE_CACHE_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBufferedFile::Init(CAbstractFile* pcFile, size iBufferSize)
{
	CAbstractFile::Init();
	mpcFile = pcFile;
	mpvMem = malloc(iBufferSize);
	if (mpvMem)
	{
		miAllocatedSize = iBufferSize;
	}
	else
	{
		mpvMem = NULL;
		miAllocatedSize = 0;
	}
	miBufferPos = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBufferedFile::Kill(void)
{
	SafeFree(mpvMem);
	if (mpcFile->mbBasicFileMustFree)
	{
		SafeKill(mpcFile);
	}
	mpcFile = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBufferedFile::Open(EFileMode eFileMode)
{
	bool	bResult;

	bResult = mpcFile->Open(eFileMode);
	meType = BT_Empty;
	meLastOp = BLO_None;
	mulliFilePos = 0;
	mulliCurrentPos = 0;
	mulliLength = mpcFile->Size();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBufferedFile::Close(void)
{
	bool	iResult;

	WriteUnwritten();
	iResult = mpcFile->Close();
	return iResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBufferedFile::Read(void* pvDest, size iSize, size iCount)
{
	size		iRemainingBuffer;
	size		iByteSize;
	size		iBytesRemaining;
	size		iBytesCopied;
	size		iResult;

	WriteUnwritten();

	if (meLastOp == BLO_Write)
	{
		mpcFile->Seek(mulliCurrentPos, EFSO_SET);
		meLastOp = BLO_Seek;
	}

	iByteSize = iSize * iCount;
	if (iByteSize > miAllocatedSize)
	{
		MatchFilePosToCurrentPos();
		iResult = mpcFile->Read(pvDest, iSize, iCount);
		meLastOp = BLO_Read;
		mulliFilePos += iResult * iSize;
		mulliCurrentPos = mulliFilePos;
		meType = BT_Empty;
		return iResult;
	}
	else
	{
		if (meType == BT_Read)
		{
			iRemainingBuffer = miBufferSizeRead - miBufferPos;
			if (iByteSize <= iRemainingBuffer)
			{
				CopyFromBuffer(pvDest, iByteSize, 0);
				return iCount;
			}
			else
			{
				iBytesCopied = iRemainingBuffer;

				CopyFromBuffer(pvDest, iBytesCopied, 0);

				BufferSourceFileRead();

				iBytesRemaining = iByteSize - iBytesCopied;
				if (miBufferSizeRead < iBytesRemaining)
				{
					iBytesRemaining = miBufferSizeRead;
				}

				CopyFromBuffer(pvDest, iBytesRemaining, iBytesCopied);

				iBytesCopied += iBytesRemaining;
				return iBytesCopied / iSize;
			}
		}
		else
		{
			BufferSourceFileRead();

			if (miBufferSizeRead < iByteSize)
			{
				iByteSize = miBufferSizeRead;
			}

			CopyFromBuffer(pvDest, iByteSize, 0);
			return iByteSize / iSize;			
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBufferedFile::Write(const void* pvSource, size iSize, size iCount)
{
	size		iRemainingBuffer;
	size		iByteSize;
	size		iResult;
	bool		bResult;
	size		iSourceRemaining;

	if (meLastOp == BLO_Read)
	{
		mpcFile->Seek(mulliCurrentPos, EFSO_SET);
		meLastOp = BLO_Seek;
	}

	iByteSize = iSize * iCount;
	if (iByteSize > miAllocatedSize)
	{
		WriteUnwritten();
		iResult = mpcFile->Write(pvSource, iSize, iCount);
		meLastOp = BLO_Write;
		mulliFilePos += iResult * iSize;
		mulliCurrentPos = mulliFilePos;
		meType = BT_Empty;
		return iResult;
	}
	else
	{
		if (meType == BT_Write)
		{
			iRemainingBuffer = miAllocatedSize - miBufferPos;
			if (iByteSize <= iRemainingBuffer)
			{
				memcpy(RemapSinglePointer(mpvMem, miBufferPos), pvSource, iByteSize);
				mulliCurrentPos += iByteSize;
				miBufferPos += iByteSize;
				if (mulliCurrentPos > mulliLength)
				{
					mulliLength = mulliCurrentPos;
				}
				return iCount;
			}
			else
			{
				memcpy(RemapSinglePointer(mpvMem, miBufferPos), pvSource, iRemainingBuffer);
				
				miBufferPos = miAllocatedSize;
				bResult = WriteUnwritten();

				iSourceRemaining = iByteSize - iRemainingBuffer;
				memcpy(mpvMem, RemapSinglePointer(pvSource, iRemainingBuffer), iSourceRemaining);
				miBufferPos = iSourceRemaining;
				meType = BT_Write;

				mulliCurrentPos += iByteSize;
				if (mulliCurrentPos > mulliLength)
				{
					mulliLength = mulliCurrentPos;
				}

				if (!bResult)
				{
					iCount = 0;
				}
				return iCount;
			}
		}
		else
		{
			memcpy(mpvMem, pvSource, iByteSize);
			mulliCurrentPos += iByteSize;
			miBufferPos = iByteSize;
			if (mulliCurrentPos > mulliLength)
			{
				mulliLength = mulliCurrentPos;
			}
			meType = BT_Write;
			return iCount;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBufferedFile::Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin)
{
	bool	bResult;

	WriteUnwritten();
	bResult = mpcFile->Seek(iOffset, iSeekOrigin);
	meLastOp = BLO_Seek;
	mulliCurrentPos = mpcFile->Tell();
	mulliFilePos = mulliCurrentPos;
	meType = BT_Empty;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CBufferedFile::Tell(void)
{
	return mulliCurrentPos;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBufferedFile::IsOpen(void)
{
	return mpcFile->IsOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CBufferedFile::Size(void)
{
	return mulliLength;
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBufferedFile::Truncate(filePos iSize)
{
	bool	bResult;
	bResult = WriteUnwritten();
	bResult &= mpcFile->Truncate(iSize);
	if (bResult)
	{
		mulliFilePos = iSize;
		mulliCurrentPos = iSize;
		bResult = mpcFile->Seek(iSize, EFSO_SET);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBufferedFile::Flush(void)
{
	bool	bResult;

	bResult = WriteUnwritten();
	bResult &= mpcFile->Flush();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CBufferedFile::Delete(void)
{
	if (IsOpen())
	{
		return false;
	}

	return mpcFile->Delete();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CBufferedFile::GetFilename(void)
{
	if (mpcFile)
	{
		return mpcFile->GetFilename();
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBufferedFile::MatchFilePosToCurrentPos(void)
{
	if (mulliFilePos != mulliCurrentPos)
	{
		mpcFile->Seek(mulliCurrentPos, EFSO_SET);
		meLastOp = BLO_Seek;
		mulliFilePos = mulliCurrentPos;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBufferedFile::BufferSourceFileRead(void)
{
	size	iResult;

	MatchFilePosToCurrentPos();
	iResult = mpcFile->Read(mpvMem, 1, miAllocatedSize);
	meLastOp = BLO_Read;
	mulliFilePos += iResult;
	miBufferPos = 0;
	miBufferSizeRead = iResult;
	meType = BT_Read;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBufferedFile::CopyFromBuffer(void* pvDest, size iByteSize, size iDestOffset)
{
	if (iByteSize > 0)
	{
		memcpy(RemapSinglePointer(pvDest, iDestOffset), RemapSinglePointer(mpvMem, miBufferPos), iByteSize);
		mulliCurrentPos += iByteSize;
		miBufferPos += iByteSize;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBufferedFile::WriteUnwritten(void)
{
	size	iResult;

	if (meType == BT_Write)
	{
		iResult = mpcFile->Write(mpvMem, miBufferPos, 1);
		meLastOp = BLO_Write;
		if (iResult == 1)
		{
			mulliFilePos += miBufferPos;
		}
		miBufferPos = 0;
		meType = BT_Empty;
		return iResult == 1;
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBufferedFile* BufferedFile(CAbstractFile* pcFile)
{
	return BufferedFile(pcFile, BUFFERED_FILE_CACHE_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBufferedFile* BufferedFile(CAbstractFile* pcFile, size iBufferSize)
{
	CBufferedFile* pCBufferedFile;

	pCBufferedFile = NewMalloc<CBufferedFile>();
	pCBufferedFile->Init(pcFile, iBufferSize);
	pCBufferedFile->mbBasicFileMustFree = true;
	return pCBufferedFile;
}

