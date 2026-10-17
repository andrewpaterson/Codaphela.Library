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
#include "PointerFunctions.h"
#include "FileBasic.h"
#include "FileUtil.h"
#include "Logger.h"
#include "LogString.h"
#include "DurableFile.h"
#include "DurableSet.h"
#include "DiskFile.h"
#include "TextFile.h"


#define DURABLE_MARK_LIST_END	"#End"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::Init(const char* szMarkStartFile, const char* szMarkRewriteFile)
{
	mapcFiles.Init();
	mszMarkStart.Init(szMarkStartFile);
	mszMarkRewrite.Init(szMarkRewriteFile);
	mbBegun = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CDurableSet::Kill(void)
{
	mapcFiles.Kill();
	mszMarkStart.Kill();
	mszMarkRewrite.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Recover(void)
{
	CFileUtil	cFileUtil;
	bool		bMarkStart;
	bool		bMarkRewrite;

	//This assumes begin has NOT been called.
	if (mbBegun)
	{
		return gcLogger.Error2(__METHOD__, " Cannot recover after begun.", NULL);
	}

	bMarkStart = cFileUtil.Exists(mszMarkStart.Text());
	bMarkRewrite = cFileUtil.Exists(mszMarkRewrite.Text());

	//If neither then in theory everything is okay.
	if ((!bMarkStart) && (!bMarkRewrite))
	{
		return CheckFilesIdentical(true, true);
	}
	else 

	//Primary files were written but backup failed.
	if ((bMarkStart) && (bMarkRewrite))
	{
		gcLogger.Info2(__METHOD__, " Primary files were written but backup failed.", NULL);
		if (!CopyPrimaryToBackup() || !CopyMarkedFiles(false))
		{
			return gcLogger.Error2(__METHOD__, " Copying primary files to backup failed.", NULL);
		}
		return MarkFinish();
	}
	else

	//Primary files were not written.  Use old backup.
	if (bMarkStart)
	{
		gcLogger.Info2(__METHOD__, " Primary files were not written.  Reverting to backup.", NULL);
		if (!CopyBackupToPrimary() || !CopyMarkedFiles(true))
		{
			return gcLogger.Error2(__METHOD__, " Copying backup files to primary failed.", NULL);
		}
		return MarkFinish();
	}
	else

	//Something bad happened
	if (bMarkRewrite)
	{
		return gcLogger.Error2(__METHOD__, " Rewrite has occurred without write.", NULL);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Begin(void)
{
	if (mbBegun)
	{
		return gcLogger.Error2(__METHOD__, " Cannot begin.  Already begun.", NULL);
	}

	if (CheckWriteStatus(true))
	{
		mbBegun = true;
	}
	else
	{
		mbBegun = false;
	}
	return mbBegun;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::End(void)
{
	size			i;
	CDurableFile*	pcDurable;
	bool			bResult;
	size			uiNumElements;

	if (!mbBegun)
	{
		return gcLogger.Error2(__METHOD__, " Cannot end.  Not begun.", NULL);
	}

	if (!MarkStart())
	{
		return false;
	}

	bResult = true;
	uiNumElements = mapcFiles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult &= pcDurable->Commit();
	}
	ReturnOnFalse(bResult);

	if (!MarkRewrite())
	{
		return false;
	}

	bResult = true;
	for (i = 0; i < mapcFiles.NumElements(); i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult &= pcDurable->Recommit();
	}
	ReturnOnFalse(bResult);

	if (!MarkFinish())
	{
		return false;
	}
	mbBegun = false;

	mapcFiles.ReInit();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Check(bool bThorough, bool bLogError)
{
	CFileUtil	cFileUtil;
	bool		bMarkStart;
	bool		bMarkRewrite;

	bMarkStart = cFileUtil.Exists(mszMarkStart.Text());
	bMarkRewrite = cFileUtil.Exists(mszMarkRewrite.Text());

	if ((!bMarkStart) && (!bMarkRewrite))
	{
		return CheckFilesIdentical(bThorough, bLogError);
	}

	return CheckWriteStatus(bMarkStart, bMarkRewrite, bLogError);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CheckWriteStatus(bool bLogError)
{
	CFileUtil	cFileUtil;
	bool		bMarkStart;
	bool		bMarkRewrite;

	bMarkStart = cFileUtil.Exists(mszMarkStart.Text());
	bMarkRewrite = cFileUtil.Exists(mszMarkRewrite.Text());

	return CheckWriteStatus(bMarkStart, bMarkRewrite, bLogError);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CheckWriteStatus(bool bMarkStart, bool bMarkRewrite, bool bLogError)
{
	if ((!bMarkStart) && (!bMarkRewrite))
	{
		return true;
	}
	else if ((bMarkStart) && (bMarkRewrite))
	{
		if (bLogError)
		{
			return gcLogger.Error2(__METHOD__, " Primary files were written but re-write failed.", NULL);
		}
		return false;
	}
	else

	if (bMarkStart)
	{
		if (bLogError)
		{
			return gcLogger.Error2(__METHOD__, " Primary files were not written.", NULL);
		}
		return false;
	}
	else
	if (bMarkRewrite)
	{
		if (bLogError)
		{
			return gcLogger.Error2(__METHOD__, " Rewrite has occurred without write.", NULL);
		}
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Add(CDurableFile* pcFile)
{
	size	iIndex;
	bool	bResult;

	bResult = mapcFiles.FindInSorted(&pcFile, ComparePtrPtr, &iIndex);
	if (!bResult)
	{
		mapcFiles.InsertAt(&pcFile, iIndex);
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
size CDurableSet::Num(void)
{
	return mapcFiles.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CDurableFile* CDurableSet::Get(size iIndex)
{
	CDurableFile*	pcDurable;

	pcDurable = *mapcFiles.Get(iIndex);
	return pcDurable;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::MarkStart(void)
{
	CChars			szList;
	size			i;
	size			uiNumElements;
	CDurableFile*	pcDurable;
	bool			bResult;

	//The start mark lists the primary and rewrite name of every file in this bracket, one per line, so Recover knows which files to copy.
	//The list ends with DURABLE_MARK_LIST_END so a mark only partly written before a crash is not used.
	szList.Init();
	uiNumElements = mapcFiles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcDurable = *mapcFiles.Get(i);
		if (pcDurable->IsDurable())
		{
			szList.Append(pcDurable->GetFilename());
			szList.AppendNewLine();
			szList.Append(pcDurable->GetRewriteName());
			szList.AppendNewLine();
		}
	}
	szList.Append(DURABLE_MARK_LIST_END);
	szList.AppendNewLine();

	bResult = Mark(mszMarkStart.Text(), &szList);
	szList.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::MarkRewrite(void)
{
	return Mark(mszMarkRewrite.Text(), NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::MarkFinish(void)
{
	//The rewrite mark must go first.  A crash between the two deletes then leaves only the start mark, which recovers by copying the (already rewritten) backup over the primary.
	//Deleting the start mark first would leave only the rewrite mark, which recovery rejects.
	if (!Unmark(mszMarkRewrite.Text()))
	{
		return false;
	}
	return Unmark(mszMarkStart.Text());
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Mark(const char* szMarkFile, CChars* pszContents)
{
	CFileUtil	cFileUtil;
	CFileBasic	cFile;
	bool		bResult;

	cFileUtil.TouchDir(szMarkFile, true);
	if (pszContents == NULL)
	{
		bResult = cFileUtil.Touch(szMarkFile);
	}
	else
	{
		cFileUtil.Delete(szMarkFile);
		cFile.Init(DiskFile(szMarkFile));
		bResult = cFile.Open(EFM_Write_Create);
		if (bResult)
		{
			bResult = cFile.WriteData(pszContents->Text(), pszContents->Length());
			bResult &= cFile.Flush();
			bResult &= cFile.Close();
		}
		cFile.Kill();
	}

	if (!bResult)
	{
		return gcLogger.Error2(__METHOD__, " Could not create mark file [", szMarkFile, "].", NULL);
	}
	if (!cFileUtil.SyncParentDirectory(szMarkFile))
	{
		return gcLogger.Error2(__METHOD__, " Could not sync the directory of mark file [", szMarkFile, "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::Unmark(const char* szMarkFile)
{
	CFileUtil	cFileUtil;

	if (!cFileUtil.Delete(szMarkFile))
	{
		return gcLogger.Error2(__METHOD__, " Could not delete mark file [", szMarkFile, "].", NULL);
	}
	if (!cFileUtil.SyncParentDirectory(szMarkFile))
	{
		return gcLogger.Error2(__METHOD__, " Could not sync the directory of mark file [", szMarkFile, "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CheckFilesIdentical(bool bThorough, bool bLogError)
{
	size			i;
	CDurableFile*	pcDurable;
	bool			bResult;
	size			uiNumElements;

	uiNumElements = mapcFiles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->CheckIdentical(bThorough, bLogError);
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CopyBackupToPrimary(void)
{
	size			i;
	CDurableFile*	pcDurable;
	bool			bResult;
	size			uiNumElements;

	uiNumElements = mapcFiles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->CopyBackupToPrimary();
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CopyPrimaryToBackup(void)
{
	size			i;
	CDurableFile*	pcDurable;
	bool			bResult;
	size			uiNumElements;

	uiNumElements = mapcFiles.NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcDurable = *mapcFiles.Get(i);
		bResult = pcDurable->CopyPrimaryToBackup();
		if (!bResult)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::ReadMarkedFiles(CArrayChars* paszFiles, bool* pbComplete)
{
	CTextFile	cFile;
	CArrayChars	aszLines;
	size		i;
	size		uiNumLines;
	CChars*		pszLine;

	*pbComplete = false;
	cFile.Init();
	if (!cFile.Read(mszMarkStart.Text()))
	{
		cFile.Kill();
		return gcLogger.Error2(__METHOD__, " Could not read mark file [", mszMarkStart.Text(), "].", NULL);
	}

	if (cFile.mcText.Empty())
	{
		//Mark files written before the list was added, or by hand, are empty and list no files.
		cFile.Kill();
		*pbComplete = true;
		return true;
	}

	cFile.PassifyNewlines();
	aszLines.Init();
	cFile.mcText.SplitLines(&aszLines);
	cFile.Kill();

	uiNumLines = aszLines.NumElements();
	for (i = 0; i < uiNumLines; i++)
	{
		pszLine = aszLines.Get(i);
		if (pszLine->Equals(DURABLE_MARK_LIST_END))
		{
			*pbComplete = true;
			break;
		}
		if (!pszLine->Empty())
		{
			paszFiles->Add(pszLine->Text());
		}
	}
	aszLines.Kill();

	if (*pbComplete && ((paszFiles->NumElements() % 2) != 0))
	{
		return gcLogger.Error2(__METHOD__, " Mark file [", mszMarkStart.Text(), "] has a primary file without a rewrite file.", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CopyMarkedFiles(bool bBackupToPrimary)
{
	CArrayChars	aszFiles;
	bool		bComplete;
	bool		bResult;
	size		i;
	size		uiNumFiles;
	char*		szPrimary;
	char*		szRewrite;

	aszFiles.Init();
	bResult = ReadMarkedFiles(&aszFiles, &bComplete);
	if (!bResult)
	{
		aszFiles.Kill();
		return false;
	}

	if (!bComplete)
	{
		aszFiles.Kill();
		if (bBackupToPrimary)
		{
			//The crash happened while writing the start mark so no primary file was written yet.
			gcLogger.Info2(__METHOD__, " Mark file [", mszMarkStart.Text(), "] is incomplete.  No files were changed.", NULL);
			return true;
		}
		return gcLogger.Error2(__METHOD__, " Mark file [", mszMarkStart.Text(), "] is incomplete but the rewrite mark exists.", NULL);
	}

	uiNumFiles = aszFiles.NumElements();
	for (i = 0; i < uiNumFiles; i += 2)
	{
		szPrimary = aszFiles.GetText(i);
		szRewrite = aszFiles.GetText(i + 1);
		if (bBackupToPrimary)
		{
			bResult = CopyRecoveredFile(szRewrite, szPrimary);
		}
		else
		{
			bResult = CopyRecoveredFile(szPrimary, szRewrite);
		}
		if (!bResult)
		{
			aszFiles.Kill();
			return false;
		}
	}
	aszFiles.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::CopyRecoveredFile(const char* szSource, const char* szDest)
{
	CFileUtil	cFileUtil;

	if (!cFileUtil.Copy(szSource, szDest))
	{
		return gcLogger.Error2(__METHOD__, " Could not copy [", szSource, "] to [", szDest, "].", NULL);
	}
	if (!cFileUtil.Compare(szSource, szDest))
	{
		return gcLogger.Error2(__METHOD__, " Copy of [", szSource, "] to [", szDest, "] is different.", NULL);
	}

	//Copy deletes the destination when the source does not exist, so only the directory may need syncing.
	if (cFileUtil.Exists(szDest))
	{
		if (!cFileUtil.SyncFile(szDest))
		{
			return gcLogger.Error2(__METHOD__, " Could not sync recovered file [", szDest, "].", NULL);
		}
	}
	if (!cFileUtil.SyncParentDirectory(szDest))
	{
		return gcLogger.Error2(__METHOD__, " Could not sync the directory of recovered file [", szDest, "].", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CDurableSet::HasBegun(void)
{
	return mbBegun;
}

