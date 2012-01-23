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
char* CFileUtil::CurrentDirectory(CChars* szDest)
{
	char	szTemp[MAX_PATH];

	getcwd(szTemp, MAX_PATH);
	szDest->Append(szTemp);
	return szDest->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::MakeDir(char* szPathName)
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
BOOL CFileUtil::RemoveDir(char* szPathName)
{
    char    sz[MAX_PATH];

    if (FullPath(sz, szPathName, MAX_PATH))
    {
        return RecurseRemoveDir(sz);
    }
    return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::RecurseRemoveDir(char* szPathName)
{
    DIR*                pDIR;
    struct dirent*      pDirEnt;
	char				szTemp[MAX_PATH];
	BOOL				bValid;
	char				szDirectory[MAX_PATH];
    struct stat         sBuffer;
    int                 iStatus;
    BOOL                bDir;

	strcpy(szDirectory, szPathName);
	RemoveFileSeparator(szDirectory);

    pDIR = opendir(szPathName);

    if (pDIR == NULL)
    {
        return FALSE;
    }

    pDirEnt = readdir(pDIR);
    while (pDirEnt != NULL)
    {
		bValid = TRUE;

        if (!((strcmp(pDirEnt->d_name, ".") == 0) || (strcmp(pDirEnt->d_name, "..") == 0)))
        {
            strcpy(szTemp, szDirectory);
            strcat(szTemp, FILE_SEPARATOR);
            strcat(szTemp, pDirEnt->d_name);

            iStatus = stat(szTemp, &sBuffer);
            if (iStatus == -1)
            {
                printf("%s: %s\n", strerror(errno), szTemp);
                return FALSE;
            }
            bDir = S_ISDIR(sBuffer.st_mode);
            if (bDir) //Directory
            {
				RemoveDir(szTemp);
			}
            else
            {
                unlink(szTemp);
            }
		}
		pDirEnt = readdir(pDIR);
	}

    closedir(pDIR);
	rmdir(szDirectory);
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::Touch(char* szFileName)
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
BOOL CFileUtil::Delete(char* szFileName)
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
BOOL CFileUtil::Exists(char* szFileName)
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
int CFileUtil::Size(char* szFileName)
{
    struct stat         sBuffer;
    int                 iStatus;
    char                sz[MAX_PATH];

    if (FullPath(sz, szFileName, MAX_PATH))
    {
        iStatus = stat(sz, &sBuffer);
        if (iStatus == -1)
        {
            return -1;
        }
        return sBuffer.st_size;
    }
    return -1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CFileUtil::IsAbsolutePath(char* szPathName)
{
	char*   pcPos;

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
char CFileUtil::GetDriveLetter(char* szPathName)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
TRISTATE CFileFinder::FindFiles(char* szInDirectory, BOOL bDirs, char* szInName, char* szExtension)
{
	DIR*                pDIR;
	struct dirent*      pDirEnt;
	char				szFindName[MAX_PATH];
	char				szTemp[MAX_PATH];
	BOOL				bValid;
	char				szDirectory[MAX_PATH];
	char				szFile[MAX_PATH];
	char*				szFileExtension;
	int                 iStatus;
	struct stat         sBuffer;
	BOOL                bDir;
	CFileUtil           cFileUtil;

	if (!cFileUtil.FullPath(szDirectory, szInDirectory, MAX_PATH))
	{
		return TRIERROR;
	}

	pDIR = opendir(szDirectory);

	if (pDIR == NULL)
	{
		return TRITRUE;
	}

	pDirEnt = readdir(pDIR);
	while (pDirEnt != NULL)
	{
		bValid = TRUE;

		sprintf(szTemp, "%s%s%s", szDirectory, FILE_SEPARATOR, pDirEnt->d_name);
		iStatus = stat(szTemp, &sBuffer);
		if (iStatus == -1)
		{
			printf("%s: %s\n", strerror(errno), szTemp);
			return TRIERROR;
		}
		bDir = S_ISDIR(sBuffer.st_mode);
		if (bDir) //Directory
		{
			if (bDirs)
			{
				if (szInName != NULL)
				{
					strcpy(szFile, pDirEnt->d_name);
					RemoveExtension(szFile);
					if (StrIStr(szFile, szInName) == NULL)
					{
						bValid = FALSE;
					}
				}
				else
				{
					bValid = FALSE;
				}
			}
		}
		else
		{
			if (!bDirs)
			{
				if (szExtension != NULL)
				{
					szFileExtension = FindExtension(pDirEnt->d_name);
					szFileExtension++;
					if (StrICmp(szFileExtension, szExtension) != 0)
					{
						bValid = FALSE;
					}
				}
				if (szInName != NULL)
				{
					strcpy(szFile, pDirEnt->d_name);
					RemoveExtension(szFile);
					if (StrIStr(szFile, szInName) == NULL)
					{
						bValid = FALSE;
					}
				}
			}
			else
			{
				bValid = FALSE;
			}
		}

		if (bValid)
		{
			mcFiles.Add(szTemp, 0);
		}

		pDirEnt = readdir(pDIR);
	}

	closedir(pDIR);
	return TRITRUE;
}


#endif //LINUX_GNU_32

