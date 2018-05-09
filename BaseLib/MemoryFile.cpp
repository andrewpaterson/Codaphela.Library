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
#include "Define.h"
#include "IntegerHelper.h"
#include "ConstructorCall.h"
#include "MemoryFile.h"


#define MEMORY_FILE_READ_FLAG         0x0001
#define MEMORY_FILE_WRITE_FLAG        0x0002
#define MEMORY_FILE_EOF_FLAG          0x0010



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
	mcArray.Init();
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
		mcArray.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CMemoryFile::Open(EFileMode eFileMode)
{
	if (IsFileModeWritable(eFileMode))
	{
		mbFakeArray = FALSE;
		iFlags = MEMORY_FILE_READ_FLAG | MEMORY_FILE_WRITE_FLAG;
		if (miInitialLength != 0)
		{
			mcArray.InsertBlockAt((char*)mpvInitialMem, miInitialLength, 0);
		}
		iPos = 0;
	}
	else if (IsFileModeReadOnly(eFileMode))
	{
		mbFakeArray = TRUE;
		iFlags = MEMORY_FILE_READ_FLAG;
		if (miInitialLength != 0)
		{
			mcArray.ReInit();
			mcArray.Fake((char*)mpvInitialMem, miInitialLength);
		}
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
		if (iFlags & MEMORY_FILE_WRITE_FLAG)
		{
			return TRUE;
		}
		if (iFlags & MEMORY_FILE_READ_FLAG)
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

	if (iFlags & MEMORY_FILE_READ_FLAG)
	{
		iOldPos = iPos;
		iAmountToRead = (int)(iSize * iCount);
		iAmountToCopy = (mcArray.NumElements() - iPos);

		if (iAmountToRead > iAmountToCopy)
		{
			iNumCopied = iAmountToCopy / (int)iSize;
			iAmountToCopy = iNumCopied * iSize;
			iFlags |= MEMORY_FILE_EOF_FLAG;
			iPos = mcArray.NumElements();
		}
		else
		{
			iAmountToCopy = iSize * iCount;
			iNumCopied = (int)iCount;
			iPos += iAmountToCopy;

			if (iPos == mcArray.NumElements())
			{
				iFlags |= MEMORY_FILE_EOF_FLAG;
			}
			else
			{
				iFlags &= ~MEMORY_FILE_EOF_FLAG;
			}
		}
		memcpy(pvBuffer, mcArray.Get((int)iOldPos), (int)iAmountToCopy);
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
		iPos = mcArray.NumElements() + iOffset;
	}
	else if (iSeekOrigin == EFSO_CURRENT)
	{
		iPos += iOffset;
	}

	if (iPos > mcArray.NumElements())
	{
		iPos = mcArray.NumElements();  //This is not the same behavior as file...
	}
	if (iPos < 0)
	{
		iPos = 0;
	}

	if (iPos == mcArray.NumElements())
	{
		iFlags |= MEMORY_FILE_EOF_FLAG;
	}
	else
	{
		iFlags &= ~MEMORY_FILE_EOF_FLAG;
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

	if (iFlags & MEMORY_FILE_WRITE_FLAG)
	{
		iAmountToCopy = iSize * iCount;
		iAmountToAdd = iAmountToCopy - (mcArray.NumElements() - iPos);
		if (iAmountToAdd > 0)
		{
			mcArray.AddNum((int)iAmountToAdd);
			if (mcArray.IsEmpty())
			{
				return 0;
			}
		}

		memcpy(mcArray.Get((int)iPos), pvBuffer, (int)iAmountToCopy);
		iPos += iAmountToCopy;

		if (iPos == mcArray.NumElements())
		{
			iFlags |= MEMORY_FILE_EOF_FLAG;
		}
		else
		{
			iFlags &= ~MEMORY_FILE_EOF_FLAG;
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
	return FixBool(iFlags & MEMORY_FILE_EOF_FLAG);
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
	return mcArray.NumElements();
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
BOOL CMemoryFile::Delete(void)
{
	if (IsOpen())
	{
		return FALSE;
	}

	mcArray.ReInit();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CMemoryFile::GetFileName(void)
{
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void* CMemoryFile::GetBufferPointer(void)
{
	return mcArray.GetData();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CMemoryFile::GetBufferSize(void)
{
	return mcArray.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryFile::SetBufferPointer(void* pvBuffer)
{
	mcArray.Fake((char*)pvBuffer, mcArray.NumElements());
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CMemoryFile::SetBufferSize(int iBufferSize)
{
	mcArray.Fake(mcArray.GetData(), iBufferSize);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CMemoryFile* MemoryFile(void)
{
	CMemoryFile* pcMemoryFile;

	pcMemoryFile = NewMalloc<CMemoryFile>();
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

	pcMemoryFile = NewMalloc<CMemoryFile>();
	pcMemoryFile->Init(pvInitialMem, iInitialLength);
	pcMemoryFile->mbBasicFileMustFree = TRUE;
	return pcMemoryFile;
}

