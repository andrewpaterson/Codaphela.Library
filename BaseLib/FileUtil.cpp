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
#include "FileUtil.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Define.h"
#include "StringHelper.h"
#include "FileBasic.h"
#include "ArrayString.h"
#include "DiskFile.h"


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
			RemoveFileSeparator(szPathName);
		}
	}
	else
	{
		CurrentDirectory(szPathName);
		RemoveFileSeparator(szPathName);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::Compare(char* szFileName1, char* szFileName2)
{
	CFileBasic	cPrimary;
	CFileBasic	cBackup;
	BOOL		bPrimary;
	BOOL		bBackup;
	filePos		iPrimary;
	filePos			iBackup;
	int			i;
	filePos		iNumBlocks;
	filePos		iPartialBlock;
	void*		pvPrimary;
	void*		pvBackup;
	BOOL		bResult;

	cPrimary.Init(DiskFile(szFileName1));
	bPrimary = cPrimary.Open(EFM_Read);

	cBackup.Init(DiskFile(szFileName2));
	bBackup = cBackup.Open(EFM_Read);

	if (!bBackup && !bPrimary)
	{
		cBackup.Kill();
		cPrimary.Kill();
		return TRUE;
	}

	if (bBackup && !bPrimary)
	{
		cBackup.Close();
		cBackup.Kill();
		cPrimary.Kill();
		return FALSE;
	}

	if (!bBackup && bPrimary)
	{
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return FALSE;
	}

	iPrimary = cPrimary.GetFileLength();
	iBackup = cBackup.GetFileLength();

	if (iPrimary != iBackup)
	{
		cBackup.Close();
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return FALSE;
	}

	iNumBlocks = iPrimary / FILE_BLOCK_SIZE;
	iPartialBlock = iPrimary % FILE_BLOCK_SIZE;

	pvPrimary = malloc(FILE_BLOCK_SIZE);
	pvBackup = malloc(FILE_BLOCK_SIZE);

	for (i = 0; i < iNumBlocks; i++)
	{
		cPrimary.Read(pvPrimary, FILE_BLOCK_SIZE, 1);
		cBackup.Read(pvBackup, FILE_BLOCK_SIZE, 1);

		bResult = memcmp(pvPrimary, pvBackup, FILE_BLOCK_SIZE);
		if (bResult != 0)
		{
			free(pvPrimary);
			free(pvBackup);
			cBackup.Close();
			cBackup.Kill();
			cPrimary.Close();
			cPrimary.Kill();
			return FALSE;
		}
	}

	cPrimary.Read(pvPrimary, (int)iPartialBlock, 1);
	cBackup.Read(pvBackup, (int)iPartialBlock, 1);

	bResult = memcmp(pvPrimary, pvBackup, (int)iPartialBlock);
	if (bResult != 0)
	{
		free(pvPrimary);
		free(pvBackup);
		cBackup.Close();
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return FALSE;
	}

	free(pvPrimary);
	free(pvBackup);
	cBackup.Close();
	cBackup.Kill();
	cPrimary.Close();
	cPrimary.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::Copy(char* szSource, char* szDest)
{
	CFileBasic	cPrimary;
	CFileBasic	cBackup;
	BOOL		bPrimary;
	BOOL		bBackup;
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
		return TRUE;
	}

	cBackup.Init(DiskFile(szDest));
	bBackup = cBackup.Open(EFM_Write_Create);
	if (!bBackup)
	{
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return FALSE;
	}

	iPrimary = cPrimary.GetFileLength();

	iNumBlocks = iPrimary / FILE_BLOCK_SIZE;
	iPartialBlock = (int)(iPrimary % FILE_BLOCK_SIZE);

	pvData = malloc(FILE_BLOCK_SIZE);

	for (i = 0; i < iNumBlocks; i++)
	{
		cPrimary.Read(pvData, FILE_BLOCK_SIZE, 1);
		if (!cBackup.Write(pvData, FILE_BLOCK_SIZE, 1))
		{
			free(pvData);
			cBackup.Close();
			cBackup.Kill();
			cPrimary.Close();
			cPrimary.Kill();
			return FALSE;
		}
	}

	cPrimary.Read(pvData, iPartialBlock, 1);
	if (!cBackup.Write(pvData, iPartialBlock, 1))
	{
		free(pvData);
		cBackup.Close();
		cBackup.Kill();
		cPrimary.Close();
		cPrimary.Kill();
		return FALSE;
	}

	free(pvData);
	cBackup.Close();
	cBackup.Kill();
	cPrimary.Close();
	cPrimary.Kill();
	return TRUE;
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
void CFileUtil::SplitPath(char* szPathName, CChars* szDestFileName, CChars* szDestDirectory)
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
				szDestFileName->AppendSubString(szTemp.Text(), iIndex+1, szTemp.Length());
			}
			else
			{
				szDestFileName->AppendSubString(szTemp.Text(), iIndex+1, szTemp.Length());
			}
		}
		else
		{
			szDestFileName->Append(szTemp.Text());
		}
		szTemp.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::SplitPath(char* szPathName, CArrayString* paszComponents)
{
	CChars	sz;
	BOOL	bAbsolute;

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
	CArrayString	szNodes;
	int				i;
	CChars*			pszNode;
	CChars*			apCharDirectories[4096];
	int				iPos;
	BOOL			bLeadingSeparator;
	char			cDriveLetter;

	cDriveLetter = GetDriveLetter(szPathName->Text());

	if (cDriveLetter != '\0')
	{
		szTemp.Init(szPathName->Text(), 2);
	}
	else
	{
		szTemp.Init(szPathName->Text());
	}

	szNodes.Init(1);
	szTemp.Split(&szNodes, FILE_SEPARATOR[0]);
	szTemp.Kill();

	bLeadingSeparator = FALSE;
	if (szNodes.Get(0)->Empty())
	{
		bLeadingSeparator = TRUE;
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
void CFileUtil::RemoveFileSeparator(CChars* szPathName)
{
	if (szPathName->EndsWith(FILE_SEPARATOR))
	{
		//Assumes file separator is one char long.
		szPathName->RemoveFromEnd(1);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::AppendToPath(CChars* szPathName, char* szItem)
{
	if (!szPathName->EndsWith(FILE_SEPARATOR))
	{
		szPathName->Append(FILE_SEPARATOR);
	}
	
	szPathName->Append(szItem);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::RemoveLastFromPath(CChars* szPathName)
{
	int		iIndex;

	iIndex = FindLastSeparator(szPathName->Text());
	if (iIndex != -1)
	{
		szPathName->RemoveEnd(iIndex);
	}
	else
	{
		szPathName->Clear();
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
BOOL CFileUtil::IsExtension(char* szFileName, char* szExtension)
{
	int		iExtension;
	CChars	sz1;

	iExtension = FindExtension(szFileName);
	if (iExtension != -1)
	{
		sz1.Fake(&szFileName[iExtension+1]);
		return sz1.Equals(szExtension);
	}
	else
	{
		return FALSE;
	}
}


////////////////////////////////////////////////////////////////////////////////////
//
//
////////////////////////////////////////////////////////////////////////////////////
int CFileUtil::FindExtension(char* szString)
{
	char*	szExtension;
	char*	szSeparator;

	szExtension = FindChar(szString, '.', TRUE);
	if (szExtension)
	{
		szSeparator = FindChar(szString, FILE_SEPARATOR[0], TRUE);
		if (szSeparator)
		{
			if (szSeparator > szExtension)
			{
				return -1;
			}
		}
		return ((int) (ENGINE_SIZE_T) (szExtension - szString));
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
int CFileUtil::FindLastSeparator(char* szString)
{
	char*	sz;

	sz = FindChar(szString, FILE_SEPARATOR[0], TRUE);
	if (sz)
	{
		return (int) (ENGINE_SIZE_T) (sz - szString);
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
BOOL CFileUtil::RecurseFindFiles(char* szDirectory, char* szInName, char* szExtension, CArrayString* paszFiles, BOOL bHidden)
{
	CChars*			szDir;
	int				i;
	CArrayString	aszDirs;
	BOOL			bDirectories;
	BOOL			bFiles;

	aszDirs.Init(8);
	bDirectories = FindFiles(szDirectory, TRUE, NULL, NULL, &aszDirs, bHidden);

	for (i = 0; i < aszDirs.NumElements(); i++)
	{
		szDir = aszDirs.Get(i);
		if (!(szDir->EndsWith(".") || szDir->EndsWith("..")))
		{
			RecurseFindFiles(szDir->Text(), szInName, szExtension, paszFiles, bHidden);
		}
	}
	aszDirs.Kill();

	bFiles = FindFiles(szDirectory, FALSE, szInName, szExtension, paszFiles, bHidden);
	return bFiles || bDirectories;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::FindFilesWithNameContaining(char* szPathName, char* szFileName, CArrayString* paszFiles, BOOL bIncludeSubDirs, BOOL bHidden)
{
	if (bIncludeSubDirs)
	{
		RecurseFindFiles(szPathName, szFileName, NULL, paszFiles, bHidden);
	}
	else
	{
		FindFiles(szPathName, FALSE, szFileName, NULL, paszFiles, bHidden);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::FindFilesWithExtension(char* szPathName, char* szExtension, CArrayString* paszFiles, BOOL bIncludeSubDirs, BOOL bHidden)
{
	if (bIncludeSubDirs)
	{
		RecurseFindFiles(szPathName, NULL, szExtension, paszFiles, bHidden);
	}
	else
	{
		FindFiles(szPathName, FALSE, NULL, szExtension, paszFiles, bHidden);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::FindAllDirectories(char* szPathName, CArrayString* paszFiles, BOOL bHidden)
{
	FindFiles(szPathName, TRUE, NULL, NULL, paszFiles, bHidden);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::FindAllFiles(char* szPathName, CArrayString* paszFiles, BOOL bIncludeSubDirs, BOOL bHidden)
{
	if (bIncludeSubDirs)
	{
		return RecurseFindFiles(szPathName, NULL, NULL, paszFiles, bHidden);
	}
	else
	{
		return FindFiles(szPathName, FALSE, NULL, NULL, paszFiles, bHidden);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::MakeNameFromDirectory(CChars* pszName, CChars* pszFileName, CChars* pszDirectory)
{
	int		iToRemove;
	CChars	szDirectory;

	pszName->Init(pszFileName->Text());

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

