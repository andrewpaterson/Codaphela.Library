/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
	mbSync = true;

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
	if (!mbBegun)
	{
		return FailBegun(__METHOD__, "end", mpcBackingFile);
	}

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
void CLogFile::Abort(void)
{
	//Throws the commands away without writing anything.
	if (mbOpenedBackingFile)
	{
		mpcBackingFile->Close();
		mbOpenedBackingFile = false;
	}
	if (mbBegun)
	{
		End();
	}
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
	const char*	pszFilename;

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
bool CLogFile::FailBegun(char* szMethod, const char* szTask, CAbstractFile* pcFile)
{
	CChars	szError;

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


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::FailFlush(char* szMethod, CAbstractFile* pcFile)
{
	CChars	szError;

	szError.Init(szMethod);
	szError.Append(" Could not flush file [");
	GetSafeFilename(&szError, pcFile);
	szError.Append("] to disk.");

	gcLogger.Error(szError.Text());
	szError.Kill();
	return false;
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
	CLogFileCommandTruncate*	psTruncate;
	bool						bResult;
	CChars						szError;
	size						iNumCommands;
	bool						bWritten;
	bool						bDirectoryChanged;
	CFileUtil					cFileUtil;

	if (!mbBegun)
	{
		return FailBegun(__METHOD__, "commit", pcFile);
	}

	//Written data must be on disk before the file is closed, and created or deleted files must have their directory entries on disk.
	//Unless syncing is turned off, then the operating system writes them when it chooses.
	bWritten = false;
	bDirectoryChanged = false;
	iNumCommands = macCommands.NumElements();
	for (i = 0; i < iNumCommands; i++)
	{
		macCommands.Get(i, &pvData);
		psCommand = (CLogFileCommand*)pvData;

		switch (psCommand->eCommand)
		{
		case LFC_Write:
			psWrite = (CLogFileCommandWrite*)psCommand;
			bResult = psWrite->Write(pcFile);
			bWritten = true;
			break;
		case LFC_Open:
			psOpen = (CLogFileCommandOpen*)psCommand;
			bResult = psOpen->Open(pcFile);
			bDirectoryChanged |= IsFileModeCreate(psOpen->eMode);
			break;
		case LFC_Close:
			if (bWritten && mbSync)
			{
				if (!pcFile->Flush())
				{
					return FailFlush(__METHOD__, pcFile);
				}
				bWritten = false;
			}
			psClose = (CLogFileCommandClose*)psCommand;
			bResult = psClose->Close(pcFile);
			break;
		case LFC_Delete:
			psDelete = (CLogFileCommandDelete*)psCommand;
			bResult = psDelete->Delete(pcFile);
			bDirectoryChanged = true;
			break;
		case LFC_Truncate:
			psTruncate = (CLogFileCommandTruncate*)psCommand;
			bResult = psTruncate->Truncate(pcFile);
			bWritten = true;
			break;
		default:
			bResult = false;
			break;
		}

		if (!bResult)
		{
			ExecuteCommandErrorString(&szError, __METHOD__, psCommand, pcFile);
			gcLogger.Error(szError.Text()); 
			szError.Kill();
			return false;
		}
	}

	if (bWritten && mbSync)
	{
		if (!pcFile->Flush())
		{
			return FailFlush(__METHOD__, pcFile);
		}
	}

	if (bDirectoryChanged && mbSync && pcFile->GetFilename())
	{
		if (!cFileUtil.SyncParentDirectory(pcFile->GetFilename()))
		{
			return gcLogger.Error2(__METHOD__, " Could not sync the directory of file [", pcFile->GetFilename(), "].", NULL);
		}
	}

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
		return NULL;
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
CLogFileCommandTruncate* CLogFile::AddTruncateCommand(filePos iSize)
{
	CLogFileCommandTruncate*	psCommand;

	psCommand = (CLogFileCommandTruncate*)macCommands.Add(sizeof(CLogFileCommandTruncate));
	if (!psCommand)
	{
		return NULL;
	}

	psCommand->Init(iSize);
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
			if (!ExtendLastWrite(&apvOverlapping, pvSource, miPosition, iByteLength))
			{
				AmalgamateOverlappingWrites(&apvOverlapping, pvSource, miPosition, iByteLength);
			}
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
// Writing just after (or inside) the last write grows that write in place.
// Amalgamating would copy everything written so far on every append.
// The result is the same as AmalgamateOverlappingWrites: one write command,
// last in the list, covering both.
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::ExtendLastWrite(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, size iLength)
{
	CLogFileCommandWrite*	psWrite;
	uint					iIndex;
	size					uiNewSize;
	size					uiNeeded;
	size					uiAllocated;
	void*					pvDest;

	if (papvOverlapping->NumElements() != 1)
	{
		return false;
	}

	papvOverlapping->Get(0, (void**)&psWrite, &iIndex);
	if (((size)iIndex != macCommands.NumElements() - 1) || (iPosition < psWrite->iPosition))
	{
		return false;
	}

	uiNewSize = (size)(iPosition - psWrite->iPosition) + iLength;
	if (uiNewSize < psWrite->iSize)
	{
		uiNewSize = psWrite->iSize;
	}

	uiNeeded = sizeof(CLogFileCommandWrite) + uiNewSize;
	uiAllocated = macCommands.GetSize(iIndex);
	if (uiNeeded > uiAllocated)
	{
		//Double so a run of appends is copied a logarithmic number of times.
		if (uiNeeded < uiAllocated * 2)
		{
			uiNeeded = uiAllocated * 2;
		}
		psWrite = (CLogFileCommandWrite*)macCommands.Grow(iIndex, uiNeeded);
		if (!psWrite)
		{
			return false;
		}
	}

	pvDest = RemapSinglePointer(psWrite->GetData(), (ptrdiff)(iPosition - psWrite->iPosition));
	memcpy_fast(pvDest, (void*)pvSource, iLength);
	psWrite->iSize = uiNewSize;
	return true;
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
	uint					iIndex;
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

		if ((psWrite->iPosition + (filePos)psWrite->iSize - 1) > iEnd)
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
		while (uiWriteIndex != ARRAY_ELEMENT_NOT_FOUND)
		{
			uiBytesRead = ReadNextTouchingWrites(uiWriteIndex, pvDest, iSize, iCount);
			if (uiBytesRead != iCount)
			{
				return 0;
			}
			uiWriteIndex = FindNextWriteCommand(uiWriteIndex+1);
		}

		if (miPosition >= miLength)
		{
			return 0;
		}
		else if ((miPosition + (filePos)uiByteSize) > miLength)
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
	size			iByteSize;

	iByteSize = iSize * iCount;
	bAny = FindTouchingWriteCommands(uiWriteIndex, &apvOverlapping, miPosition, iByteSize, true);
	if (bAny)
	{
		bHoles = FindHoles(&apvOverlapping, miPosition, iByteSize);
		if (bHoles)
		{
			//Holes past the end of the backing file read as zeros, as they do in a disk file.
			memset(pvDest, 0, iByteSize);
			ReadFromBackingFile(pvDest, iSize, iCount);
		}

		CopyWritesToRead(&apvOverlapping, iByteSize, pvDest);
		apvOverlapping.Kill();
		return iCount;
	}
	else
	{
		//Writes or a truncation elsewhere can make the file longer than the backing file.  The rest reads as zeros and Read limits it to the file length.
		memset(pvDest, 0, iByteSize);
		if (miBackingFileLength != 0)
		{
			ReadWithNoTouchingWrites(pvDest, iSize, iCount);
		}
		return iCount;
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
	uint					iIndex;
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
	CLogFileCommandOpen*	psOpen;
	bool					bFoundWriteOpen;
	size					uiNumCommands;

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
		else if (psCommand->IsTruncate())
		{
			//A truncation happens while the file is open and does not end the writes.
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
void CLogFile::SetSync(bool bSync)
{
	mbSync = bSync;
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
	uint					eCommand;
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
		if (iEnd >= (iPosition + (filePos)iLength - 1))
		{
			break;
		}
	}

	if (iEnd < (iPosition + (filePos)iLength - 1))
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

				papvOverlapping->Add(psWrite, (uint)iIndex);
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
	size						i;
	CLogFileCommand*			psCommand;
	CLogFileCommandWrite*		psWrite;
	size						uiGrowth;

	if (!IsFileModeWritable(meFileMode) || (iSize < 0))
	{
		return false;
	}

	mbTouched = true;
	if (iSize > miLength)
	{
		//Growing fills with zeros, as a disk file does.
		uiGrowth = (size)(iSize - miLength);
		psWrite = AddWriteCommand(miLength, uiGrowth);
		if (!psWrite)
		{
			return false;
		}
		memset(RemapSinglePointer(psWrite, sizeof(CLogFileCommandWrite)), 0, uiGrowth);
		miLength = iSize;
	}
	else if (iSize < miLength)
	{
		//Writes past the new end are cut back or removed, and the backing file is no longer read past it.
		i = macCommands.NumElements();
		while (i > 0)
		{
			i--;
			psCommand = (CLogFileCommand*)macCommands.Get(i);
			if (psCommand->IsWrite())
			{
				psWrite = (CLogFileCommandWrite*)psCommand;
				if (psWrite->iPosition >= iSize)
				{
					macCommands.RemoveAt(i, true);
				}
				else if ((psWrite->iPosition + (filePos)psWrite->iSize) > iSize)
				{
					psWrite->iSize = (size)(iSize - psWrite->iPosition);
				}
			}
		}

		if (miBackingFileLength > iSize)
		{
			miBackingFileLength = iSize;
		}

		if (!AddTruncateCommand(iSize))
		{
			return false;
		}
		miLength = iSize;
	}
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
const char* CLogFile::GetFilename(void)
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
void CLogFile::Print(CChars* psz)
{
	size						i;
	CLogFileCommand*			psCommand;
	CLogFileCommandWrite*		psWrite;
	CLogFileCommandOpen*		psOpen;
	CLogFileCommandTruncate*	psTruncate;
	size						uiNumCommands;
	size						uiShown;

	uiNumCommands = macCommands.NumElements();
	psz->Append("Log File [");
	GetSafeFilename(psz, mpcBackingFile);
	psz->Append("] Commands: ");
	psz->Append(uiNumCommands);
	psz->AppendNewLine();

	for (i = 0; i < uiNumCommands; i++)
	{
		psCommand = (CLogFileCommand*)macCommands.Get(i);
		psz->Append("   ");
		psz->Append(psCommand->GetType());
		switch (psCommand->eCommand)
		{
		case LFC_Write:
			psWrite = (CLogFileCommandWrite*)psCommand;
			psz->Append(" (");
			psz->Append(psWrite->iPosition);
			psz->Append(", ");
			psz->Append(psWrite->iSize);
			psz->Append(") ");
			uiShown = (psWrite->iSize > 80) ? 80 : psWrite->iSize;
			psz->AppendData((char*)psWrite->GetData(), uiShown);
			break;
		case LFC_Open:
			psOpen = (CLogFileCommandOpen*)psCommand;
			psz->Append(" (");
			psz->Append((int)psOpen->eMode);
			psz->Append(")");
			break;
		case LFC_Truncate:
			psTruncate = (CLogFileCommandTruncate*)psCommand;
			psz->Append(" (");
			psz->Append(psTruncate->iSize);
			psz->Append(")");
			break;
		default:
			break;
		}
		psz->AppendNewLine();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLogFile::Dump(void)
{
	CChars	sz;

	sz.Init();
	Print(&sz);
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

