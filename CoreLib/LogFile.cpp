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
	CAbstractFile::Init();

	mpcBackingFile = pcBackingFile;

	miPosition = 0;
	miLength = 0;
	miFileLength = 0;

	mbTouched = FALSE;
	meFileMode = EFM_Unknown;
	mbOpenedBackingFile = FALSE;

	macCommands.Init(LOG_FILE_COMMAND_CHUNK_SIZE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::Kill(void)
{
	macCommands.Kill();

	if (mpcBackingFile)
	{
		mpcBackingFile->Kill();
		if (mpcBackingFile->mbBasicFileMustFree)
		{
			free(mpcBackingFile);
			mpcBackingFile = NULL;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Begin(void)
{
	BOOL	bMustOpen;

	macCommands.ReInit();

	miPosition = 0;

	if (mpcBackingFile)
	{
		bMustOpen = !mpcBackingFile->IsOpen();
		if (bMustOpen)
		{
			ReturnOnFalse(mpcBackingFile->Open(EFM_Read));
			mbOpenedBackingFile = TRUE;
		}
		miFileLength = mpcBackingFile->Size();
	}
	else
	{
		mbOpenedBackingFile = FALSE;
		miFileLength = 0;
	}
	miLength = miFileLength;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Commit(void)
{
	if (mbOpenedBackingFile)
	{
		mpcBackingFile->Close();
		mbOpenedBackingFile = FALSE;
	}

	return Commit(mpcBackingFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::End(void)
{
	miPosition = 0;
	miLength = 0;
	miFileLength = 0;

	mbTouched = FALSE;
	meFileMode = EFM_Unknown;

	macCommands.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Commit(CAbstractFile* pcFile)
{
	int							i;
	void*						pvData;
	CLogFileCommand*			psCommand;
	CLogFileCommandWrite*		psWrite;
	CLogFileCommandOpen*		psOpen;
	CLogFileCommandClose*		psClose;
	CLogFileCommandDelete*		psDelete;

	for (i = 0; i < macCommands.NumElements(); i++)
	{
		macCommands.Get(i, &pvData);
		psCommand = (CLogFileCommand*)pvData;
		if (psCommand->IsWrite())
		{
			psWrite = (CLogFileCommandWrite*)psCommand;
			ReturnOnFalse(psWrite->Write(pcFile));
		}
		else if (psCommand->IsOpen())
		{
			psOpen = (CLogFileCommandOpen*)psCommand;
			ReturnOnFalse(psOpen->Open(pcFile));
		}
		else if (psCommand->IsClose())
		{
			psClose = (CLogFileCommandClose*)psCommand;
			ReturnOnFalse(psClose->Close(pcFile));
		}
		else if (psCommand->IsDelete())
		{
			psDelete = (CLogFileCommandDelete*)psCommand;
			ReturnOnFalse(psDelete->Delete(pcFile));
		}
	}

	pcFile->Flush();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Open(EFileMode eFileMode)
{
	if (!AddOpenCommand(eFileMode))
	{
		meFileMode = EFM_Unknown;
		return FALSE;
	}

	meFileMode = eFileMode;
	
	miPosition = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogFileCommandOpen* CLogFile::AddOpenCommand(EFileMode eFileMode)
{
	CLogFileCommandOpen*	psCommand;

	psCommand = (CLogFileCommandOpen*)macCommands.Add(sizeof(CLogFileCommandOpen));
	if (!psCommand)
	{
		return NULL;
	}

	psCommand->Init(eFileMode);
	return psCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Close(void)
{
	if (!AddCloseCommand())
	{
		return FALSE;
	}

	meFileMode = EFM_Unknown;
	miPosition = 0;
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogFileCommandClose* CLogFile::AddCloseCommand(void)
{
	CLogFileCommandClose*	psCommand;

	psCommand = (CLogFileCommandClose*)macCommands.Add(sizeof(CLogFileCommandClose));
	if (!psCommand)
	{
		return NULL;
	}

	psCommand->Init();
	return psCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogFileCommandDelete* CLogFile::AddDeleteCommand(void)
{
	CLogFileCommandDelete*	psCommand;

	psCommand = (CLogFileCommandDelete*)macCommands.Add(sizeof(CLogFileCommandDelete));
	if (!psCommand)
	{
		return NULL;
	}

	psCommand->Init();
	return psCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Write(const void* pvSource, filePos iSize, filePos iCount)
{
	CArrayIntAndPointer			apvOverlapping;
	BOOL					bAny;
	filePos					iByteLength;
	CLogFileCommandWrite*	pcWrite;

	if (IsFileModeWritable(meFileMode))
	{
		mbTouched = TRUE;

		iByteLength = iSize * iCount;

		bAny = FindTouchingWriteCommands(0, &apvOverlapping, miPosition, iByteLength, FALSE);
		if (bAny)
		{
			AmalgamateOverlappingWrites(&apvOverlapping, pvSource, miPosition, iByteLength);
			apvOverlapping.Kill();
			miPosition += iByteLength;
		}
		else
		{
			pcWrite = AddWriteCommand(miPosition, (void*)pvSource, iByteLength);
			if (!pcWrite)
			{
				return 0;
			}

			miPosition += iByteLength;
		}
		UpdateLength();
		return iCount;
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
CLogFileCommandWrite* CLogFile::AddWriteCommand(filePos iPosition, void* pvSource, filePos iByteLength)
{
	CLogFileCommandWrite*	psCommand;

	psCommand = (CLogFileCommandWrite*)macCommands.Add(sizeof(CLogFileCommandWrite) + (int)iByteLength);
	if (!psCommand)
	{
		return NULL;
	}

	psCommand->Init(iPosition, pvSource, iByteLength);
	return psCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogFileCommandWrite* CLogFile::AddWriteCommand(filePos iPosition, filePos iByteLength)
{
	CLogFileCommandWrite*	psCommand;

	psCommand = (CLogFileCommandWrite*)macCommands.Add(sizeof(CLogFileCommandWrite) + (int)iByteLength);
	if (!psCommand)
	{
		return NULL;
	}

	psCommand->Init(iPosition, iByteLength);
	return psCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::AmalgamateOverlappingWrites(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, filePos iLength)
{
	filePos					iStart;
	filePos					iEnd;  //Inclusive;
	int						i;
	CLogFileCommandWrite*	psWrite;
	int						iIndex;
	filePos					iIndeedSize;
	CLogFileCommandWrite*	psCommand;
	void*					pvData;
	void*					pvDest;
	void*					pvNewSource;

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
	psCommand = AddWriteCommand(iStart, iIndeedSize);
	if (!psCommand)
	{
		return FALSE;
	}

	pvData = RemapSinglePointer(psCommand, sizeof(CLogFileCommandWrite));

	//Copy all the overlapping chunks into the new one.
	for (i = 0; i < papvOverlapping->NumElements(); i++)
	{
		iIndex = papvOverlapping->GetType(i);
		psWrite = (CLogFileCommandWrite*)macCommands.Get(iIndex);
		pvDest = RemapSinglePointer(pvData, (int)(psWrite->iPosition - iStart));
		pvNewSource = RemapSinglePointer(psWrite, sizeof(CLogFileCommandWrite));
		memcpy_fast(pvDest, pvNewSource, (int)psWrite->iSize);
	}

	pvDest = RemapSinglePointer(pvData, (int)(iPosition - iStart));
	memcpy_fast(pvDest, (void*)pvSource, (size_t)iLength);

	//Remove all the old overlapping chunks.
	for (i = papvOverlapping->NumElements()-1; i >= 0; i--)
	{
		iIndex = papvOverlapping->GetType(i);
		macCommands.RemoveAt(iIndex, TRUE);
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::Read(void* pvDest, filePos iSize, filePos iCount)
{
	filePos		iResult;
	int			iWriteIndex;
	filePos		iByteSize;

	iByteSize = iSize * iCount;

	if (IsFileModeReadable(meFileMode))
	{
		iWriteIndex = FindNextWriteCommand(0);
		if (iWriteIndex == -1)
		{
			iResult = ReadWithNoTouchingWrites(pvDest, iSize, iCount);
			if (iResult != iByteSize)
			{
				return 0;
			}
			else
			{
				miPosition += iByteSize;
				return iCount;
			}

		}

		iResult = ReadFirstTouchingWrites(iWriteIndex, pvDest, iSize, iCount);
		if (iResult != iCount)
		{
			return iResult;
		}

		iWriteIndex = FindNextWriteCommand(iWriteIndex+1);
		while (iWriteIndex != -1)
		{
			iResult = ReadNextTouchingWrites(iWriteIndex, pvDest, iSize, iCount);
			if (iResult != iCount)
			{
				return 0;
			}
			iWriteIndex = FindNextWriteCommand(iWriteIndex+1);
		}

		if ((miPosition + iByteSize) > miLength)
		{
			iResult = (miLength - miPosition) / iSize;
			miPosition += iResult * iSize;
		}
		else
		{
			iResult = iCount;
			miPosition += iByteSize;
		}

		return iResult;
	}
	else
	{
		return 0;
	}
}

//{
//	CArrayIntAndPointer			apvOverlapping;
//	BOOL						bAnyTouchingWriteCommands;
//	int							i;
//	BOOL						bHoles;
//	filePos						iResult;
//	int							iIndex;
//	SDurableFileCommandWrite*	psWrite;
//	void*						pvData;
//	filePos						iLength;
//	filePos						iDestOffset;
//	filePos						iSourceOffset;
//	void*						pvSource;
//	void*						pvNewDest;
//	filePos						iBytesReadFromFile;
//	filePos						iByteSize;
//
//	//You can test to see what sections actually NEED zeroing.
//	iByteSize = iSize * iCount;
//	memset_fast(pvDest, 0, (size_t)iByteSize);
//
//	bAnyTouchingWriteCommands = FindTouchingWriteCommands(&apvOverlapping, miPosition, iByteSize, TRUE);
//	if (bAnyTouchingWriteCommands)
//	{
//		bHoles = DoOverlappingWritesContainHoles(&apvOverlapping, miPosition, iByteSize);
//		if (bHoles)
//		{
//			iBytesReadFromFile = ReadPrimaryFileForDurableRead(pvDest, iSize, iCount);
//		}
//
//		for (i = 0; i < apvOverlapping.NumElements(); i++)
//		{
//			apvOverlapping.Get(i, (void**)&psWrite, &iIndex);
//
//			pvData = RemapSinglePointer(psWrite, sizeof(SDurableFileCommandWrite));
//
//			iLength = psWrite->iSize;
//			if ((psWrite->iPosition + psWrite->iSize) >(miPosition + iByteSize))
//			{
//				iLength -= (psWrite->iPosition + psWrite->iSize) - (miPosition + iByteSize);
//			}
//
//			iSourceOffset = 0;
//			iDestOffset = psWrite->iPosition - miPosition;
//			if (psWrite->iPosition < miPosition)
//			{
//				iSourceOffset = miPosition - psWrite->iPosition;
//				iDestOffset = 0;
//			}
//
//			iLength -= iSourceOffset;
//
//			pvSource = RemapSinglePointer(pvData, (int)iSourceOffset);
//			pvNewDest = RemapSinglePointer(pvDest, (int)iDestOffset);
//
//			memcpy_fast(pvNewDest, pvSource, (int)iLength);
//		}
//		apvOverlapping.Kill();
//
//		if (miPosition + (iByteSize) > miLength)
//		{
//			iResult = (miLength - miPosition) / iSize;
//			miPosition += iResult*iSize;
//			return iResult;
//		}
//		else
//		{
//			miPosition += iByteSize;
//			return iCount;
//		}
//	}
//	else
//	{
//		iBytesReadFromFile = ReadPrimaryFileForDurableRead(pvDest, iSize, iCount);
//		iResult = iBytesReadFromFile / iSize;
//		miPosition += iBytesReadFromFile;
//		return iResult;
//	}
//}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::ReadWithNoTouchingWrites(void* pvDest, filePos iSize, filePos iCount)
{
	filePos		iBytesReadFromFile;

	iBytesReadFromFile = ReadFromBackingFile(pvDest, iSize, iCount);
	return iBytesReadFromFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::ReadFirstTouchingWrites(int iWriteIndex, void* pvDest, filePos iSize, filePos iCount)
{
	CArrayIntAndPointer	apvOverlapping;
	BOOL			bAny;
	BOOL			bHoles;
	filePos			iBytesReadFromFile;
	filePos			iByteSize;

	iByteSize = iSize * iCount;
	bAny = FindTouchingWriteCommands(iWriteIndex, &apvOverlapping, miPosition, iByteSize, TRUE);
	if (bAny)
	{
		bHoles = FindHoles(&apvOverlapping, miPosition, iByteSize);
		if (bHoles)
		{
			iBytesReadFromFile = ReadFromBackingFile(pvDest, iSize, iCount);
		}

		CopyWritesToRead(&apvOverlapping, iByteSize, pvDest);
		apvOverlapping.Kill();
		return iCount;
	}
	else
	{
		if (miFileLength == 0)
		{
			return iCount;
		}
		else
		{
			iBytesReadFromFile = ReadWithNoTouchingWrites(pvDest, iSize, iCount);
			if (iBytesReadFromFile == iByteSize)
			{
				return iCount;
			}
			else
			{
				return iBytesReadFromFile / iSize;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::ReadNextTouchingWrites(int iWriteIndex, void* pvDest, filePos iSize, filePos iCount)
{
	CArrayIntAndPointer	apvOverlapping;
	BOOL			bAny;
	filePos			iByteSize;

	iByteSize = iSize * iCount;
	bAny = FindTouchingWriteCommands(iWriteIndex, &apvOverlapping, miPosition, iByteSize, TRUE);
	if (bAny)
	{
		CopyWritesToRead(&apvOverlapping, iByteSize, pvDest);
		apvOverlapping.Kill();
	}
	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::CopyWritesToRead(CArrayIntAndPointer* papvOverlapping, filePos iByteSize, void* pvDest)
{
	int						i;
	int						iNumWrites;
	CLogFileCommandWrite*	psWrite;
	int						iIndex;
	void*					pvData;
	filePos					iDestOffset;
	filePos					iSourceOffset;
	void*					pvSource;
	void*					pvNewDest;
	filePos					iLength;

	iNumWrites = papvOverlapping->NumElements();
	for (i = 0; i < iNumWrites; i++)
	{
		papvOverlapping->Get(i, (void**)&psWrite, &iIndex);

		pvData = RemapSinglePointer(psWrite, sizeof(CLogFileCommandWrite));

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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CLogFile::FindNextWriteCommand(int iIndex)
{
	int						i;
	CLogFileCommand*		psCommand;
	int						iNumWrites;
	CLogFileCommandOpen*	psOpen;
	BOOL					bFoundWriteOpen;

	iNumWrites = 0;
	bFoundWriteOpen = FALSE;
	for (i = iIndex; i < macCommands.NumElements(); i++)
	{
		psCommand = (CLogFileCommand*)macCommands.Get(i);
		if (psCommand->IsOpen())
		{
			psOpen = (CLogFileCommandOpen*)psCommand;
			if (IsFileModeWritable(psOpen->eMode))
			{
				bFoundWriteOpen = TRUE;
			}
		}
		else if (psCommand->IsWrite() && bFoundWriteOpen)
		{
			return i;
		}
		else if (bFoundWriteOpen)
		{
			bFoundWriteOpen = FALSE;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::ReadFromBackingFile(void* pvDest, filePos iSize, filePos iCount)
{
	filePos		iByteSize;
	filePos		iRead;

	if (miPosition >= miFileLength)
	{
		return 0;
	}

	iByteSize = iSize * iCount;
	if (iByteSize > (miFileLength - miPosition))
	{
		iByteSize = miFileLength - miPosition;
	}

	mpcBackingFile->Seek(miPosition, EFSO_SET);
	iRead = mpcBackingFile->Read(pvDest, iByteSize, 1);
	if (iRead != 1)
	{
		return 0;
	}
	return iByteSize;  //Not the count.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareLogFileWrite(const void* pv1, const void* pv2)
{
	SPointerAndSize*			psTypedPointer1;
	SPointerAndSize*			psTypedPointer2;
	CLogFileCommandWrite*	psWrite1;
	CLogFileCommandWrite*	psWrite2;

	psTypedPointer1 = (SPointerAndSize*)pv1;
	psTypedPointer2 = (SPointerAndSize*)pv2;

	psWrite1 = (CLogFileCommandWrite*)psTypedPointer1->pvData;
	psWrite2 = (CLogFileCommandWrite*)psTypedPointer2->pvData;

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
BOOL CLogFile::TestFindHoles(int iWriteIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength)
{
	BOOL	bAny;

	bAny = FindTouchingWriteCommands(iWriteIndex, papvOverlapping, iPosition, iLength, TRUE);
	if (bAny)
	{
		return FindHoles(papvOverlapping, iPosition, iLength);
	}
	else
	{
		if (iLength == 0)
		{
			return FALSE;
		}
		else
		{
			return TRUE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::FindHoles(CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength)
{
	CArrayIntAndPointer				apvOverlappingSorted;
	int							i;
	CLogFileCommandWrite*	psWrite;
	int							eCommand;
	BOOL						bHoles;
	filePos						iEnd;

	if (iLength == 0)
	{
		return FALSE;
	}
	if (papvOverlapping->NumElements() == 0)
	{
		return FALSE;
	}

	apvOverlappingSorted.Init(1);
	apvOverlappingSorted.Copy(papvOverlapping);
	apvOverlappingSorted.QuickSort(CompareLogFileWrite);

	apvOverlappingSorted.Get(0, (void**)&psWrite, &eCommand);
	if (psWrite->iPosition > iPosition)
	{
		apvOverlappingSorted.Kill();
		return TRUE;
	}
	else
	{
		iEnd = psWrite->iPosition + psWrite->iSize -1;
	}

	bHoles = FALSE;
	for (i = 1; i < apvOverlappingSorted.NumElements(); i++)
	{
		apvOverlappingSorted.Get(i, (void**)&psWrite, &eCommand);
		if (psWrite->iPosition > (iEnd+1))
		{
			bHoles = TRUE;
			break;
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
BOOL CLogFile::FindTouchingWriteCommands(int iStartIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength, BOOL bMustOverlap)
{
	int						iIndex;
	CLogFileCommandWrite*	psWrite;
	CLogFileCommand*		psCommand;
	BOOL					bInitialised;

	bInitialised = FALSE;
	if (!bMustOverlap)
	{
		iPosition--;
		iLength += 2;
	}

	for (iIndex = iStartIndex; iIndex < macCommands.NumElements(); iIndex++)
	{
		macCommands.Get(iIndex, (void**)&psCommand);
		if (psCommand->IsWrite())
		{
			psWrite = (CLogFileCommandWrite*)psCommand;
			if (Overlaps(iPosition, iLength, psWrite))
			{
				if (!bInitialised)
				{
					papvOverlapping->Init(8);
					bInitialised = TRUE;
				}

				papvOverlapping->Add(psWrite, iIndex);
			}
		}
	}

	return bInitialised;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Overlaps(filePos iPosition, filePos iLength, CLogFileCommandWrite* psWrite)
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
	return meFileMode != EFM_Unknown;
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
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CLogFile::Delete(void)
{
	if (IsOpen())
	{
		return FALSE;
	}

	macCommands.ReInit();
	miFileLength = 0;
	miLength = 0;
	return AddDeleteCommand() != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CLogFile::GetNumWrites(void)
{
	int					i;
	CLogFileCommand*	psCommand;
	int					iNumWrites;

	iNumWrites = 0;
	for (i = 0; i < macCommands.NumElements(); i++)
	{
		psCommand = (CLogFileCommand*)macCommands.Get(i);
		if (psCommand->IsWrite())
		{
			iNumWrites++;
		}
	}
	return iNumWrites;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogFileCommandWrite* CLogFile::GetWriteData(int iWrite)
{
	int					i;
	CLogFileCommand*	psCommand;
	int					iNumWrites;

	iNumWrites = 0;
	for (i = 0; i < macCommands.NumElements(); i++)
	{
		psCommand = (CLogFileCommand*)macCommands.Get(i);
		if (psCommand->IsWrite())
		{
			if (iNumWrites == iWrite)
			{
				return (CLogFileCommandWrite*)psCommand;
			}
			iNumWrites++;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CLogFile::GetWriteSize(int iIndex)
{
	int						i;
	CLogFileCommand*		psCommand;
	int						iNumWrites;
	CLogFileCommandWrite*	psWrite;

	iNumWrites = 0;
	for (i = 0; i < macCommands.NumElements(); i++)
	{
		psCommand = (CLogFileCommand*)macCommands.Get(i);
		if (psCommand->IsWrite())
		{
			if (iNumWrites == iIndex)
			{
				psWrite = (CLogFileCommandWrite*)psCommand;
				return psWrite->iSize;
			}
			iNumWrites++;
		}
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CLogFile::GetNumCommands(void)
{
	return macCommands.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::Dump(void)
{
	CChars					sz;
	int						i;
	char*					pvData;
	CLogFileCommandWrite*	psWrite;
	filePos					iLen;

	sz.Init();
	sz.Append("Log File (?");
	sz.Append(")\n------------------\n");
	sz.Append("Commands: ");
	sz.Append(macCommands.NumElements());
	sz.AppendNewLine();

	for (i = 0; i < macCommands.NumElements(); i++)
	{
		macCommands.Get(i, (void**)&pvData);
		sz.Append("   ");
		sz.Append("Write: (");
		psWrite = (CLogFileCommandWrite*)pvData;
		sz.Append(psWrite->iPosition);
		sz.Append(", ");
		sz.Append(psWrite->iSize);
		sz.Append(") ");

		pvData = (char*)RemapSinglePointer(psWrite, sizeof(CLogFileCommandWrite));
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

	pcLogFile = NewMalloc<CLogFile>();
	pcLogFile->Init(pcFile);
	pcLogFile->mbBasicFileMustFree = TRUE;
	return pcLogFile;

}

