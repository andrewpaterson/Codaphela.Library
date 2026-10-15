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
#ifdef LINUX_GNU_32
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
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

	getcwd(szTemp, MAX_PATH);
	szDest->Append(szTemp);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::MakeDir(const char*szPathName)
{
    int iResult;

	iResult = mkdir(szPathName, S_IRWXU);
	if (iResult == 0)
	{
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
bool CFileUtil::RemoveDir(const char*szPathName)
{
    CChars  sz;
    bool    bResult;

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
bool CFileUtil::RecurseRemoveDir(const char*szPathName)
{
    DIR*                pDIR;
    struct dirent*      pDirEnt;
	CChars              szTemp;
	CChars              szDirectory;
    struct stat         sBuffer;
    int                 iStatus;
    bool                bDir;

	szDirectory.Init(szPathName);
	RemoveLastSeparator(&szDirectory);

    pDIR = opendir(szPathName);

    if (pDIR == NULL)
    {
        //A directory that does not exist is already removed, matching Windows.
        szDirectory.Kill();
        return errno == ENOENT;
    }

    pDirEnt = readdir(pDIR);
    while (pDirEnt != NULL)
    {
        if (!((StringCompare(pDirEnt->d_name, ".") == 0) || (StringCompare(pDirEnt->d_name, "..") == 0)))
        {
            szTemp.Init(szDirectory);
            szTemp.Append(FILE_SEPARATOR);
            szTemp.Append(pDirEnt->d_name);

            iStatus = stat(szTemp.Text(), &sBuffer);
            if (iStatus == -1)
            {
                printf("%s: %s\n", strerror(errno), szTemp.Text());
                return false;
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Touch(const char*szFilename)
{
    int     iHandle;

    iHandle = open(szFilename, O_WRONLY | O_CREAT | O_EXCL, S_IWUSR | S_IRUSR);
	if (iHandle != -1)
	{
		close(iHandle);
		return true;
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Delete(const char*szFilename)
{
    int     iResult;

	iResult = unlink(szFilename);
	if ((iResult == 0) || (errno == ENOENT))
	{
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
bool CFileUtil::Exists(const char* szFilename)
{
    int     iHandle;

    iHandle = open(szFilename, O_RDONLY, S_IWUSR | S_IRUSR);
	if (iHandle != -1)
	{
		close(iHandle);
		return true;
	}
	return false;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
filePos CFileUtil::Size(const char*szFilename)
{
    struct stat         sBuffer;
    int                 iStatus;
    CChars              sz;

    sz.Init(szFilename);
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
bool CFileUtil::IsAbsolutePath(const char*szPathName)
{
	const char*  pcPos;

	pcPos = FindChar(szPathName, FILE_SEPARATOR[0]);
	if (pcPos == szPathName)
	{
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
char CFileUtil::GetDriveLetter(const char*szPathName)
{
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void FreeDirEnts(struct dirent** apDirEnts, int iNumDirEnts)
{
	int		i;

	for (i = 0; i < iNumDirEnts; i++)
	{
		free(apDirEnts[i]);
	}
	free(apDirEnts);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CompareDirEntsNTFS(const struct dirent** ppDirEnt1, const struct dirent** ppDirEnt2)
{
	//Match the order NTFS returns directory entries in: names compared upper-cased.
	const unsigned char*	ps1;
	const unsigned char*	ps2;
	int						c1;
	int						c2;

	ps1 = (const unsigned char*)(*ppDirEnt1)->d_name;
	ps2 = (const unsigned char*)(*ppDirEnt2)->d_name;
	do
	{
		c1 = toupper(*ps1++);
		c2 = toupper(*ps2++);
	}
	while ((c1 == c2) && (c1 != 0));
	return c1 - c2;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::FindFiles(const char*szInDirectory, bool bDirs, const char*szInName, const char*szExtension, CArrayChars* paszFiles, bool bHidden)
{
	struct dirent**     apDirEnts;
	struct dirent*      pDirEnt;
	int                 iNumDirEnts;
	int                 i;
	CChars				szFindName;
    CChars				szTemp;
	bool				bValid;
    CChars				szDirectory;
    CChars				szFile;
    CChars				szFake;
	int                 iStatus;
	struct stat         sBuffer;
	bool                bDirectory;
	CFileUtil           cFileUtil;
	int                 iFileExtension;

	szDirectory.Init(szInDirectory);
	FullPath(&szDirectory);

	iNumDirEnts = scandir(szDirectory.Text(), &apDirEnts, NULL, CompareDirEntsNTFS);
	if (iNumDirEnts < 0)
	{
	    szDirectory.Kill();
		return true;
	}

	for (i = 0; i < iNumDirEnts; i++)
	{
		pDirEnt = apDirEnts[i];
		bValid = true;

		szTemp.Init(szDirectory);
		szTemp.Append(FILE_SEPARATOR);
        szTemp.Append(pDirEnt->d_name);

		iStatus = stat(szTemp.Text(), &sBuffer);
		if (iStatus == -1)
		{
			printf("%s: %s\n", strerror(errno), szTemp.Text());
			szTemp.Kill();
			szDirectory.Kill();
			FreeDirEnts(apDirEnts, iNumDirEnts);
			return false;
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
                    szFake.Fake(pDirEnt->d_name);
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
                    szTemp.Init(pDirEnt->d_name);
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
            if (!((StringCompare(pDirEnt->d_name, ".") == 0) || (StringCompare(pDirEnt->d_name, "..") == 0)))
            {
                szTemp.Init(szDirectory);
                AppendToPath(&szTemp, pDirEnt->d_name);
                paszFiles->Add(szTemp.Text());
                szTemp.Kill();
            }
        }
	}

	FreeDirEnts(apDirEnts, iNumDirEnts);
	return true;
}


#endif //LINUX_GNU_32

