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
#include "BaseLib/FileBasic.h"
#include "BaseLib/FileUtil.h"
#include "BaseLib/PointerRemapper.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/DiskFile.h"
#include "BaseLib/ConstructorCall.h"
#include "LogFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::Init(CAbstractFile* pcBackingFile)
{
	mpcBackingFile = pcBackingFile;

	miPosition = 0;
	miLength = 0;
	miFileLength = 0;
	mbTouched = FALSE;

	mcWrites.Init(LOG_FILE_COMMAND_CHUNK_SIZE);
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::Kill(void)
{
	mcWrites.Kill();
	mpcBackingFile->Kill();
	if (mpcBackingFile->mbBasicFileMustFree)
	{
		free(mpcBackingFile);
		mpcBackingFile = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Rewrite(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::PrivateWrite(CAbstractFile* pcFile)
{
	int							i;
	void*						pvData;
	SLogFileCommandWrite*		psWrite;
	filePos						iWritten;

	//Non-durable files cannot hit this code.

	for (i = 0; i < mcWrites.NumElements(); i++)
	{
		mcWrites.Get(i, &pvData);
		psWrite = (SLogFileCommandWrite*)pvData;
		pcFile->Seek(psWrite->iPosition, EFSO_SET);
		pvData = RemapSinglePointer(psWrite, sizeof(SLogFileCommandWrite));

		iWritten = pcFile->Write(pvData, psWrite->iSize, 1);
		if (iWritten != 1)
		{
			return FALSE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Open(EFileMode eFileMode)
{
	if (!mpcBackingFile->Open(eFileMode))
	{
		return FALSE;
	}

	miFileLength = mpcBackingFile->Size();
	miLength = miFileLength;
	miPosition = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Close(void)
{
	if (!mpcBackingFile->Close())
	{
		return FALSE;
	}
	miPosition = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Write(filePos iDistance, const void* pvSource, filePos iSize, filePos iCount)
{
	return Write(EFSO_SET, iDistance, pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Write(EFileSeekOrigin eOrigin, filePos iDistance, const void* pvSource, filePos iSize, filePos iCount)
{
	Seek(iDistance, eOrigin);
	return Write(pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Write(const void* pvSource, filePos iSize, filePos iCount)
{
	SLogFileCommandWrite*	psCommand;
	CArrayPointer				apvOverlapping;
	BOOL						bAny;
	void*						pvData;
	filePos						iByteLength;

	mbTouched = TRUE;

	iByteLength = iSize * iCount;
	bAny = FindTouchingWriteCommands(&apvOverlapping, miPosition, iByteLength, FALSE);
	if (bAny)
	{
		AmalgamateOverlappingWrites(&apvOverlapping, pvSource, miPosition, iByteLength);
		apvOverlapping.Kill();
		miPosition += iByteLength;
	}
	else
	{
		psCommand = (SLogFileCommandWrite*)mcWrites.Add(sizeof(SLogFileCommandWrite) + (int)iByteLength);
		if (!psCommand)
		{
			return 0;
		}
		pvData = RemapSinglePointer(psCommand, sizeof(SLogFileCommandWrite));

		psCommand->iSize = iByteLength;
		psCommand->iPosition = miPosition;
		memcpy_fast(pvData, (void*)pvSource, (int)iByteLength);

		miPosition += iByteLength;
	}
	UpdateLength();
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::AmalgamateOverlappingWrites(CArrayPointer* papvOverlapping, const void* pvSource, filePos iPosition, filePos iLength)
{
	filePos						iStart;
	filePos						iEnd;  //Inclusive;
	int							i;
	SLogFileCommandWrite*	psWrite;
	int							iIndex;
	filePos						iIndeedSize;
	SLogFileCommandWrite*	psCommand;
	void*						pvData;
	void*						pvDest;
	void*						pvNewSource;

	//Find the total size of the write chunk.
	iStart = iPosition;
	iEnd = iPosition + iLength - 1;
	for (i = 0; i < papvOverlapping->NumElements(); i++)
	{
		papvOverlapping->Get(i, (void**)&psWrite, &iIndex);

		if (psWrite->iPosition < iStart)
		{
			iStart = psWrite->iPosition;
		}

		if ((psWrite->iPosition + psWrite->iSize - 1) > iEnd)
		{
			iEnd = psWrite->iPosition + psWrite->iSize - 1;
		}
	}

	iIndeedSize = iEnd - iStart + 1;

	//Allocate enough memory for the new chunk.
	psCommand = (SLogFileCommandWrite*)mcWrites.Add(sizeof(SLogFileCommandWrite) + (int)iIndeedSize);
	if (!psCommand)
	{
		return FALSE;
	}

	pvData = RemapSinglePointer(psCommand, sizeof(SLogFileCommandWrite));

	psCommand->iSize = iIndeedSize;
	psCommand->iPosition = iStart;

	//Copy all the overlapping chunks into the new one.
	for (i = 0; i < papvOverlapping->NumElements(); i++)
	{
		iIndex = papvOverlapping->GetType(i);
		psWrite = (SLogFileCommandWrite*)mcWrites.Get(iIndex);
		pvDest = RemapSinglePointer(pvData, (int)(psWrite->iPosition - iStart));
		pvNewSource = RemapSinglePointer(psWrite, sizeof(SLogFileCommandWrite));
		memcpy_fast(pvDest, pvNewSource, (int)psWrite->iSize);
	}

	pvDest = RemapSinglePointer(pvData, (int)(iPosition - iStart));
	memcpy(pvDest, pvSource, (size_t)iLength);

	//Remove all the old overlapping chunks.
	for (i = papvOverlapping->NumElements()-1; i >= 0; i--)
	{
		iIndex = papvOverlapping->GetType(i);
		mcWrites.RemoveAt(iIndex, TRUE);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Read(filePos iDistance, void* pvDest, filePos iSize, filePos iCount)
{
	return Read(EFSO_SET, iDistance, pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Read(EFileSeekOrigin eOrigin, filePos iDistance, void* pvDest, filePos iSize, filePos iCount)
{
	Seek(iDistance, eOrigin);
	return Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Read(void* pvDest, filePos iSize, filePos iCount)
{
	CArrayPointer				apvOverlapping;
	BOOL						bAny;
	int							i;
	BOOL						bHoles;
	filePos						iResult;
	int							iIndex;
	SLogFileCommandWrite*		psWrite;
	void*						pvData;
	filePos						iLength;
	filePos						iDestOffset;
	filePos						iSourceOffset;
	void*						pvSource;
	void*						pvNewDest;
	filePos						iBytesReadFromFile;
	filePos						iByteSize;

	//You can test to see what sections actually NEED zero'ing.
	iByteSize = iSize * iCount;
	memset_fast(pvDest, 0, (size_t)iByteSize);

	bAny = FindTouchingWriteCommands(&apvOverlapping, miPosition, iByteSize, TRUE);
	if (bAny)
	{
		bHoles = FindHoles(&apvOverlapping, miPosition, iByteSize);
		if (bHoles)
		{
			iBytesReadFromFile = PrivateRead(pvDest, iSize, iCount);
		}

		for (i = 0; i < apvOverlapping.NumElements(); i++)
		{
			apvOverlapping.Get(i, (void**)&psWrite, &iIndex);

			pvData = RemapSinglePointer(psWrite, sizeof(SLogFileCommandWrite));

			iLength = psWrite->iSize;
			if ((psWrite->iPosition + psWrite->iSize) > (miPosition + iByteSize))
			{
				iLength -= (psWrite->iPosition + psWrite->iSize) - (miPosition + iByteSize);
			}

			iSourceOffset = 0;
			iDestOffset = psWrite->iPosition - miPosition;
			if (psWrite->iPosition < miPosition)
			{
				iSourceOffset = miPosition - psWrite->iPosition;
				iDestOffset = 0;
			}

			iLength -= iSourceOffset;

			pvSource = RemapSinglePointer(pvData, (int)iSourceOffset);
			pvNewDest = RemapSinglePointer(pvDest, (int)iDestOffset);

			memcpy_fast(pvNewDest, pvSource, (int)iLength);
		}
		apvOverlapping.Kill();

		if (miPosition + (iByteSize) > miLength)
		{
			iResult = (miLength - miPosition)/iSize;
			miPosition += iResult*iSize;
			return iResult;
		}
		else
		{
			miPosition += iByteSize;
			return iCount;
		}
	}
	else
	{
		iBytesReadFromFile = PrivateRead(pvDest, iSize, iCount);
		iResult = iBytesReadFromFile/iSize;
		miPosition += iResult*iSize;
		return iResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::ReadFromFile(void* pvDest, filePos iSize, filePos iCount)
{
	return mpcBackingFile->Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::PrivateRead(void* pvDest, filePos iSize, filePos iCount)
{
	filePos		iBytes;
	filePos		iRead;

	if (miPosition >= miFileLength)
	{
		return 0;
	}

	iBytes = iSize * iCount;
	if (iBytes > (miFileLength - miPosition))
	{
		iBytes = miFileLength - miPosition;
	}

	mpcBackingFile->Seek(miPosition, EFSO_SET);
	iRead = mpcBackingFile->Read(pvDest, iBytes, 1);
	if (iRead != 1)
	{
		return 0;
	}
	return iBytes;  //Not the count.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareLogFileWrite(const void* pv1, const void* pv2)
{
	SPointerAndSize*			psTypedPointer1;
	SPointerAndSize*			psTypedPointer2;
	SLogFileCommandWrite*	psWrite1;
	SLogFileCommandWrite*	psWrite2;

	psTypedPointer1 = (SPointerAndSize*)pv1;
	psTypedPointer2 = (SPointerAndSize*)pv2;

	psWrite1 = (SLogFileCommandWrite*)psTypedPointer1->pvData;
	psWrite2 = (SLogFileCommandWrite*)psTypedPointer2->pvData;

	if (psWrite1->iPosition < psWrite2->iPosition)
	{
		return -1;
	}
	if (psWrite1->iPosition > psWrite2->iPosition)
	{
		return -1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::FindHoles(CArrayPointer* papvOverlapping, filePos iPosition, filePos iLength)
{
	CArrayPointer				apvOverlappingSorted;
	int							i;
	SLogFileCommandWrite*	psWrite;
	int							eCommand;
	BOOL						bHoles;
	filePos						iEnd;

	apvOverlappingSorted.Init();
	apvOverlappingSorted.Copy(papvOverlapping);
	apvOverlappingSorted.QuickSort(CompareLogFileWrite);

	bHoles = FALSE;
	iEnd = -1;
	for (i = 0; i < apvOverlappingSorted.NumElements(); i++)
	{
		apvOverlappingSorted.Get(i, (void**)&psWrite, &eCommand);
		if ((i == 0) && (psWrite->iPosition > iPosition))
		{
			bHoles = TRUE;
			break;
		}
		else
		{
			if (psWrite->iPosition >= (iEnd+1))
			{
				bHoles = TRUE;
				break;
			}
		}
		iEnd = psWrite->iPosition + psWrite->iSize -1;
		if (iEnd >= (iPosition + iLength-1))
		{
			break;
		}
	}

	if (iEnd < (iPosition + iLength-1))
	{
		bHoles = TRUE;
	}

	apvOverlappingSorted.Kill();
	return bHoles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::FindTouchingWriteCommands(CArrayPointer* papvOverlapping, filePos iPosition, filePos iLength, BOOL bMustOverlap)
{
	int							i;
	SLogFileCommandWrite*	psWrite;
	BOOL						bInitialised;

	bInitialised = FALSE;
	if (!bMustOverlap)
	{
		iPosition--;
		iLength+=2;
	}

	for (i = 0; i < mcWrites.NumElements(); i++)
	{
		mcWrites.Get(i, (void**)&psWrite);
		if (Overlaps(iPosition, iLength, psWrite))
		{
			if (!bInitialised)
			{
				papvOverlapping->Init(8);
				bInitialised = TRUE;
			}

			papvOverlapping->Add(psWrite, i /* <-- Cheating like Hell */);
		}
	}

	return bInitialised;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Overlaps(filePos iPosition, filePos iLength, SLogFileCommandWrite* psWrite)
{
	filePos		iNewStart;
	filePos		iNewEnd;  //Inclusive
	filePos		iNextStart;
	filePos		iNextEnd; //Inclusive

	iNewStart = iPosition;
	iNewEnd = iPosition + iLength - 1;

	iNextStart = psWrite->iPosition;
	iNextEnd = iNextStart + psWrite->iSize - 1;

	if ((iNewStart <= iNextStart) && (iNewEnd >= iNextStart))
	{
		return TRUE;
	}
	if ((iNewStart <= iNextEnd) && (iNewEnd >= iNextEnd))
	{
		return TRUE;
	}
	if ((iNewStart >= iNextStart) && (iNewStart <= iNextEnd))
	{
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::UpdateLength(void)
{
	if (miPosition > miLength)
	{
		miLength = miPosition;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Tell(void)
{
	return miPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Size(void)
{
	return miLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::SizeFromFile(void)
{
	if (mpcBackingFile->IsOpen())
	{
		return mpcBackingFile->Size();
	}
	else
	{
		return 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Seek(filePos iOffset, EFileSeekOrigin eOrigin)
{
	if (eOrigin == EFSO_SET)
	{
		miPosition = iOffset;
	}
	else if (eOrigin == EFSO_CURRENT)
	{
		miPosition += iOffset;
	}
	else if (eOrigin == EFSO_END)
	{
		miPosition = miLength + iOffset;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Eof(void)
{
	return miPosition >= miLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::IsOpen(void)
{
	return mpcBackingFile->IsOpen();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Flush(void)
{
	return mpcBackingFile->Flush();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::Dump(void)
{
	CChars						sz;
	int							i;
	char*						pvData;
	SLogFileCommandWrite*	psWrite;
	filePos						iLen;

	sz.Init();
	sz.Append("Log File (?");
	sz.Append(")\n------------------\n");
	sz.Append("Commands: ");
	sz.Append(mcWrites.NumElements());
	sz.AppendNewLine();

	for (i = 0; i < mcWrites.NumElements(); i++)
	{
		mcWrites.Get(i, (void**)&pvData);
		sz.Append("   ");
		sz.Append("Write: (");
		psWrite = (SLogFileCommandWrite*)pvData;
		sz.Append(psWrite->iPosition);
		sz.Append(", ");
		sz.Append(psWrite->iSize);
		sz.Append(") ");

		pvData = (char*)RemapSinglePointer(psWrite, sizeof(SLogFileCommandWrite));
		iLen = psWrite->iSize;
		sz.AppendData(pvData, 80);
		sz.AppendNewLine();
	}
	sz.AppendNewLine();
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogFile* LogFile(CAbstractFile* pcFile)
{
	CLogFile* pcLogFile;

	pcLogFile = Malloc(CLogFile);
	pcLogFile->Init(pcFile);
	pcLogFile->mbBasicFileMustFree = TRUE;
	return pcLogFile;

}

