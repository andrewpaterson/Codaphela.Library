/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

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
	CChars						szError;

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
	int							i;
	void*						pvData;
	CLogFileCommand*			psCommand;
	CLogFileCommandWrite*		psWrite;
	CLogFileCommandOpen*		psOpen;
	CLogFileCommandClose*		psClose;
	CLogFileCommandDelete*		psDelete;
	bool						bResult;
	CChars						szError;
	int							iNumCommands;

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
filePos CLogFile::Write(const void* pvSource, filePos iSize, filePos iCount)
{
	CArrayIntAndPointer			apvOverlapping;
	bool					bAny;
	filePos					iByteLength;
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
bool CLogFile::AmalgamateOverlappingWrites(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, filePos iLength)
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
		return false;
	}

	pvData = RemapSinglePointer(psCommand, sizeof(CLogFileCommandWrite));

	//Copy all the overlapping chunks into the new one.
	for (i = 0; i < papvOverlapping->NumElements(); i++)
	{
		iIndex = papvOverlapping->GetType(i);
		psWrite = (CLogFileCommandWrite*)macCommands.Get(iIndex);
		pvDest = RemapSinglePointer(pvData, (ptrdiff_t)psWrite->iPosition - (ptrdiff_t)iStart);
		pvNewSource = RemapSinglePointer(psWrite, sizeof(CLogFileCommandWrite));
		memcpy_fast(pvDest, pvNewSource, (int)psWrite->iSize);
	}

	pvDest = RemapSinglePointer(pvData, (int)(iPosition - iStart));
	memcpy_fast(pvDest, (void*)pvSource, (size_t)iLength);

	//Remove all the old overlapping chunks.
	for (i = papvOverlapping->NumElements()-1; i >= 0; i--)
	{
		iIndex = papvOverlapping->GetType(i);
		macCommands.RemoveAt(iIndex, true);
	}
	return true;
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
	bool			bAny;
	bool			bHoles;
	filePos			iBytesReadFromFile;
	filePos			iByteSize;

	iByteSize = iSize * iCount;
	bAny = FindTouchingWriteCommands(iWriteIndex, &apvOverlapping, miPosition, iByteSize, true);
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
filePos CLogFile::ReadNextTouchingWrites(int iWriteIndex, void* pvDest, filePos iSize, filePos iCount)
{
	CArrayIntAndPointer	apvOverlapping;
	bool			bAny;
	filePos			iByteSize;

	iByteSize = iSize * iCount;
	bAny = FindTouchingWriteCommands(iWriteIndex, &apvOverlapping, miPosition, iByteSize, true);
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

		pvSource = RemapSinglePointer(pvData, (ptrdiff_t)iSourceOffset);
		pvNewDest = RemapSinglePointer(pvDest, (ptrdiff_t)iDestOffset);

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
	bool					bFoundWriteOpen;

	iNumWrites = 0;
	bFoundWriteOpen = false;
	for (i = iIndex; i < macCommands.NumElements(); i++)
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

	if (miPosition >= miBackingFileLength)
	{
		return 0;
	}

	iByteSize = iSize * iCount;
	if (iByteSize > (miBackingFileLength - miPosition))
	{
		iByteSize = miBackingFileLength - miPosition;
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
bool CLogFile::TestFindHoles(int iWriteIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength)
{
	bool	bAny;

	bAny = FindTouchingWriteCommands(iWriteIndex, papvOverlapping, iPosition, iLength, true);
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
bool CLogFile::FindHoles(CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength)
{
	CArrayIntAndPointer				apvOverlappingSorted;
	int							i;
	CLogFileCommandWrite*	psWrite;
	int							eCommand;
	bool						bHoles;
	filePos						iEnd;

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
		iEnd = psWrite->iPosition + psWrite->iSize -1;
	}

	bHoles = false;
	for (i = 1; i < apvOverlappingSorted.NumElements(); i++)
	{
		apvOverlappingSorted.Get(i, (void**)&psWrite, &eCommand);
		if (psWrite->iPosition > (iEnd+1))
		{
			bHoles = true;
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
		bHoles = true;
	}

	apvOverlappingSorted.Kill();
	return bHoles;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CLogFile::FindTouchingWriteCommands(int iStartIndex, CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength, bool bMustOverlap)
{
	int						iIndex;
	CLogFileCommandWrite*	psWrite;
	CLogFileCommand*		psCommand;
	bool					bInitialised;

	bInitialised = false;
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
bool CLogFile::Overlaps(filePos iPosition, filePos iLength, CLogFileCommandWrite* psWrite)
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
	//return xxx;
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
	pcLogFile->mbBasicFileMustFree = true;
	return pcLogFile;
}

