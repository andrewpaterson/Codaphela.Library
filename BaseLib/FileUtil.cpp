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
#include "FileUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	int			iCount;

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
	int			iCount;
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
	int		iCount;
	int		iEnd;

	sz.Init(szPathName);
	FullPath(&sz);
	CollapsePath(&sz);
	iCount = sz.Count(FILE_SEPARATOR[0]);
	if (iCount == 1)
	{
		iEnd = sz.FindFromEnd(FILE_SEPARATOR[0]);
		if (iEnd == sz.Length()-1)
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
	filePos		iPrimary;
	filePos			iBackup;
	int			i;
	filePos		iNumBlocks;
	filePos		iPartialBlock;
	void*		pvPrimary;
	void*		pvBackup;
	bool		bResult;

	cPrimary.Init(DiskFile(szFilename1));
	bPrimary = cPrimary.Open(EFM_Read);

	cBackup.Init(DiskFile(szFilename2));
	bBackup = cBackup.Open(EFM_Read);

	if (!bBackup && !bPrimary)
	{
		cBackup.Kill();
		cPrimary.Kill();
		return true;
	}

	if (bBackup && !bPrimary)
	{
		cBackup.Close();
		cBackup.Kill();
		cPrimary.Kill();
		return false;
	}

	if (!bBackup && bPrimary)
	{
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return false;
	}

	iPrimary = cPrimary.GetFileLength();
	iBackup = cBackup.GetFileLength();

	if (iPrimary != iBackup)
	{
		cBackup.Close();
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return false;
	}

	if (bSizeOnly)
	{
		cBackup.Close();
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return true;
	}

	iNumBlocks = iPrimary / FILE_BLOCK_SIZE;
	iPartialBlock = iPrimary % FILE_BLOCK_SIZE;

	pvPrimary = malloc(FILE_BLOCK_SIZE);
	pvBackup = malloc(FILE_BLOCK_SIZE);

	for (i = 0; i < iNumBlocks; i++)
	{
		cPrimary.ReadData(pvPrimary, FILE_BLOCK_SIZE);
		cBackup.ReadData(pvBackup, FILE_BLOCK_SIZE);

		bResult = memcmp(pvPrimary, pvBackup, FILE_BLOCK_SIZE);
		if (bResult != 0)
		{
			SafeFree(pvPrimary);
			SafeFree(pvBackup);
			cBackup.Close();
			cBackup.Kill();
			cPrimary.Close();
			cPrimary.Kill();
			return false;
		}
	}

	cPrimary.ReadData(pvPrimary, (int)iPartialBlock);
	cBackup.ReadData(pvBackup, (int)iPartialBlock);

	bResult = memcmp(pvPrimary, pvBackup, (int)iPartialBlock);
	if (bResult != 0)
	{
		SafeFree(pvPrimary);
		SafeFree(pvBackup);
		cBackup.Close();
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return false;
	}

	SafeFree(pvPrimary);
	SafeFree(pvBackup);
	cBackup.Close();
	cBackup.Kill();
	cPrimary.Close();
	cPrimary.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Copy(const char* szSource, const char* szDest)
{
	CFileBasic	cPrimary;
	CFileBasic	cBackup;
	bool		bPrimary;
	bool		bBackup;
	filePos		iPrimary;
	int			i;
	filePos		iNumBlocks;
	int			iPartialBlock;
	void*		pvData;

	cPrimary.Init(DiskFile(szSource));
	bPrimary = cPrimary.Open(EFM_Read);

	if (!bPrimary)
	{
		Delete(szDest);
		cPrimary.Kill();
		return true;
	}

	cBackup.Init(DiskFile(szDest));
	bBackup = cBackup.Open(EFM_Write_Create);
	if (!bBackup)
	{
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return false;
	}

	iPrimary = cPrimary.GetFileLength();

	iNumBlocks = iPrimary / FILE_BLOCK_SIZE;
	iPartialBlock = (int)(iPrimary % FILE_BLOCK_SIZE);

	pvData = malloc(FILE_BLOCK_SIZE);

	for (i = 0; i < iNumBlocks; i++)
	{
		cPrimary.ReadData(pvData, FILE_BLOCK_SIZE);
		if (!cBackup.WriteData(pvData, FILE_BLOCK_SIZE))
		{
			SafeFree(pvData);
			cBackup.Close();
			cBackup.Kill();
			cPrimary.Close();
			cPrimary.Kill();
			return false;
		}
	}

	cPrimary.ReadData(pvData, iPartialBlock);
	if (!cBackup.WriteData(pvData, iPartialBlock))
	{
		SafeFree(pvData);
		cBackup.Close();
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return false;
	}

	SafeFree(pvData);
	cBackup.Close();
	cBackup.Kill();
	cPrimary.Close();
	cPrimary.Kill();
	return true;
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
	int		iIndex;
	CChars	szTemp;

	//Does not append a trailing separator.
	if (szPathName)
	{
		szTemp.Init(szPathName);
		FixSeparators(&szTemp);
		iIndex = FindLastSeparator(szTemp.Text());
		if (iIndex > 0)
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
void CFileUtil::CollapsePath(CChars* szPathName)
{
	CChars			szTemp;
	CArrayChars		szNodes;
	int				i;
	CChars*			pszNode;
	CChars*			apCharDirectories[4096];
	int				iPos;
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

	iPos = 0;
	for (i = 0; i < szNodes.NumElements(); i++)
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
			if (i != iPos-1)
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
	int		iLastIndex;
	int		iFirstIndex;

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
		if (iLastIndex != -1)
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
	int		iIndex;

	iIndex = FindExtension(szPathName->Text());
	if (iIndex != -1)
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
	int		iIndex;

	if (szPathName == NULL)
	{
		return;
	}

	if (szPathName->Empty())
	{
		return;
	}

	iIndex = FindLastSeparator(szPathName->Text());
	if (iIndex == -1)
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
	int		iExtension;
	CChars	sz1;

	iExtension = FindExtension(szFilename);
	if (iExtension != -1)
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
int CFileUtil::FindExtension(const char* szString)
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
				return -1;
			}
		}
		return ((int) (size_t) (szExtension - szString));
	}
	else
	{
		return -1;
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
int CFileUtil::FindLastSeparator(const char* szString)
{
	const char*	sz;

	sz = FindChar(szString, FILE_SEPARATOR[0], true);
	if (sz)
	{
		return (int) (size_t) (sz - szString);
	}
	else
	{
		return -1;
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
int CFileUtil::FindFirstSeparator(const char* szString)
{
	const char*	sz;

	sz = FindChar(szString, FILE_SEPARATOR[0], false);
	if (sz)
	{
		return (int) (size_t) (sz - szString);
	}
	else
	{
		return -1;
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::RecurseFindFiles(const char* szDirectory, const char* szInName, const char* szExtension, CArrayChars* paszFiles, bool bHidden)
{
	CChars*			szDir;
	int				i;
	CArrayChars	aszDirs;
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
	int		iToRemove;
	CChars	szDirectory;

	pszName->Init(pszFilename->Text());

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
	int				i;
	CChars			szPartialPath;
	bool			bResult;

	szPath.Init(szDirectory);
	CollapsePath(&szPath);
	
	if (szPath.Empty())
	{
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

