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
#include "DurableFile.h"
#include "DurableFileController.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::Init(CDurableFileController* pcController, char* szFileName, char* szRewriteName)
{
	CDiskFile*	pcPrimaryDiskFile;
	CDiskFile*	pcRewriteDiskFile;

	mpcController = pcController;
	mbBegun = FALSE;
	meOpenMode = EFM_Unknown;

	miPosition = 0;
	miLength = 0;
	miFileLength = 0;
	mbTouched = FALSE;

	mcWrites.Init(COMMAND_CHUNK_SIZE);

	mpcPrimaryFile = NewMalloc<CFileBasic>();
	mpcRewriteFile = NewMalloc<CFileBasic>();

	if (szFileName)
	{
		pcPrimaryDiskFile = DiskFile(szFileName);
		mpcPrimaryFile->Init(pcPrimaryDiskFile);
		mszFileName.Init(pcPrimaryDiskFile->mszFileName);
	}
	else
	{
		mszFileName.Init();
	}

	if (szRewriteName)
	{
		pcRewriteDiskFile = DiskFile(szRewriteName);
		mpcRewriteFile->Init(pcRewriteDiskFile);
		mszRewriteName.Init(pcRewriteDiskFile->mszFileName);
	}
	else
	{
		mszRewriteName.Init();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Kill(void)
{
	BOOL	bAnyOpen;

	bAnyOpen = mpcPrimaryFile->IsOpen();
	bAnyOpen &= mpcRewriteFile->IsOpen();

	mpcPrimaryFile->Kill();
	mpcRewriteFile->Kill();

	mcWrites.Kill();

	mszFileName.Kill();
	mszRewriteName.Kill();

	free(mpcPrimaryFile);
	free(mpcRewriteFile);

	return !bAnyOpen;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Begin(void)
{
	if (IsDurable())
	{
		if (mbBegun)
		{
			return FALSE;
		}

		OpenFilesForBegin();

		mcWrites.Kill();
		mcWrites.Init(COMMAND_CHUNK_SIZE);

		mbBegun = TRUE;
		mbTouched = FALSE;
		return TRUE;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::OpenFilesForBegin(void)
{
	BOOL	bFileOpened;

	if (!mpcPrimaryFile->IsOpen())
	{
		bFileOpened = mpcPrimaryFile->Open(EFM_ReadWrite);
		if (bFileOpened)
		{
			miFileLength = mpcPrimaryFile->GetFileLength();
			miLength = miFileLength;
		}
		else
		{
			miFileLength = 0;
		}
	}
	else
	{
		miFileLength = mpcPrimaryFile->GetFileLength();
		miLength = miFileLength;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::OpenFilesForEnd(CFileBasic* pcFile)
{
	BOOL bFileOpened;

	if (!pcFile->IsOpen())
	{
		bFileOpened = pcFile->Open(EFM_ReadWrite_Create);
		if (!bFileOpened)
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
BOOL CDurableFile::End(void)
{
	BOOL	bResult;

	if (IsDurable())
	{
		if (!mbBegun)
		{
			return FALSE;
		}

		if (!mbTouched)
		{
			mbBegun = FALSE;
			return TRUE;
		}

		bResult = OpenFilesForEnd(mpcPrimaryFile);
		if (!bResult)
		{
			return FALSE;
		}

		bResult = WriteWriteCommands(mpcPrimaryFile);
		if (!bResult)
		{
			return FALSE;
		}

		mpcPrimaryFile->Flush();
		miPosition = mpcPrimaryFile->GetFilePos();

		mbBegun = FALSE;
		return TRUE;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Rewrite(void)
{
	BOOL	bResult;

	if (IsDurable())
	{
		if (!mbTouched)
		{
			return TRUE;
		}

		bResult = OpenFilesForEnd(mpcRewriteFile);
		if (!bResult)
		{
			return FALSE;
		}

		bResult = WriteWriteCommands(mpcRewriteFile);
		if (!bResult)
		{
			return FALSE;
		}

		mpcRewriteFile->Flush();

		mcWrites.Kill();
		mcWrites.Init(COMMAND_CHUNK_SIZE);

		return TRUE;
	}
	else
	{
		//Rewriting a non-durable file makes no sense.
		//Yes it does.  For consistency.
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::WriteWriteCommands(CFileBasic* pcFile)
{
	int							i;
	void*						pvData;
	SDurableFileCommandWrite*	psWrite;
	BOOL						bResult;

	//Non-durable files cannot hit this code.

	for (i = 0; i < mcWrites.NumElements(); i++)
	{
		mcWrites.Get(i, &pvData);
		psWrite = (SDurableFileCommandWrite*)pvData;
		pcFile->Seek(psWrite->iPosition, EFSO_SET);
		pvData = RemapSinglePointer(psWrite, sizeof(SDurableFileCommandWrite));

		bResult = pcFile->WriteData(pvData, psWrite->iSize);
		if (!bResult)
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
BOOL CDurableFile::Open(void)
{
	BOOL bFileOpened;

	if ((mszFileName.Empty()))
	{
		return FALSE;
	}

	if (IsDurable())
	{
		if (mszRewriteName.Empty())
		{
			return FALSE;
		}

		bFileOpened = mpcPrimaryFile->Open(EFM_ReadWrite);
		miPosition = 0;
		if (bFileOpened)
		{
			miFileLength = mpcPrimaryFile->GetFileLength();
		}
		else
		{
			miFileLength = 0;
		}
		miLength = miFileLength;
		return TRUE;
	}
	else
	{
		bFileOpened = mpcPrimaryFile->Open(EFM_ReadWrite_Create);
		if (bFileOpened)
		{
			miFileLength = mpcPrimaryFile->GetFileLength();
			miLength = miFileLength;
			return TRUE;
		}
		else
		{
			return FALSE;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Close(void)
{
	if (mbBegun)
	{
		return FALSE;
	}

	if (mpcPrimaryFile->IsOpen())
	{
		mpcPrimaryFile->Close();
	}
	if (mpcRewriteFile->IsOpen())
	{
		mpcRewriteFile->Close();
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::Seek(EFileSeekOrigin eOrigin, filePos iDistance)
{
	if (mpcController->IsBegun())
	{
		if (IsDurable())
		{
			SeekDurable(eOrigin, iDistance);
		}
		else
		{
			SeekNonDurable(eOrigin, iDistance);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::SeekDurable(EFileSeekOrigin eOrigin, filePos iDistance)
{
	if (eOrigin == EFSO_SET)
	{
		miPosition = iDistance;
	}
	else if (eOrigin == EFSO_CURRENT)
	{
		miPosition += iDistance;
	}
	else if (eOrigin == EFSO_END)
	{
		miPosition = miLength + iDistance;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::SeekNonDurable(EFileSeekOrigin eOrigin, filePos iDistance)
{
	if (mpcPrimaryFile->IsOpen())
	{
		mpcPrimaryFile->Seek(iDistance, eOrigin);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Write(filePos iDistance, const void* pvSource, filePos iSize, filePos iCount)
{
	return Write(EFSO_SET, iDistance, pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Write(EFileSeekOrigin eOrigin, filePos iDistance, const void* pvSource, filePos iSize, filePos iCount)
{
	Seek(eOrigin, iDistance);
	return Write(pvSource, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Write(const void* pvSource, filePos iSize, filePos iCount)
{
	if (IsDurable())
	{
		if (mbBegun)
		{
			return WriteDurable(pvSource, iSize, iCount);
		}
		return 0;
	}
	else
	{
		return mpcPrimaryFile->Write(pvSource, iSize, iCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::WriteDurable(const void* pvSource, filePos iSize, filePos iCount)
{
	SDurableFileCommandWrite*	psCommand;
	CArrayIntAndPointer			apvOverlapping;
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
		psCommand = (SDurableFileCommandWrite*)mcWrites.Add(sizeof(SDurableFileCommandWrite) + (int)iByteLength);
		if (!psCommand)
		{
			return 0;
		}
		pvData = RemapSinglePointer(psCommand, sizeof(SDurableFileCommandWrite));

		psCommand->iSize = iByteLength;
		psCommand->iPosition = miPosition;
		memcpy(pvData, (void*)pvSource, (int)iByteLength);

		miPosition += iByteLength;
	}
	
	if (miPosition > miLength)
	{
		miLength = miPosition;
	}

	return iCount;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::AmalgamateOverlappingWrites(CArrayIntAndPointer* papvOverlapping, const void* pvSource, filePos iPosition, filePos iLength)
{
	filePos						iStart;
	filePos						iEnd;  //Inclusive;
	int							i;
	SDurableFileCommandWrite*	psWrite;
	int							iIndex;
	filePos						iIndeedSize;
	SDurableFileCommandWrite*	psCommand;
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
	psCommand = (SDurableFileCommandWrite*)mcWrites.Add(sizeof(SDurableFileCommandWrite) + (int)iIndeedSize);
	if (!psCommand)
	{
		return FALSE;
	}

	pvData = RemapSinglePointer(psCommand, sizeof(SDurableFileCommandWrite));

	psCommand->iSize = iIndeedSize;
	psCommand->iPosition = iStart;

	//Copy all the overlapping chunks into the new one.
	for (i = 0; i < papvOverlapping->NumElements(); i++)
	{
		iIndex = papvOverlapping->GetType(i);
		psWrite = (SDurableFileCommandWrite*)mcWrites.Get(iIndex);
		pvDest = RemapSinglePointer(pvData, (int)(psWrite->iPosition - iStart));
		pvNewSource = RemapSinglePointer(psWrite, sizeof(SDurableFileCommandWrite));
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
filePos CDurableFile::Read(filePos iDistance, void* pvDest, filePos iSize, filePos iCount)
{
	return Read(EFSO_SET, iDistance, pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Read(EFileSeekOrigin eOrigin, filePos iDistance, void* pvDest, filePos iSize, filePos iCount)
{
	Seek(eOrigin, iDistance);
	return Read(pvDest, iSize, iCount);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Read(void* pvDest, filePos iSize, filePos iCount)
{
	if ((IsDurable()) && (mbBegun))
	{
		return ReadDurable(pvDest, iSize, iCount);
	}
	else
	{
		return ReadNonDurable(pvDest, iSize, iCount);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::ReadDurable(void* pvDest, filePos iSize, filePos iCount)
{
	CArrayIntAndPointer			apvOverlapping;
	BOOL						bAnyTouchingWriteCommands;
	int							i;
	BOOL						bHoles;
	filePos						iResult;
	int							iIndex;
	SDurableFileCommandWrite*	psWrite;
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

	bAnyTouchingWriteCommands = FindTouchingWriteCommands(&apvOverlapping, miPosition, iByteSize, TRUE);
	if (bAnyTouchingWriteCommands)
	{
		bHoles = DoOverlappingWritesContainHoles(&apvOverlapping, miPosition, iByteSize);
		if (bHoles)
		{
			iBytesReadFromFile = ReadFromPrimaryFile(pvDest, iSize, iCount);
		}

		for (i = 0; i < apvOverlapping.NumElements(); i++)
		{
			apvOverlapping.Get(i, (void**)&psWrite, &iIndex);

			pvData = RemapSinglePointer(psWrite, sizeof(SDurableFileCommandWrite));

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
			iResult = (miLength - miPosition) / iSize;
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
		iBytesReadFromFile = ReadFromPrimaryFile(pvDest, iSize, iCount);
		iResult = iBytesReadFromFile/iSize;
		miPosition += iResult*iSize;
		return iResult;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::ReadNonDurable(void* pvDest, filePos iSize, filePos iCount)
{
	if (mpcPrimaryFile->IsOpen())
	{
		return mpcPrimaryFile->Read(pvDest, iSize, iCount);
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::ReadFromPrimaryFile(void* pvDest, filePos iSize, filePos iCount)
{
	BOOL		bResult;
	filePos		iBytes;

	if (miPosition >= miFileLength)
	{
		return 0;
	}

	iBytes = iSize * iCount;
	if (iBytes > (miFileLength - miPosition))
	{
		iBytes = miFileLength - miPosition;
	}

	if (mpcPrimaryFile->IsOpen())
	{
		mpcPrimaryFile->Seek(miPosition, EFSO_SET);
		bResult = mpcPrimaryFile->ReadData(pvDest, iBytes);
		if (!bResult)
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}


	return iBytes;  //Not the count.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareDurableWrite(const void* pv1, const void* pv2)
{
	SPointerAndSize*			psTypedPointer1;
	SPointerAndSize*			psTypedPointer2;
	SDurableFileCommandWrite*	psWrite1;
	SDurableFileCommandWrite*	psWrite2;

	psTypedPointer1 = (SPointerAndSize*)pv1;
	psTypedPointer2 = (SPointerAndSize*)pv2;

	psWrite1 = (SDurableFileCommandWrite*)psTypedPointer1->pvData;
	psWrite2 = (SDurableFileCommandWrite*)psTypedPointer2->pvData;

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
BOOL CDurableFile::DoOverlappingWritesContainHoles(CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength)
{
	CArrayIntAndPointer			apvOverlappingSorted;
	int							i;
	SDurableFileCommandWrite*	psWrite;
	int							eCommand;
	BOOL						bHoles;
	filePos						iEnd;

	apvOverlappingSorted.Init(1);
	apvOverlappingSorted.Copy(papvOverlapping);
	apvOverlappingSorted.QuickSort(CompareDurableWrite);

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
BOOL CDurableFile::FindTouchingWriteCommands(CArrayIntAndPointer* papvOverlapping, filePos iPosition, filePos iLength, BOOL bMustOverlap)
{
	int							i;
	SDurableFileCommandWrite*	psWrite;
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
		if (DoesWriteOverlap(psWrite, iPosition, iLength))
		{
			if (!bInitialised)
			{
				papvOverlapping->Init(8);
				bInitialised = TRUE;
			}

			//The index in 'type' is used in the method AmalgamateOverlappingWrites because reasons.
			papvOverlapping->Add(psWrite, i /* <-- Cheating like Hell */);
		}
	}

	return bInitialised;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::DoesWriteOverlap(SDurableFileCommandWrite* psWrite, filePos iPosition, filePos iLength)
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
filePos CDurableFile::Tell(void)
{
	if (mbBegun)
	{
		return miPosition;
	}
	else
	{
		return mpcPrimaryFile->GetFilePos();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::Size(void)
{
	if (mbBegun)
	{
		return SizeDurable();
	}
	else
	{
		return SizeNonDurable();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::SizeDurable(void)
{
	return miLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::SizeNonDurable(void)
{
	if (mpcPrimaryFile->IsOpen())
	{
		return mpcPrimaryFile->GetFileSize();
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
BOOL CDurableFile::IsEof(void)
{
	if (mbBegun)
	{
		return miPosition >= miLength;
	}
	else
	{
		return mpcPrimaryFile->IsEndOfFile();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::IsDurable(void)
{
	if (mpcController != NULL)
	{
		return mpcController->IsDurable();
	}
	else
	{
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::TestGetPosition(void)
{
	return miPosition;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CDurableFile::TestGetLength(void)
{
	return miLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CDurableFile::GetNumWrites(void)
{
	return mcWrites.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void* CDurableFile::GetWriteData(int iWrites)
{
	return RemapSinglePointer(mcWrites.Get(iWrites), sizeof(SDurableFileCommandWrite));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::CheckIdentical(void)
{
	CFileUtil	cFileUtil;

	if (mbBegun)
	{
		return FALSE;
	}
	return cFileUtil.Compare(mszFileName.Text(), mszRewriteName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::Delete(void)
{
	CFileUtil	cFileUtil;
	BOOL		bResult;

	if (mbBegun)
	{
		return FALSE;
	}
	else
	{
		Close();
		if (IsDurable())
		{
			bResult = cFileUtil.Delete(mszFileName.Text());
			bResult &= cFileUtil.Delete(mszRewriteName.Text());
			return bResult;
		}
		else
		{
			return cFileUtil.Delete(mszFileName.Text());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::CopyBackupToPrimary(void)
{
	CFileUtil	cFileUtil;

	if (mbBegun)
	{
		return FALSE;
	}
	if (!cFileUtil.Copy(mszRewriteName.Text(), mszFileName.Text()))
	{
		return FALSE;
	}
	return cFileUtil.Compare(mszFileName.Text(), mszRewriteName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CDurableFile::CopyPrimaryToBackup(void)
{
	CFileUtil	cFileUtil;

	if (mbBegun)
	{
		return FALSE;
	}
	if (!cFileUtil.Copy(mszFileName.Text(), mszRewriteName.Text()))
	{
		return FALSE;
	}

	return cFileUtil.Compare(mszFileName.Text(), mszRewriteName.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableFile::Dump(void)
{
	CChars						sz;
	int							i;
	char*						pvData;
	SDurableFileCommandWrite*	psWrite;
	filePos						iLen;

	sz.Init();
	sz.Append("Durable File (");
	sz.Append(mszFileName);
	sz.Append(")\n------------------\n");
	sz.Append("Commands: ");
	sz.Append(mcWrites.NumElements());
	sz.AppendNewLine();

	for (i = 0; i < mcWrites.NumElements(); i++)
	{
		mcWrites.Get(i, (void**)&pvData);
		sz.Append("   ");
		sz.Append("Write: (");
		psWrite = (SDurableFileCommandWrite*)pvData;
		sz.Append(psWrite->iPosition);
		sz.Append(", ");
		sz.Append(psWrite->iSize);
		sz.Append(") ");

		pvData = (char*)RemapSinglePointer(psWrite, sizeof(SDurableFileCommandWrite));
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
CFileBasic*	CDurableFile::DumpGetPrimaryFile(void)
{
	return mpcPrimaryFile;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* FindSecondString(char* szFileName)
{
	int		        iLen;
	int		        i;
	unsigned char*	szSecondStart;

	iLen = (int)strlen(szFileName);
	szSecondStart = (unsigned char*)RemapSinglePointer(szFileName, iLen+1);
	for (i = 0; i < 32768; i++)
	{
		if (i >= 1)
		{
			if (szSecondStart[i] == 0)
			{
				return (char*)szSecondStart;
			}
		}
		if ((szSecondStart[i] < 32) || (szSecondStart[i] >= 128))
		{
			return NULL;
		}
	}
	return NULL;
}

