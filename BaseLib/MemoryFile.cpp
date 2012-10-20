/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include <string.h>
#include "IntegerHelper.h"
#include "ConstructorCall.h"
#include "MemoryFile.h"
#define MEMORY_FILE_CHUNK_SIZE	4096


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryFile::Init(void)
{
	Init(NULL, 0);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryFile::Init(void* pvInitialMem, int iInitialLength)
{
	CAbstractFile::Init();
	cArray.Init(MEMORY_FILE_CHUNK_SIZE);
	mbOpen = FALSE;
	mpvInitialMem = pvInitialMem;
	miInitialLength = iInitialLength;
	mbFakeArray = FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryFile::Kill(void)
{
	mbOpen = FALSE;
	if (!mbFakeArray)
	{
		cArray.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryFile::Open(EFileMode eFileMode)
{
	if ((eFileMode == EFM_ReadWrite) || (eFileMode == EFM_Write_Create) || (eFileMode == EFM_ReadWrite_Create))
	{
		mbFakeArray = FALSE;
		iFlags = _IOREAD | _IOWRT;
		if (miInitialLength != 0)
		{
			cArray.InsertBlockAt((char*)mpvInitialMem, miInitialLength, 0);
		}
		iPos = 0;
	}
	else if ((eFileMode == EFM_Read) || (eFileMode == EFM_Read_Create))
	{
		mbFakeArray = TRUE;
		iFlags = _IOREAD;
		if (miInitialLength != 0)
		{
			cArray.ReInit();
			cArray.Fake((char*)mpvInitialMem, miInitialLength);
		}
		cArray.SetAllocateSize(MEMORY_FILE_CHUNK_SIZE);
		iPos = 0;
	}
	else
	{
		return FALSE;
	}
	mbOpen = TRUE;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryFile::Close(void)
{
	if (mbOpen)
	{
		mbOpen = FALSE;
		if (iFlags & _IOWRT)
		{
			return TRUE;
		}
		if (iFlags & _IOREAD)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
filePos CMemoryFile::Read(void* pvBuffer, filePos iSize, filePos iCount)
{
	filePos	iAmountToCopy;
	filePos iAmountToRead;
	filePos	iNumCopied;
	filePos	iOldPos;

	if (iFlags & _IOREAD)
	{
		iOldPos = iPos;
		iAmountToRead = (int)(iSize * iCount);
		iAmountToCopy = (cArray.NumElements() - iPos);

		if (iAmountToRead > iAmountToCopy)
		{
			iNumCopied = iAmountToCopy / (int)iSize;
			iAmountToCopy = iNumCopied * iSize;
			iFlags |= _IOEOF;
			iPos = cArray.NumElements();
		}
		else
		{
			iAmountToCopy = iSize * iCount;
			iNumCopied = (int)iCount;
			iPos += iAmountToCopy;

			if (iPos == cArray.NumElements())
			{
				iFlags |= _IOEOF;
			}
			else
			{
				iFlags &= ~_IOEOF;
			}
		}
		memcpy(pvBuffer, cArray.Get((int)iOldPos), (int)iAmountToCopy);
		return (int)iNumCopied;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryFile::Seek(filePos iOffset, EFileSeekOrigin iSeekOrigin)
{
	if (iSeekOrigin == EFSO_SET)
	{
		iPos = iOffset;
	}
	else if (iSeekOrigin == EFSO_END)
	{
		iPos = cArray.NumElements() + iOffset;
	}
	else if (iSeekOrigin == EFSO_CURRENT)
	{
		iPos += iOffset;
	}

	if (iPos > cArray.NumElements())
	{
		iPos = cArray.NumElements();  //This is not the same behavior as file...
	}
	if (iPos < 0)
	{
		iPos = 0;
	}

	if (iPos == cArray.NumElements())
	{
		iFlags |= _IOEOF;
	}
	else
	{
		iFlags &= ~_IOEOF;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
filePos CMemoryFile::Write(const void* pvBuffer, filePos iSize, filePos iCount)
{
	filePos iAmountToCopy;
	filePos	iAmountToAdd;

	if (iFlags & _IOWRT)
	{
		iAmountToCopy = iSize * iCount;
		iAmountToAdd = iAmountToCopy - (cArray.NumElements() - iPos);
		if (iAmountToAdd > 0)
		{
			cArray.GrowByNumElements((int)iAmountToAdd);
			if (cArray.IsEmpty())
			{
				return 0;
			}
		}

		memcpy(cArray.Get((int)iPos), pvBuffer, (int)iAmountToCopy);
		iPos += iAmountToCopy;

		if (iPos == cArray.NumElements())
		{
			iFlags |= _IOEOF;
		}
		else
		{
			iFlags &= ~_IOEOF;
		}
		return iCount;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
filePos CMemoryFile::Tell(void)
{
	return iPos;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CMemoryFile::Eof(void)
{
	return FixBool(iFlags & _IOEOF);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CMemoryFile::IsOpen(void)
{
	return mbOpen;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
filePos CMemoryFile::Size(void)
{
	return cArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryFile::Flush(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryFile::GetBufferPointer(void)
{
	return (void*)cArray.GetData();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CMemoryFile::GetBufferSize(void)
{
	return cArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryFile::SetBufferPointer(void* pvBuffer)
{
	cArray.Fake((char*)pvBuffer, cArray.NumElements());
	cArray.SetAllocateSize(MEMORY_FILE_CHUNK_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryFile::SetBufferSize(int iBufferSize)
{
	cArray.Fake(cArray.GetData(), iBufferSize);
	cArray.SetAllocateSize(MEMORY_FILE_CHUNK_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMemoryFile* MemoryFile(void)
{
	CMemoryFile* pcMemoryFile;

	pcMemoryFile = Malloc(CMemoryFile);
	pcMemoryFile->Init();
	pcMemoryFile->mbBasicFileMustFree = TRUE;
	return pcMemoryFile;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMemoryFile* MemoryFile(void* pvInitialMem, int iInitialLength)
{
	CMemoryFile*	pcMemoryFile;

	pcMemoryFile = Malloc(CMemoryFile)
	pcMemoryFile->Init(pvInitialMem, iInitialLength);
	pcMemoryFile->mbBasicFileMustFree = TRUE;
	return pcMemoryFile;
}

