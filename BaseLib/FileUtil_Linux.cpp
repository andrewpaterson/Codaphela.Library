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
#ifdef LINUX_GNU_32
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include "FileUtil.h"
#include "StringHelper.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CFileUtil::CurrentDirectory(CChars* szDest)
{
	char	szTemp[MAX_PATH];

	getcwd(szTemp, MAX_PATH);
	szDest->Append(szTemp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::MakeDir(const char*szPathName)
{
    int iResult;

	iResult = mkdir(szPathName, S_IRWXU);
	if (iResult == 0)
	{
	    return TRUE;
	}
	else
	{
	    return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::RemoveDir(const char*szPathName)
{
    CChars  sz;
    BOOL    bResult;

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
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::RecurseRemoveDir(const char*szPathName)
{
    DIR*                pDIR;
    struct dirent*      pDirEnt;
	CChars              szTemp;
	CChars              szDirectory;
    struct stat         sBuffer;
    int                 iStatus;
    BOOL                bDir;

	szDirectory.Init(szPathName);
	RemoveLastSeparator(&szDirectory);

    pDIR = opendir(szPathName);

    if (pDIR == NULL)
    {
        return FALSE;
    }

    pDirEnt = readdir(pDIR);
    while (pDirEnt != NULL)
    {
        if (!((strcmp(pDirEnt->d_name, ".") == 0) || (strcmp(pDirEnt->d_name, "..") == 0)))
        {
            szTemp.Init(szDirectory);
            szTemp.Append(FILE_SEPARATOR);
            szTemp.Append(pDirEnt->d_name);

            iStatus = stat(szTemp.Text(), &sBuffer);
            if (iStatus == -1)
            {
                printf("%s: %s\n", strerror(errno), szTemp.Text());
                return FALSE;
            }
            bDir = S_ISDIR(sBuffer.st_mode);
            if (bDir) //Directory
            {
				RemoveDir(szTemp.Text());
			}
            else
            {
                unlink(szTemp.Text());
            }
            szTemp.Kill();
		}
		pDirEnt = readdir(pDIR);
	}

    closedir(pDIR);
	rmdir(szDirectory.Text());
	szDirectory.Kill();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::Touch(const char*szFileName)
{
    int     iHandle;

    iHandle = open(szFileName, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR);
	if (iHandle != -1)
	{
		close(iHandle);
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::Delete(const char*szFileName)
{
    int     iResult;

	iResult = unlink(szFileName);
	if (iResult == 0)
	{
	    return TRUE;
	}
	else
	{
	    return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::Exists(const char* szFileName)
{
    int     iHandle;

    iHandle = open(szFileName, O_RDONLY, S_IWUSR | S_IRUSR);
	if (iHandle != -1)
	{
		close(iHandle);
		return TRUE;
	}
	printf("%s: %s\n", strerror(errno), szFileName);
	return FALSE;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CFileUtil::Size(const char*szFileName)
{
    struct stat         sBuffer;
    int                 iStatus;
    CChars              sz;

    sz.Init(szFileName);
    FullPath(&sz);
    iStatus = stat(sz.Text(), &sBuffer);
    if (iStatus == -1)
    {
        return -1;
    }
    return sBuffer.st_size;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::IsAbsolutePath(const char*szPathName)
{
	const char*  pcPos;

	pcPos = FindChar(szPathName, FILE_SEPARATOR[0]);
	if (pcPos == szPathName)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char CFileUtil::GetDriveLetter(const char*szPathName)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::FindFiles(const char*szInDirectory, BOOL bDirs, const char*szInName, const char*szExtension, CArrayChars* paszFiles, BOOL bHidden)
{
	DIR*                pDIR;
	struct dirent*      pDirEnt;
	CChars				szFindName;
    CChars				szTemp;
	BOOL				bValid;
    CChars				szDirectory;
    CChars				szFile;
    CChars				szFake;
	int                 iStatus;
	struct stat         sBuffer;
	BOOL                bDirectory;
	CFileUtil           cFileUtil;
	int                 iFileExtension;

	szDirectory.Init(szInDirectory);
	FullPath(&szDirectory);

	pDIR = opendir(szDirectory.Text());

	if (pDIR == NULL)
	{
	    szDirectory.Kill();
		return TRUE;
	}

	pDirEnt = readdir(pDIR);
	while (pDirEnt != NULL)
	{
		bValid = TRUE;

		szTemp.Init(szDirectory);
		szTemp.Append(FILE_SEPARATOR);
        szTemp.Append(pDirEnt->d_name);

		iStatus = stat(szTemp.Text(), &sBuffer);
		if (iStatus == -1)
		{
			printf("%s: %s\n", strerror(errno), szTemp.Text());
			szTemp.Kill();
			szDirectory.Kill();
			return FALSE;
		}

        bDirectory = S_ISDIR(sBuffer.st_mode);
		if (bDirectory)
		{
            if (bDirs)
            {
                if (szInName != NULL)
                {
                    szTemp.Init(pDirEnt->d_name);
                    RemoveExtension(&szTemp);
                    if (!szTemp.Contains(szInName))
                    {
                        bValid = FALSE;
                    }
                    szTemp.Kill();
                }
            }
            else
            {
                bValid = FALSE;
			}
		}
		else
		{
            if (!bDirs)
            {
                if (szExtension != NULL)
                {
                    szFake.Fake(pDirEnt->d_name);
                    iFileExtension = FindExtension(szFake.Text());
                    if (iFileExtension != -1)
                    {
                        if (!(szFake.SubStringEquals(iFileExtension+1, szExtension)))
                        {
                            bValid = FALSE;
                        }
                    }
                    else
                    {
                        //If there is no file extension on the current file and the
                        //extension being looked for is not empty...
                        if (szExtension[0] != 0)
                        {
                            //Then this file is not valid.
                            bValid = FALSE;
                        }
                    }
                }
                if (szInName != NULL)
                {
                    szTemp.Init(pDirEnt->d_name);
                    RemoveExtension(&szTemp);
                    if (!szTemp.ContainsIgnoreCase(szInName))
                    {
                        bValid = FALSE;
                    }
                    szTemp.Kill();
                }
            }
            else
            {
                bValid = FALSE;
            }
		}

        if (bValid)
        {
            if (!((strcmp(pDirEnt->d_name, ".") == 0) || (strcmp(pDirEnt->d_name, "..") == 0)))
            {
                szTemp.Init(szDirectory);
                AppendToPath(&szTemp, pDirEnt->d_name);
                paszFiles->Add(szTemp.Text());
                szTemp.Kill();
            }
        }

		pDirEnt = readdir(pDIR);
	}

	closedir(pDIR);
	return TRUE;
}


#endif //LINUX_GNU_32

