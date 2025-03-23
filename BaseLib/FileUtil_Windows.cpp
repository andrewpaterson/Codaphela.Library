/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#ifdef _WIN32
#include <stdio.h>
#include "WindowsHeaders.h"
#include "Logger.h"
#include "FilePosition.h"
#include "FileUtil.h"
#include "StringHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::MakeDir(const char* szPathName)
{
	return CreateDirectory(szPathName, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::RemoveDir(const char* szPathName)
{
	WIN32_FIND_DATA		sFindData;
	CChars				szFindName;
	CChars				szTemp;
	HANDLE				hFindHandle;
	bool				bContinue;
	bool				bValid;
	CChars				szDirectory;
	bool				bDeleted;

	if (IsRootDirectory(szPathName))
	{
		return gcLogger.Error2(__METHOD__, " Aborting RemoveDir.  Tried to delete root directory.", NULL);
	}

	szDirectory.Init(szPathName);
	RemoveLastSeparator(&szDirectory);

	szFindName.Init(szPathName);
	AppendToPath(&szFindName, "*.*");

	hFindHandle = FindFirstFile(szFindName.Text(), &sFindData);
	bContinue = (hFindHandle != INVALID_HANDLE_VALUE);
	bDeleted = true;
	while (bContinue)
	{
		bValid = true;
		if (sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!((StringCompare(sFindData.cFileName, ".") == 0) || (StringCompare(sFindData.cFileName, "..") == 0)))
			{
				szTemp.Init(szDirectory);
				AppendToPath(&szTemp, sFindData.cFileName);
				RemoveDir(szTemp.Text());
				szTemp.Kill();
			}
		}
		else
		{
			szTemp.Init(szPathName);
			AppendToPath(&szTemp, sFindData.cFileName);
			bDeleted &= (bool)DeleteFile(szTemp.Text());
			szTemp.Kill();
		}
		bContinue = FindNextFile(hFindHandle, &sFindData);
	}
	FindClose(hFindHandle);
	RemoveDirectory(szDirectory.Text());

	szDirectory.Kill();
	szFindName.Kill();
	return bDeleted;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::CopyDir(const char* szSource, const char* szDest)
{
	WIN32_FIND_DATA		sFindData;
	CChars				szFindName;
	CChars				szSourceTemp;
	CChars				szDestTemp;
	HANDLE				hFindHandle;
	bool				bContinue;
	bool				bValid;
	CChars				szSourceDirectory;
	CChars				szDestDirectory;

	szSourceDirectory.Init(szSource);
	RemoveLastSeparator(&szSourceDirectory);
	szDestDirectory.Init(szDest);
	RemoveLastSeparator(&szDestDirectory);

	szFindName.Init(szSource);
	AppendToPath(&szFindName, "*.*");

	hFindHandle = FindFirstFile(szFindName.Text(), &sFindData);
	bContinue = (hFindHandle != INVALID_HANDLE_VALUE);
	while (bContinue)
	{
		bValid = true;
		if (sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			if (!((StringCompare(sFindData.cFileName, ".") == 0) || (StringCompare(sFindData.cFileName, "..") == 0)))
			{
				szSourceTemp.Init(szSourceDirectory);
				szDestTemp.Init(szDestDirectory);

				AppendToPath(&szSourceTemp, sFindData.cFileName);
				AppendToPath(&szDestTemp, sFindData.cFileName);
				
				MakeDir(szDestTemp.Text());
				CopyDir(szSourceTemp.Text(), szDestTemp.Text());

				szSourceTemp.Kill();
				szDestTemp.Kill();
			}
		}
		else
		{
			szSourceTemp.Init(szSourceDirectory);
			szDestTemp.Init(szDestDirectory);

			AppendToPath(&szSourceTemp, sFindData.cFileName);
			AppendToPath(&szDestTemp, sFindData.cFileName);

			Copy(szSourceTemp.Text(), szDestTemp.Text());
			
			szSourceTemp.Kill();
			szDestTemp.Kill();
		}
		bContinue = FindNextFile(hFindHandle, &sFindData);
	}
	FindClose(hFindHandle);

	szSourceDirectory.Kill();
	szDestDirectory.Kill();
	szFindName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Touch(const char* szFilename)
{
	HANDLE	h;

	h = CreateFile(szFilename, STANDARD_RIGHTS_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE)
	{
		CloseHandle(h);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Delete(const char* szFilename)
{
	if (Exists(szFilename))
	{
		return DeleteFile(szFilename);
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
bool CFileUtil::Exists(const char* szFilename)
{
	return PathFileExists(szFilename);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CFileUtil::Size(const char*szFilename)
{
	HANDLE		h;
	filePos		iSize;

	h = CreateFile(szFilename, STANDARD_RIGHTS_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h != INVALID_HANDLE_VALUE)
	{
		GetFileSizeEx(h, (PLARGE_INTEGER)&iSize);
		CloseHandle(h);
		return (int)iSize;
	}

	return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::IsAbsolutePath(const char* szPathName)
{
	const char*	szPos;
	char		cDrive;

	szPos = FindChar(szPathName, FILE_SEPARATOR[0]);
	if (szPos == szPathName)
	{
		return true;
	}
	else
	{
		cDrive = GetDriveLetter(szPathName);
		if (cDrive != '\0')
		{
			return true;
		}
		else
		{
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CFileUtil::GetDriveLetter(const char* szPathName)
{
	int		i;
	char	c;
	char	cDrive;

	cDrive = 0;
	for (i = 0;; i++)
	{
		c = szPathName[i];
		if (c == '\0')
		{
			return 0;
		}

		if (i == 1)
		{
			if (c == ':')
			{
				return cDrive;
			}
			else
			{
				return 0;
			}
		}

		cDrive = c;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::CurrentDirectory(CChars* szDest)
{
	char	szTemp[MAX_PATH];

	//Does not append a trailing separator.
	GetCurrentDirectory(MAX_PATH, szTemp);
	szDest->Append(szTemp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::FindFiles(const char*szInDirectory, bool bDirs, const char*szInName, const char*szExtension, CArrayChars* paszFiles, bool bHidden)
{
	WIN32_FIND_DATA		sFindData;
	CChars				szFindName;
	CChars				szTemp;
	HANDLE				hFindHandle;
	bool				bContinue;
	int					iFileExtension;
	bool				bValid;
	CChars				szFake;
	CChars				szDirectory;

	szDirectory.Init(szInDirectory);
	FullPath(&szDirectory);
	szFindName.Init(szDirectory);
	AppendToPath(&szFindName, "*.*");

	hFindHandle = FindFirstFile(szFindName.Text(), &sFindData);
	bContinue = (hFindHandle != INVALID_HANDLE_VALUE);

	if (!bContinue)
	{
		szDirectory.Kill();
		szFindName.Kill();
		return false;
	}

	while (bContinue)
	{
		if (!(sFindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) || bHidden)
		{
			bValid = true;
			if (sFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (bDirs)
				{
					if (szInName != NULL)
					{
						szTemp.Init(sFindData.cFileName);
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
						szFake.Fake(sFindData.cFileName);
						iFileExtension = FindExtension(szFake.Text());
						if (iFileExtension != -1)
						{
							if (!(szFake.SubStringEquals(iFileExtension+1, szExtension)))
							{
								bValid = false;
							}
						}
						else
						{
							//If there is no file extension on the current file and the
							//extension being looked for is not empty...
							if (szExtension[0] != 0)
							{
								//Then this file is not valid.
								bValid = false;
							}
						}
					}
					if (szInName != NULL)
					{
						szTemp.Init(sFindData.cFileName);
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
		}
		else
		{
			bValid = false;
		}

		if (bValid)
		{
			if (!((StringCompare(sFindData.cFileName, ".") == 0) || (StringCompare(sFindData.cFileName, "..") == 0)))
			{
				szTemp.Init(szDirectory);
				AppendToPath(&szTemp, sFindData.cFileName);
				paszFiles->Add(szTemp.Text());
				szTemp.Kill();
			}
		}

		bContinue = FindNextFile(hFindHandle, &sFindData);
	}
	FindClose(hFindHandle);

	szDirectory.Kill();
	szFindName.Kill();
	return true;
}


#endif //_WIN32

