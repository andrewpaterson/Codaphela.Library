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
#include "Logger.h"
#include "FileBasic.h"
#include "FileUtil.h"
#include "PointerRemapper.h"
#include "FastFunctions.h"
#include "DiskFile.h"
#include "ConstructorCall.h"
#include "LogFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::Init(CAbstractFile* pcBackingFile)
{
	CAbstractFile::Init();

	if (pcBackingFile == NULL)
	{
		gcLogger.Error2(__METHOD__, " Log file must have a backing file.", NULL);
		return;
	}

	mpcBackingFile = pcBackingFile;

	miPosition = 0;
	miLength = 0;
	miBackingFileLength = 0;
	mbBackingFileExists = false;

	mbTouched = false;
	meFileMode = EFM_Unknown;
	mbOpenedBackingFile = false;
	mbBegun = false;

	macCommands.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::Kill(void)
{
	macCommands.Kill();

	if (mpcBackingFile->mbBasicFileMustFree)
	{
		SafeKill(mpcBackingFile);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::Begin(void)
{
	bool	bMustOpen;

	if (macCommands.NumElements() != 0)
	{
		return gcLogger.Error2(__METHOD__, " Cannot begin when log file already has outstanding commands.", NULL);
	}

	bMustOpen = !mpcBackingFile->IsOpen();
	if (bMustOpen)
	{
		mbBackingFileExists = mpcBackingFile->Open(EFM_Read);
		if (mbBackingFileExists)
		{
			mbOpenedBackingFile = true;
			miBackingFileLength = mpcBackingFile->Size();
			miLength = miBackingFileLength;
			mpcBackingFile->Seek(0, EFSO_SET);
			miPosition = 0;
			mbBegun = true;
			return true;
		}
		else
		{
			miBackingFileLength = 0;
			miLength = 0;
			miPosition = 0;
			mbBegun = true;
			return true;
		}
	}
	else
	{
		miBackingFileLength = mpcBackingFile->Size();
		miLength = miBackingFileLength;
		mpcBackingFile->Seek(0, EFSO_SET);
		miPosition = 0;
		mbBegun = true;
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::Commit(void)
{
	if (mbOpenedBackingFile)
	{
		mpcBackingFile->Close();
		mbOpenedBackingFile = false;
	}

	return Commit(mpcBackingFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::End(void)
{
	ReturnOnFalse(ValidateBegun(__METHOD__, "end", mpcBackingFile));

	miPosition = 0;
	miLength = 0;
	miBackingFileLength = 0;
	mbBegun = false;

	mbTouched = false;
	meFileMode = EFM_Unknown;

	macCommands.ReInit();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::ExecuteCommandErrorString(CChars* pszDest, char* szMethod, CLogFileCommand* psCommand, CAbstractFile* pcFile)
{
	pszDest->Init(szMethod);
	pszDest->Append(" Could not execute command [");
	pszDest->Append(psCommand->GetType());
	pszDest->Append("] on file [");
	GetSafeFilename(pszDest, pcFile);
	pszDest->Append("].");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::AddCommandErrorString(CChars* pszDest, char* szMethod, CLogFileCommand* psCommand, CAbstractFile* pcFile)
{
	pszDest->Init(szMethod);
	pszDest->Append(" Could not Add command [");
	pszDest->Append(psCommand->GetType());
	pszDest->Append("] if not begun on file [");
	GetSafeFilename(pszDest, pcFile);
	pszDest->Append("].");
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::GetSafeFilename(CChars* pszDest, CAbstractFile* pcFile)
{
	char*	pszFilename;

	pszFilename = pcFile->GetFilename();
	if (pszFilename)
	{
		pszDest->Append(pszFilename);
	}
	else
	{
		pszDest->Append("Without Name");
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::ValidateBegun(char* szMethod, char* szTask, CAbstractFile* pcFile)
{
	CChars	szError;

	if (!mbBegun)
	{
		szError.Init(szMethod);
		szError.Append(" Cannot ");
		szError.Append(szTask);
		szError.Append(" if not begun.");

		szError.Append("] on file [");
		GetSafeFilename(&szError, pcFile);
		szError.Append("].");

		gcLogger.Error(szError.Text());
		szError.Kill();
		return false;
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
bool CLogFile::Commit(CAbstractFile* pcFile)
{
	size						i;
	void*						pvData;
	CLogFileCommand*			psCommand;
	CLogFileCommandWrite*		psWrite;
	CLogFileCommandOpen*		psOpen;
	CLogFileCommandClose*		psClose;
	CLogFileCommandDelete*		psDelete;
	bool						bResult;
	CChars						szError;
	size						iNumCommands;

	ReturnOnFalse(ValidateBegun(__METHOD__, "commit", pcFile));

	iNumCommands = macCommands.NumElements();
	for (i = 0; i < iNumCommands; i++)
	{
		macCommands.Get(i, &pvData);
		psCommand = (CLogFileCommand*)pvData;

		if (psCommand->IsWrite())
		{
			psWrite = (CLogFileCommandWrite*)psCommand;
			bResult = psWrite->Write(pcFile);
		}
		else if (psCommand->IsOpen())
		{
			psOpen = (CLogFileCommandOpen*)psCommand;
			bResult = psOpen->Open(pcFile);
		}
		else if (psCommand->IsClose())
		{
			psClose = (CLogFileCommandClose*)psCommand;
			bResult = psClose->Close(pcFile);
		}
		else if (psCommand->IsDelete())
		{
			psDelete = (CLogFileCommandDelete*)psCommand;
			bResult = psDelete->Delete(pcFile);
		}

		if (!bResult)
		{
			ExecuteCommandErrorString(&szError, __METHOD__, psCommand, pcFile);
			gcLogger.Error(szError.Text()); 
			szError.Kill();
			return false;
		}
	}

	//Shouldn't we check the file is open before flushing?
	pcFile->Flush();

	miBackingFileLength = miLength;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::Open(EFileMode eFileMode)
{
	if ((mbBackingFileExists) || IsFileModeCreate(eFileMode))
	{
		mbBackingFileExists = true;
		if (!AddOpenCommand(eFileMode))
		{
			meFileMode = EFM_Unknown;
			return false;
		}

		meFileMode = eFileMode;
	
		miPosition = 0;
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLogFileCommandOpen* CLogFile::AddOpenCommand(EFileMode eFileMode)
{
	CLogFileCommandOpen*	psCommand;
	CChars					szError;
	CLogFileCommandOpen		sCommand;

	sCommand.Init(eFileMode);
	if (!mbBegun)
	{
		AddCommandErrorString(&szError, __METHOD__, &sCommand, mpcBackingFile);
		gcLogger.Error(szError.Text());
		szError.Kill();
		return false;
	}


	psCommand = (CLogFileCommandOpen*)macCommands.Add(&sCommand, sizeof(CLogFileCommandOpen));
	if (!psCommand)
	{
		return NULL;
	}

	return psCommand;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::Close(void)
{
	if (!AddCloseCommand())
	{
		return false;
	}

	meFileMode = EFM_Unknown;
	miPosition = 0;
	return true;
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
size CLogFile::Write(const void* pvSource, size iSize, size iCount)
{
	CArrayIntAndPointer		apvOverlapping;
	bool					bAny;
	size					iByteLength;
	CLogFileCommandWrite*	pcWrite;

	if (IsFileModeWritable(meFileMode))
	{
		mbTouched = true;

		iByteLength = iSize * iCount;

		bAny = FindTouchingWriteCommands(0, &apvOverlapping, miPosition, iByteLength, false);
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
CLogFileCommandWrite* CLogFile::AddWriteCommand(filePos iPosition, void* pvSource, size iByteLength)
{
	CLogFileCommandWrite*	psCommand;

	psCommand = (CLogFileCommandWrite*)macCommands.Add(sizeof(CLogFileCommandWrite) + iByteLength);
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
CLogFileCommandWrite* CLogFile::AddWriteCommand(filePos iPosition, size iByteLength)
{
	CLogFileCommandWrite*	psCommand;

	psCommand = (CLogFileCommandWrite*)macCommands.Add(sizeof(CLogFileCommandWrite) + iByteLength);
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
bool CLogFile::AmalgamateOverlappingWrites(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, size iLength)
{
	filePos					iStart;
	filePos					iEnd;  //Inclusive;
	size					i;
	CLogFileCommandWrite*	psWrite;
	size					iIndex;
	size					iIndeedSize;
	CLogFileCommandWrite*	psCommand;
	void*					pvData;
	void*					pvDest;
	void*					pvNewSource;
	size					uiNumElements;

	//Find the total size of the write chunk.
	iStart = iPosition;
	iEnd = iPosition + iLength - 1;
	uiNumElements = papvOverlapping->NumElements();
	for (i = 0; i < uiNumElements; i++)
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

	iIndeedSize = (size)(iEnd - iStart + 1);

	//Allocate enough memory for the new chunk.
	psCommand = AddWriteCommand(iStart, iIndeedSize);
	if (!psCommand)
	{
		return false;
	}

	pvData = RemapSinglePointer(psCommand, sizeof(CLogFileCommandWrite));

	//Copy all the overlapping chunks into the new one.
	uiNumElements = papvOverlapping->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		iIndex = papvOverlapping->GetType(i);
		psWrite = (CLogFileCommandWrite*)macCommands.Get(iIndex);
		pvDest = RemapSinglePointer(pvData, (ptrdiff)(psWrite->iPosition - iStart));
		pvNewSource = RemapSinglePointer(psWrite, sizeof(CLogFileCommandWrite));
		memcpy_fast(pvDest, pvNewSource, psWrite->iSize);
	}

	pvDest = RemapSinglePointer(pvData, (ptrdiff)(iPosition - iStart));
	memcpy_fast(pvDest, (void*)pvSource, iLength);

	//Remove all the old overlapping chunks.
	i = papvOverlapping->NumElements();
	if (i != 0)
	{
		do
		{
			i--;
			iIndex = papvOverlapping->GetType(i);
			macCommands.RemoveAt(iIndex, true);
		}
		while (i != 0);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLogFile::Read(void* pvDest, size iSize, size iCount)
{
	size	uiBytesRead;
	size	uiWriteIndex;
	size	uiByteSize;

	uiByteSize = iSize * iCount;

	if (IsFileModeReadable(meFileMode))
	{
		uiWriteIndex = FindNextWriteCommand(0);
		if (uiWriteIndex == ARRAY_ELEMENT_NOT_FOUND)
		{
			uiBytesRead = ReadWithNoTouchingWrites(pvDest, iSize, iCount);
			if (uiBytesRead != uiByteSize)
			{
				return 0;
			}
			else
			{
				miPosition += uiByteSize;
				return iCount;
			}

		}

		uiBytesRead = ReadFirstTouchingWrites(uiWriteIndex, pvDest, iSize, iCount);
		if (uiBytesRead != iCount)
		{
			return uiBytesRead;
		}

		uiWriteIndex = FindNextWriteCommand(uiWriteIndex+1);
		while (uiWriteIndex != -1)
		{
			uiBytesRead = ReadNextTouchingWrites(uiWriteIndex, pvDest, iSize, iCount);
			if (uiBytesRead != iCount)
			{
				return 0;
			}
			uiWriteIndex = FindNextWriteCommand(uiWriteIndex+1);
		}

		if ((miPosition + uiByteSize) > miLength)
		{
			uiBytesRead = (size)(miLength - miPosition) / iSize;
			miPosition += uiBytesRead * iSize;
		}
		else
		{
			uiBytesRead = iCount;
			miPosition += uiByteSize;
		}

		return uiBytesRead;
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
size CLogFile::ReadWithNoTouchingWrites(void* pvDest, size iSize, size iCount)
{
	size		iBytesReadFromFile;

	iBytesReadFromFile = ReadFromBackingFile(pvDest, iSize, iCount);
	return iBytesReadFromFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLogFile::ReadFirstTouchingWrites(size uiWriteIndex, void* pvDest, size iSize, size iCount)
{
	CArrayIntAndPointer	apvOverlapping;
	bool			bAny;
	bool			bHoles;
	size			iBytesReadFromFile;
	size			iByteSize;

	iByteSize = iSize * iCount;
	bAny = FindTouchingWriteCommands(uiWriteIndex, &apvOverlapping, miPosition, iByteSize, true);
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
		if (miBackingFileLength == 0)
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
size CLogFile::ReadNextTouchingWrites(size uiWriteIndex, void* pvDest, size iSize, size iCount)
{
	CArrayIntAndPointer		apvOverlapping;
	bool					bAny;
	size					iByteSize;

	iByteSize = iSize * iCount;
	bAny = FindTouchingWriteCommands(uiWriteIndex, &apvOverlapping, miPosition, iByteSize, true);
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
void CLogFile::CopyWritesToRead(CArrayIntAndPointer* papvOverlapping, size iByteSize, void* pvDest)
{
	size					i;
	size					iNumWrites;
	CLogFileCommandWrite*	psWrite;
	size					iIndex;
	void*					pvData;
	size					iDestOffset;
	size					iSourceOffset;
	void*					pvSource;
	void*					pvNewDest;
	size					iLength;
	filePos					iWriteEnd;
	filePos					iPositionEnd;

	iNumWrites = papvOverlapping->NumElements();
	for (i = 0; i < iNumWrites; i++)
	{
		papvOverlapping->Get(i, (void**)&psWrite, &iIndex);

		pvData = RemapSinglePointer(psWrite, sizeof(CLogFileCommandWrite));

		iLength = psWrite->iSize;
		iWriteEnd = psWrite->iPosition + psWrite->iSize;
		iPositionEnd = miPosition + iByteSize;
		if (iWriteEnd > iPositionEnd)
		{
			iLength -= (size)(iWriteEnd - iPositionEnd);
		}

		iSourceOffset = 0;
		iDestOffset = (size)(psWrite->iPosition - miPosition);
		if (psWrite->iPosition < miPosition)
		{
			iSourceOffset = (size)(miPosition - psWrite->iPosition);
			iDestOffset = 0;
		}

		iLength -= iSourceOffset;

		pvSource = RemapSinglePointer(pvData, (ptrdiff)iSourceOffset);
		pvNewDest = RemapSinglePointer(pvDest, (ptrdiff)iDestOffset);

		memcpy_fast(pvNewDest, pvSource, iLength);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLogFile::FindNextWriteCommand(size iIndex)
{
	size					i;
	CLogFileCommand*		psCommand;
	size					iNumWrites;
	CLogFileCommandOpen*	psOpen;
	bool					bFoundWriteOpen;
	size					uiNumCommands;

	iNumWrites = 0;
	bFoundWriteOpen = false;
	uiNumCommands = macCommands.NumElements();
	for (i = iIndex; i < uiNumCommands; i++)
	{
		psCommand = (CLogFileCommand*)macCommands.Get(i);
		if (psCommand->IsOpen())
		{
			psOpen = (CLogFileCommandOpen*)psCommand;
			if (IsFileModeWritable(psOpen->eMode))
			{
				bFoundWriteOpen = true;
			}
		}
		else if (psCommand->IsWrite() && bFoundWriteOpen)
		{
			return i;
		}
		else if (bFoundWriteOpen)
		{
			bFoundWriteOpen = false;
		}
	}
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLogFile::ReadFromBackingFile(void* pvDest, size iSize, size iCount)
{
	size		iByteSize;
	size		iRead;
	filePos		iLongByteSize;

	iLongByteSize = miBackingFileLength - miPosition;
	if (iLongByteSize <= 0)
	{
		return 0;
	}

	iByteSize = iSize * iCount;
	if (iByteSize > (size)iLongByteSize)
	{
		iByteSize = (size)iLongByteSize;
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
	SPointerAndSize*		psTypedPointer1;
	SPointerAndSize*		psTypedPointer2;
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
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::TestFindHoles(size uiWriteIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, size iLength)
{
	bool	bAny;

	bAny = FindTouchingWriteCommands(uiWriteIndex, papvOverlapping, iPosition, iLength, true);
	if (bAny)
	{
		return FindHoles(papvOverlapping, iPosition, iLength);
	}
	else
	{
		if (iLength == 0)
		{
			return false;
		}
		else
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::IsBegun(void)
{
	return mbBegun;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::FindHoles(CArrayIntAndPointer* papvOverlapping, filePos iPosition, size iLength)
{
	CArrayIntAndPointer		apvOverlappingSorted;  //Should be an CArraySizeAndPointer
	size					i;
	CLogFileCommandWrite*	psWrite;
	size					eCommand;
	bool					bHoles;
	filePos					iEnd;
	size					uiNumElements;

	if (iLength == 0)
	{
		return false;
	}
	if (papvOverlapping->NumElements() == 0)
	{
		return false;
	}

	apvOverlappingSorted.Init();
	apvOverlappingSorted.Copy(papvOverlapping);
	apvOverlappingSorted.QuickSort(CompareLogFileWrite);

	apvOverlappingSorted.Get(0, (void**)&psWrite, &eCommand);
	if (psWrite->iPosition > iPosition)
	{
		apvOverlappingSorted.Kill();
		return true;
	}
	else
	{
		iEnd = psWrite->iPosition + psWrite->iSize - 1;
	}

	bHoles = false;
	uiNumElements = apvOverlappingSorted.NumElements();
	for (i = 1; i < uiNumElements; i++)
	{
		apvOverlappingSorted.Get(i, (void**)&psWrite, &eCommand);
		if (psWrite->iPosition > (iEnd + 1))
		{
			bHoles = true;
			break;
		}
		iEnd = psWrite->iPosition + psWrite->iSize - 1;
		if (iEnd >= (iPosition + iLength - 1))
		{
			break;
		}
	}

	if (iEnd < (iPosition + iLength - 1))
	{
		bHoles = true;
	}

	apvOverlappingSorted.Kill();
	return bHoles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::FindTouchingWriteCommands(size iStartIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, size iLength, bool bMustOverlap)
{
	size					iIndex;
	CLogFileCommandWrite*	psWrite;
	CLogFileCommand*		psCommand;
	bool					bInitialised;
	size					uiNumCommands;

	bInitialised = false;
	if (!bMustOverlap)
	{
		iPosition--;
		iLength += 2;
	}

	uiNumCommands = macCommands.NumElements();
	for (iIndex = iStartIndex; iIndex < uiNumCommands; iIndex++)
	{
		macCommands.Get(iIndex, (void**)&psCommand);
		if (psCommand->IsWrite())
		{
			psWrite = (CLogFileCommandWrite*)psCommand;
			if (Overlaps(iPosition, iLength, psWrite))
			{
				if (!bInitialised)
				{
					papvOverlapping->Init();
					bInitialised = true;
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
bool CLogFile::Overlaps(filePos iPosition, size iLength, CLogFileCommandWrite* psWrite)
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
		return true;
	}
	if ((iNewStart <= iNextEnd) && (iNewEnd >= iNextEnd))
	{
		return true;
	}
	if ((iNewStart >= iNextStart) && (iNewStart <= iNextEnd))
	{
		return true;
	}
	return false;
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
bool CLogFile::Truncate(filePos iSize)
{
	//You should probably do something here.
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::Seek(filePos iOffset, EFileSeekOrigin eOrigin)
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::Eof(void)
{
	return miPosition >= miLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::IsOpen(void)
{
	return meFileMode != EFM_Unknown;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::Flush(void)
{
	return mpcBackingFile->Flush();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::Delete(void)
{
	if (IsOpen())
	{
		return false;
	}

	macCommands.ReInit();
	miBackingFileLength = 0;
	miLength = 0;
	return AddDeleteCommand() != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CLogFile::GetFilename(void)
{
	return mpcBackingFile->GetFilename();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLogFile::GetNumWrites(void)
{
	size				i;
	CLogFileCommand*	psCommand;
	size				iNumWrites;
	size				uiNumCommands;

	iNumWrites = 0;
	uiNumCommands = macCommands.NumElements();
	for (i = 0; i < uiNumCommands; i++)
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
CLogFileCommandWrite* CLogFile::GetWriteData(size iWrite)
{
	size				i;
	CLogFileCommand*	psCommand;
	size				iNumWrites;
	size				uiNumCommands;

	iNumWrites = 0;
	uiNumCommands = macCommands.NumElements();
	for (i = 0; i < uiNumCommands; i++)
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
size CLogFile::GetWriteSize(size iIndex)
{
	size					i;
	CLogFileCommand*		psCommand;
	size					iNumWrites;
	CLogFileCommandWrite*	psWrite;
	size					uiNumCommands;

	iNumWrites = 0;
	uiNumCommands = macCommands.NumElements();
	for (i = 0; i < uiNumCommands; i++)

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
	return ARRAY_ELEMENT_NOT_FOUND;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CLogFile::GetNumCommands(void)
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
	size					i;
	char*					pvData;
	CLogFileCommandWrite*	psWrite;
	size					iLen;
	size					uiNumCommands;

	sz.Init();
	sz.Append("Log File (?");
	sz.Append(")\n------------------\n");
	sz.Append("Commands: ");
	sz.Append(macCommands.NumElements());
	sz.AppendNewLine();

	uiNumCommands = macCommands.NumElements();
	for (i = 0; i < uiNumCommands; i++)
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
	pcLogFile->mbBasicFileMustFree = true;
	return pcLogFile;
}

