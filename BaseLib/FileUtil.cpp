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
#include "FileUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "Define.h"
#include "StringHelper.h"
#include "FileBasic.h"
#include "ArrayChars.h"
#include "DiskFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::MakeDirs(bool bRemoveFirst, const char* szPathName, ...)
{
	va_list		vaMarker;
	char*		pc;
	size		iCount;

	iCount = 0;
	pc = (char*)szPathName;

	va_start(vaMarker, szPathName);
	while (pc != NULL)
	{
		if (bRemoveFirst)
		{
			RemoveDir(pc);
		}
		if (!TouchDir(pc, false))
		{
			return false;
		}

		iCount++;
		pc = va_arg(vaMarker, char*);
	}
	va_end(vaMarker);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::RemoveDirs(const char* szPathName, ...)
{
	va_list		vaMarker;
	char*		pc;
	size		iCount;
	bool		bResult;

	iCount = 0;
	pc = (char*)szPathName;

	bResult = true;
	va_start(vaMarker, szPathName);
	while (pc != NULL)
	{
		bResult &= RemoveDir(pc);

		iCount++;
		pc = va_arg(vaMarker, char*);
	}
	va_end(vaMarker);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::FullPath(CChars* szPathName)
{
	CChars	szTemp;

	if (!szPathName->Empty())
	{
		if (IsAbsolutePath(szPathName->Text()))
		{
			CollapsePath(szPathName);
		}
		else
		{
			szTemp.Init();
			CurrentDirectory(&szTemp);
			szTemp.Append(FILE_SEPARATOR);
			szTemp.Append(szPathName);
			szPathName->Set(szTemp);
			szTemp.Kill();
			FixSeparators(szPathName);
			CollapsePath(szPathName);
			RemoveLastSeparator(szPathName);
		}
	}
	else
	{
		CurrentDirectory(szPathName);
		RemoveLastSeparator(szPathName);
	}
}



////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
bool CFileUtil::IsRootDirectory(const char* szPathName)
{
	CChars	sz;
	size	iCount;
	size	iEnd;

	sz.Init(szPathName);
	FullPath(&sz);
	CollapsePath(&sz);
	iCount = sz.Count(FILE_SEPARATOR[0]);
	if (iCount == 1)
	{
		iEnd = sz.FindFromEnd(FILE_SEPARATOR[0]);
		if (iEnd == sz.Length() - 1)
		{
			sz.Kill();
			return true;
		}
	}
	sz.Kill();

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::CompareSize(const char* szFilename1, const char* szFilename2)
{
	return Compare(szFilename1, szFilename2, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::CopyDir(const char* szSource, const char* szDest)
{
	CChars	szFullSource;
	CChars	szFullDest;
	bool	bInside;

	if (!IsDirectory(szSource))
	{
		return gcLogger.Error2(__METHOD__, " Source [", szSource, "] is not a directory.", NULL);
	}

	//Copying a directory into itself would never finish.
	szFullSource.Init(szSource);
	FullPath(&szFullSource);
	szFullDest.Init(szDest);
	FullPath(&szFullDest);
	bInside = szFullDest.Equals(szFullSource.Text());
	szFullSource.Append(FILE_SEPARATOR);
	bInside |= szFullDest.StartsWith(szFullSource.Text());
	szFullSource.Kill();
	szFullDest.Kill();
	if (bInside)
	{
		return gcLogger.Error2(__METHOD__, " Destination [", szDest, "] is inside source [", szSource, "].", NULL);
	}

	TouchDir(szDest, false);
	if (!IsDirectory(szDest))
	{
		return gcLogger.Error2(__METHOD__, " Could not create destination directory [", szDest, "].", NULL);
	}
	return RecurseCopyDir(szSource, szDest);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::CopyDirEntry(const char* szSource, const char* szDest, bool bDirectory)
{
	if (bDirectory)
	{
		MakeDir(szDest);
		if (!IsDirectory(szDest))
		{
			return gcLogger.Error2(__METHOD__, " Could not create directory [", szDest, "].", NULL);
		}
		return RecurseCopyDir(szSource, szDest);
	}
	else
	{
		if (!Copy(szSource, szDest))
		{
			return gcLogger.Error2(__METHOD__, " Could not copy file [", szSource, "] to [", szDest, "].", NULL);
		}
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::SyncParentDirectory(const char* szFilename)
{
	CChars	szDirectory;
	bool	bResult;

	szDirectory.Init(szFilename);
	RemoveLastFromPath(&szDirectory);
	if (szDirectory.Empty())
	{
		szDirectory.Append(".");
	}
	bResult = SyncDirectory(szDirectory.Text());
	szDirectory.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Compare(const char* szFilename1, const char* szFilename2)
{
	return Compare(szFilename1, szFilename2, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Compare(const char* szFilename1, const char* szFilename2, bool bSizeOnly)
{
	CFileBasic	cPrimary;
	CFileBasic	cBackup;
	bool		bPrimary;
	bool		bBackup;
	filePos		iRemaining;
	size		iChunk;
	void*		pvPrimary;
	void*		pvBackup;
	bool		bResult;

	cPrimary.Init(DiskFile(szFilename1));
	bPrimary = cPrimary.Open(EFM_Read);
	cBackup.Init(DiskFile(szFilename2));
	bBackup = cBackup.Open(EFM_Read);

	//Two files that do not exist are the same.  One that exists and one that does not are different.
	if (!bPrimary || !bBackup)
	{
		bResult = (bPrimary == bBackup);
	}
	else
	{
		iRemaining = cPrimary.GetFileLength();
		bResult = (iRemaining == cBackup.GetFileLength());
		if (bResult && !bSizeOnly)
		{
			pvPrimary = malloc(FILE_BLOCK_SIZE);
			pvBackup = malloc(FILE_BLOCK_SIZE);
			while (bResult && (iRemaining > 0))
			{
				iChunk = (iRemaining > FILE_BLOCK_SIZE) ? FILE_BLOCK_SIZE : (size)iRemaining;
				bResult = cPrimary.ReadData(pvPrimary, iChunk) && cBackup.ReadData(pvBackup, iChunk);
				bResult = bResult && (memcmp(pvPrimary, pvBackup, iChunk) == 0);
				iRemaining -= iChunk;
			}
			SafeFree(pvPrimary);
			SafeFree(pvBackup);
		}
	}

	if (bPrimary)
	{
		cPrimary.Close();
	}
	if (bBackup)
	{
		cBackup.Close();
	}
	cPrimary.Kill();
	cBackup.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Copy(const char* szSource, const char* szDest)
{
	CFileBasic	cSource;
	CFileBasic	cDest;
	filePos		iRemaining;
	size		iChunk;
	void*		pvData;
	bool		bResult;

	cSource.Init(DiskFile(szSource));
	if (!cSource.Open(EFM_Read))
	{
		cSource.Kill();
		return false;
	}

	//Opening for write does not truncate so a longer destination would keep its old tail.
	Delete(szDest);
	cDest.Init(DiskFile(szDest));
	if (!cDest.Open(EFM_Write_Create))
	{
		cDest.Kill();
		cSource.Close();
		cSource.Kill();
		return false;
	}

	bResult = true;
	pvData = malloc(FILE_BLOCK_SIZE);
	iRemaining = cSource.GetFileLength();
	while (bResult && (iRemaining > 0))
	{
		iChunk = (iRemaining > FILE_BLOCK_SIZE) ? FILE_BLOCK_SIZE : (size)iRemaining;
		bResult = cSource.ReadData(pvData, iChunk) && cDest.WriteData(pvData, iChunk);
		iRemaining -= iChunk;
	}
	SafeFree(pvData);

	bResult &= cDest.Close();
	cDest.Kill();
	cSource.Close();
	cSource.Kill();

	if (!bResult)
	{
		//Do not leave a partial copy behind.
		Delete(szDest);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::FixSeparators(CChars* szPathName)
{
	//You really need to make this function less retarded.
	szPathName->Replace(OTHER_OS_FILE_SEPARATOR[0],  FILE_SEPARATOR[0]);
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::SplitPath(const char* szPathName, CChars* szDestFilename, CChars* szDestDirectory)
{
	size	iIndex;
	CChars	szTemp;

	//Does not append a trailing separator.
	if (szPathName)
	{
		szTemp.Init(szPathName);
		FixSeparators(&szTemp);
		iIndex = FindLastSeparator(szTemp.Text());
		if ((iIndex != 0) && (iIndex != ARRAY_ELEMENT_NOT_FOUND))
		{
			if (iIndex > 1)
			{
				szDestDirectory->Append(szTemp.Text(), iIndex);
				szDestFilename->AppendSubString(szTemp.Text(), iIndex+1, szTemp.Length());
			}
			else
			{
				szDestFilename->AppendSubString(szTemp.Text(), iIndex+1, szTemp.Length());
			}
		}
		else
		{
			szDestFilename->Append(szTemp.Text());
		}
		szTemp.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::SplitPath(const char* szPathName, CArrayChars* paszComponents)
{
	CChars	sz;
	bool	bAbsolute;

	sz.Init(szPathName);

	FixSeparators(&sz);

	sz.Split(paszComponents, FILE_SEPARATOR[0]);

	bAbsolute = IsAbsolutePath(sz.Text());
	if (bAbsolute)
	{
		if (paszComponents->NumElements() > 0)
		{
			paszComponents->Get(0)->Append(FILE_SEPARATOR[0]);
			if (paszComponents->NumElements() > 1)
			{
				paszComponents->Get(0)->Append(paszComponents->Get(1));
				paszComponents->Remove(1);
			}
		}
	}

	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::GetFileName(CChars* szDest, char* szPathName)
{
	size	iIndex;
	size	iLength;

	if (StrEmpty(szPathName))
	{
		return;
	}

	iLength = StrLen(szPathName);
	iIndex = FindLastSeparator(szPathName);
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		szDest->AppendSubString(&szPathName[iIndex + 1], &szPathName[iLength]);
	}
	else
	{
		szDest->Append(szPathName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::CollapsePath(CChars* szPathName)
{
	CChars			szTemp;
	CArrayChars		szNodes;
	int				i;
	CChars*			pszNode;
	CChars*			apCharDirectories[4096];
	int				iPos;
	int				iNumElements;
	bool			bLeadingSeparator;
	char			cDriveLetter;

	if (szPathName->Empty())
	{
		return;
	}

	cDriveLetter = GetDriveLetter(szPathName->Text());

	if (cDriveLetter != '\0')
	{
		szTemp.Init(szPathName->Text(), 2);
	}
	else
	{
		szTemp.Init(szPathName->Text());
	}

	szNodes.Init();
	szTemp.Split(&szNodes, FILE_SEPARATOR[0]);
	szTemp.Kill();

	bLeadingSeparator = false;
	if (szNodes.Get(0)->Empty())
	{
		bLeadingSeparator = true;
	}

	iNumElements = (int)szNodes.NumElements();
	iPos = 0;
	for (i = 0; i < iNumElements; i++)
	{
		pszNode = szNodes.Get(i);
		if (pszNode->Equals("."))
		{
		}
		else if (pszNode->Equals(".."))
		{
			iPos--;
		}
		else
		{
			if (iPos >= 0)
			{
				if (pszNode->Empty())
				{
					iPos--;
				}
				else
				{
					apCharDirectories[iPos] = pszNode;
				}
			}
			iPos++;
		}
	}

	szTemp.Init();
	if (cDriveLetter != '\0')
	{
		szTemp.Append(cDriveLetter);
		szTemp.Append(':');
	}

	if (iPos >= 0)
	{
		if (bLeadingSeparator)
		{
			szTemp.Append(FILE_SEPARATOR);
		}

		for (i = 0; i < iPos; i++)
		{
			szTemp.Append(apCharDirectories[i]);
			if (i != iPos - 1)
			{
				szTemp.Append(FILE_SEPARATOR);
			}
		}
	}

	szPathName->Set(szTemp.Text());
	szTemp.Kill();

	szNodes.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::RemoveLastSeparator(CChars* szPathName)
{
	if (szPathName->EndsWith(FILE_SEPARATOR))
	{
		szPathName->RemoveFromEnd(1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::AppendToPath(CChars* szPathName, const char* szItem)
{
	if (StrEmpty(szItem))
	{
		return;
	}
	
	if ((!szPathName->Empty()) && (!szPathName->EndsWith(FILE_SEPARATOR)))
	{
		szPathName->Append(FILE_SEPARATOR);
	}

	szPathName->Append(szItem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::PrependToPath(CChars* szPathName, const char* szItem)
{
	CChars			szTemp;
	bool			bLeadingSeparator;
	char			cDriveLetter;

	if (StrEmpty(szItem))
	{
		return;
	}

	cDriveLetter = GetDriveLetter(szPathName->Text());

	if (cDriveLetter != '\0')
	{
		szTemp.Init(szPathName->Text(), 2);
	}
	else
	{
		szTemp.Init(szPathName->Text());
	}

	bLeadingSeparator = false;
	if (szTemp.StartsWith(FILE_SEPARATOR))
	{
		szTemp.RemoveCharacter(0);
		bLeadingSeparator = true;
	}

	szPathName->Clear();
	if (cDriveLetter != '\0')
	{
		szPathName->Append(cDriveLetter);
		szPathName->Append(':');
	}

	if (bLeadingSeparator)
	{
		szPathName->Append(FILE_SEPARATOR);
	}
	
	szPathName->Append(szItem);
	szPathName->Append(FILE_SEPARATOR);
	szPathName->Append(&szTemp);
	
	szTemp.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::RemoveLastFromPath(CChars* szPathName)
{
	size	iLastIndex;
	size	iFirstIndex;

	iLastIndex = FindLastSeparator(szPathName->Text());
	if (IsAbsolutePath(szPathName->Text()))
	{
		iFirstIndex = FindFirstSeparator(szPathName->Text());
		if (iFirstIndex != iLastIndex)
		{
			szPathName->RemoveEnd(iLastIndex);
		}
		else
		{
			szPathName->RemoveEnd(iLastIndex + 1);
		}
	}
	else
	{
		if (iLastIndex != ARRAY_ELEMENT_NOT_FOUND)
		{
			szPathName->RemoveEnd(iLastIndex);
		}
		else
		{
			szPathName->Clear();
		}
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
void CFileUtil::RemoveExtension(CChars* szPathName)
{
	size	iIndex;

	iIndex = FindExtension(szPathName->Text());
	if (iIndex != ARRAY_ELEMENT_NOT_FOUND)
	{
		szPathName->RemoveEnd(iIndex);
	}
}



////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
void CFileUtil::RemovePath(CChars* szPathName)
{
	size	iIndex;

	if (szPathName == NULL)
	{
		return;
	}

	if (szPathName->Empty())
	{
		return;
	}

	iIndex = FindLastSeparator(szPathName->Text());
	if (iIndex == ARRAY_ELEMENT_NOT_FOUND)
	{
		return;
	}

	szPathName->RemoveFromStart(iIndex+1);
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
bool CFileUtil::IsExtension(const char* szFilename, const char* szExtension)
{
	size	iExtension;
	CChars	sz1;

	iExtension = FindExtension(szFilename);
	if (iExtension != ARRAY_ELEMENT_NOT_FOUND)
	{
		sz1.Fake((char*)&szFilename[iExtension+1]);
		return sz1.Equals(szExtension);
	}
	else
	{
		return StrEmpty(szExtension);
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
size CFileUtil::FindExtension(const char* szString)
{
	const char*	szExtension;
	const char*	szSeparator;

	szExtension = FindChar(szString, '.', true);
	if (szExtension)
	{
		szSeparator = FindChar(szString, FILE_SEPARATOR[0], true);
		if (szSeparator)
		{
			if (szSeparator > szExtension)
			{
				return ARRAY_ELEMENT_NOT_FOUND;
			}
		}
		return szExtension - szString;
	}
	else
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
size CFileUtil::FindLastSeparator(const char* szString)
{
	const char*	sz;

	sz = FindChar(szString, FILE_SEPARATOR[0], true);
	if (sz)
	{
		return sz - szString;
	}
	else
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
size CFileUtil::FindFirstSeparator(const char* szString)
{
	const char*	sz;

	sz = FindChar(szString, FILE_SEPARATOR[0], false);
	if (sz)
	{
		return sz - szString;
	}
	else
	{
		return ARRAY_ELEMENT_NOT_FOUND;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::RecurseFindFiles(const char* szDirectory, const char* szInName, const char* szExtension, CArrayChars* paszFiles, bool bHidden)
{
	CChars*			szDir;
	size			i;
	CArrayChars		aszDirs;
	bool			bDirectories;
	bool			bFiles;

	aszDirs.Init();
	bDirectories = FindFiles(szDirectory, true, NULL, NULL, &aszDirs, bHidden);

	for (i = 0; i < aszDirs.NumElements(); i++)
	{
		szDir = aszDirs.Get(i);
		if (!(szDir->EndsWith(".") || szDir->EndsWith("..")))
		{
			RecurseFindFiles(szDir->Text(), szInName, szExtension, paszFiles, bHidden);
		}
	}
	aszDirs.Kill();

	bFiles = FindFiles(szDirectory, false, szInName, szExtension, paszFiles, bHidden);
	return bFiles || bDirectories;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::FindFilesWithNameContaining(const char* szPathName, const char* szFilename, CArrayChars* paszFiles, bool bIncludeSubDirs, bool bHidden)
{
	if (bIncludeSubDirs)
	{
		RecurseFindFiles(szPathName, szFilename, NULL, paszFiles, bHidden);
	}
	else
	{
		FindFiles(szPathName, false, szFilename, NULL, paszFiles, bHidden);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::FindFilesWithExtension(const char* szPathName, const char* szExtension, CArrayChars* paszFiles, bool bIncludeSubDirs, bool bHidden)
{
	if (bIncludeSubDirs)
	{
		RecurseFindFiles(szPathName, NULL, szExtension, paszFiles, bHidden);
	}
	else
	{
		FindFiles(szPathName, false, NULL, szExtension, paszFiles, bHidden);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::FindAllDirectories(const char* szPathName, CArrayChars* paszFiles, bool bHidden)
{
	FindFiles(szPathName, true, NULL, NULL, paszFiles, bHidden);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::FindAllFiles(const char* szPathName, CArrayChars* paszFiles, bool bIncludeSubDirs, bool bHidden)
{
	if (bIncludeSubDirs)
	{
		return RecurseFindFiles(szPathName, NULL, NULL, paszFiles, bHidden);
	}
	else
	{
		return FindFiles(szPathName, false, NULL, NULL, paszFiles, bHidden);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::MakeNameFromDirectory(CChars* pszName, CChars* pszFilename, CChars* pszDirectory)
{
	size	iToRemove;
	CChars	szDirectory;

	pszName->Init(pszFilename->Text());
	FullPath(pszName);

	szDirectory.Init(pszDirectory->Text());
	FullPath(&szDirectory);

	iToRemove = szDirectory.Length();
	szDirectory.Kill();
	if (pszName->GetChar(iToRemove) == FILE_SEPARATOR[0])
	{
		iToRemove++;
	}
	pszName->RemoveFromStart(iToRemove);
	pszName->Replace(FILE_SEPARATOR[0], '/');
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::TouchDir(const char* szDirectory, bool bLastIsFilename)
{
	CChars			szPath;
	char			cDrive;
	CArrayChars		aszPathComponents;
	size			i;
	CChars			szPartialPath;
	bool			bResult;

	szPath.Init(szDirectory);

	//Most calls are for a directory that already exists.  One stat is much cheaper than collapsing, splitting and making every part of the path.
	if (bLastIsFilename)
	{
		RemoveLastFromPath(&szPath);
	}
	if (!szPath.Empty() && IsDirectory(szPath.Text()))
	{
		//Nothing was made, the same result as MakeDir on the existing last directory.
		szPath.Kill();
		return false;
	}
	szPath.Set(szDirectory);

	CollapsePath(&szPath);
	
	if (szPath.Empty())
	{
		szPath.Kill();
		return false;
	}

	if (bLastIsFilename)
	{
		RemoveLastFromPath(&szPath);
		if (szPath.Empty())
		{
			szPath.Kill();
			return false;
		}
	}

	if (IsAbsolutePath(szPath.Text()))
	{
		cDrive = GetDriveLetter(szPath.Text());
		if ((cDrive != 0) && (szPath.Length() == 3))
		{
			szPath.Kill();
			return false;
		}
		if (szPath.Length() == 1)
		{
			szPath.Kill();
			return false;
		}
	}

	aszPathComponents.Init();
	SplitPath(szPath.Text(), &aszPathComponents);
	szPath.Kill();
	szPartialPath.Init();
	bResult = false;
	for (i = 0; i < aszPathComponents.NumElements(); i++)
	{
		if (i != 0)
		{
			szPartialPath.Append(FILE_SEPARATOR);
		}
		szPartialPath.Append(aszPathComponents.Get(i));
		bResult = MakeDir(szPartialPath.Text());
	}
	aszPathComponents.Kill();
	szPartialPath.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a directory entry named szName is one FindFiles wants: directories (bDirectories) or files.  Its name
// without its extension must contain szInName (ignoring case) and a file's extension must be szExtension ("" for no
// extension), when they are not NULL.
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::IsFindMatch(const char* szName, bool bDirectory, bool bDirectories, const char* szInName, const char* szExtension)
{
	CChars	szTemp;
	CChars	szFake;
	int		iFileExtension;
	bool	bValid;

	if (bDirectory != bDirectories)
	{
		return false;
	}

	bValid = true;
	if (!bDirectory && (szExtension != NULL))
	{
		szFake.Fake(szName);
		iFileExtension = (int)FindExtension(szFake.Text());
		if (iFileExtension != -1)
		{
			bValid = szFake.SubStringEquals(iFileExtension + 1, szExtension);
		}
		else
		{
			//A file without an extension only matches no extension.
			bValid = szExtension[0] == 0;
		}
	}
	if (bValid && (szInName != NULL))
	{
		szTemp.Init(szName);
		RemoveExtension(&szTemp);
		bValid = szTemp.ContainsIgnoreCase(szInName);
		szTemp.Kill();
	}
	return bValid;
}


//////////////////////////////////////////////////////////////////////////
//
// The order NTFS lists directory entries in: names compared upper cased.
//
//////////////////////////////////////////////////////////////////////////
int CompareFileNamesNTFS(const char* szName1, const char* szName2)
{
	const unsigned char*	ps1;
	const unsigned char*	ps2;
	int						c1;
	int						c2;

	ps1 = (const unsigned char*)szName1;
	ps2 = (const unsigned char*)szName2;
	do
	{
		c1 = toupper(*ps1++);
		c2 = toupper(*ps2++);
	}
	while ((c1 == c2) && (c1 != 0));
	return c1 - c2;
}

