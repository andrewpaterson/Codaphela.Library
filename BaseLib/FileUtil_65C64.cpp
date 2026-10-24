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
#ifdef __65C64__
#include <string.h>
#include <host.h>
#include "Logger.h"
#include "FileUtil.h"
#include "StringHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::CurrentDirectory(CChars* szDest)
{
	char	szTemp[MAX_PATH];

	if (HostCurrentDirectory(szTemp, MAX_PATH) > 0)
	{
		szDest->Append(szTemp);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::MakeDir(const char* szPathName)
{
	return HostMakeDir(szPathName) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::RemoveDir(const char* szPathName)
{
	CChars	sz;
	bool	bResult;

	if (IsRootDirectory(szPathName))
	{
		return gcLogger.Error2(__METHOD__, " Aborting RemoveDir.  Tried to delete root directory.", NULL);
	}
	sz.Init(szPathName);
	FullPath(&sz);
	bResult = RecurseRemoveDir(sz.Text());
	sz.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The host removes the directory and everything in it.  A directory that does not exist is already removed.
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::RecurseRemoveDir(const char* szPathName)
{
	return HostRemoveDir(szPathName) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::RecurseCopyDir(const char* szSource, const char* szDest)
{
	CChars			szSourceTemp;
	CChars			szDestTemp;
	CChars			szSourceDirectory;
	CChars			szDestDirectory;
	char			szName[MAX_PATH];
	unsigned int	uiIndex;
	int				iKind;
	bool			bResult;

	szSourceDirectory.Init(szSource);
	RemoveLastSeparator(&szSourceDirectory);
	szDestDirectory.Init(szDest);
	RemoveLastSeparator(&szDestDirectory);

	if (!HostIsDirectory(szSourceDirectory.Text()))
	{
		bResult = gcLogger.Error2(__METHOD__, " Could not read directory [", szSourceDirectory.Text(), "].", NULL);
		szSourceDirectory.Kill();
		szDestDirectory.Kill();
		return bResult;
	}

	bResult = true;
	for (uiIndex = 0; bResult; uiIndex++)
	{
		iKind = HostFindFile(szSourceDirectory.Text(), uiIndex, szName, MAX_PATH);
		if (iKind == HOST_FIND_NONE)
		{
			break;
		}
		szSourceTemp.Init(szSourceDirectory);
		szDestTemp.Init(szDestDirectory);
		AppendToPath(&szSourceTemp, szName);
		AppendToPath(&szDestTemp, szName);
		bResult = CopyDirEntry(szSourceTemp.Text(), szDestTemp.Text(), iKind == HOST_FIND_DIRECTORY);
		szSourceTemp.Kill();
		szDestTemp.Kill();
	}
	szSourceDirectory.Kill();
	szDestDirectory.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::IsDirectory(const char* szPathName)
{
	return HostIsDirectory(szPathName) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Creates the file, or empties it if it exists, like Windows CREATE_ALWAYS.
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Touch(const char* szFilename)
{
	return HostTouch(szFilename) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::SyncFile(const char* szFilename)
{
	return HostExists(szFilename) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::SyncDirectory(const char* szDirectory)
{
	return HostIsDirectory(szDirectory) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A file that does not exist is already deleted.
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Delete(const char* szFilename)
{
	return HostDelete(szFilename) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Exists(const char* szFilename)
{
	return HostExists(szFilename) != 0;
}


//////////////////////////////////////////////////////////////////////////
//
// -1 for a directory or a file that does not exist.
//
//////////////////////////////////////////////////////////////////////////
filePos CFileUtil::Size(const char* szFilename)
{
	CChars	sz;
	filePos	iSize;

	sz.Init(szFilename);
	FullPath(&sz);
	iSize = HostFileSize(sz.Text());
	sz.Kill();
	return iSize;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::IsAbsolutePath(const char* szPathName)
{
	return szPathName[0] == FILE_SEPARATOR[0];
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CFileUtil::GetDriveLetter(const char* szPathName)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The host gives the entries in the order NTFS returns them, without "." and "..".
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::FindFiles(const char* szInDirectory, bool bDirs, const char* szInName, const char* szExtension, CArrayChars* paszFiles, bool bHidden)
{
	CChars			szTemp;
	CChars			szDirectory;
	CChars			szFake;
	char			szName[MAX_PATH];
	unsigned int	uiIndex;
	int				iKind;
	int				iFileExtension;
	bool			bValid;

	szDirectory.Init(szInDirectory);
	FullPath(&szDirectory);
	if (!HostIsDirectory(szDirectory.Text()))
	{
		szDirectory.Kill();
		return false;
	}

	for (uiIndex = 0;; uiIndex++)
	{
		iKind = HostFindFile(szDirectory.Text(), uiIndex, szName, MAX_PATH);
		if (iKind == HOST_FIND_NONE)
		{
			break;
		}

		//Dot files are hidden.
		if ((szName[0] == '.') && !bHidden)
		{
			continue;
		}

		bValid = true;
		if (iKind == HOST_FIND_DIRECTORY)
		{
			if (bDirs)
			{
				if (szInName != NULL)
				{
					szTemp.Init(szName);
					RemoveExtension(&szTemp);
					if (!szTemp.ContainsIgnoreCase(szInName))
					{
						bValid = false;
					}
					szTemp.Kill();
				}
			}
			else
			{
				bValid = false;
			}
		}
		else
		{
			if (!bDirs)
			{
				if (szExtension != NULL)
				{
					szFake.Fake(szName);
					iFileExtension = (int)FindExtension(szFake.Text());
					if (iFileExtension != -1)
					{
						if (!(szFake.SubStringEquals(iFileExtension + 1, szExtension)))
						{
							bValid = false;
						}
					}
					else if (szExtension[0] != 0)
					{
						bValid = false;
					}
				}
				if (szInName != NULL)
				{
					szTemp.Init(szName);
					RemoveExtension(&szTemp);
					if (!szTemp.ContainsIgnoreCase(szInName))
					{
						bValid = false;
					}
					szTemp.Kill();
				}
			}
			else
			{
				bValid = false;
			}
		}

		if (bValid)
		{
			szTemp.Init(szDirectory);
			AppendToPath(&szTemp, szName);
			paszFiles->Add(szTemp.Text());
			szTemp.Kill();
		}
	}
	szDirectory.Kill();
	return true;
}


#endif // __65C64__

