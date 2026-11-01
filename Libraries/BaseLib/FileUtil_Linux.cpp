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
#ifdef LINUX_GNU
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

	if (getcwd(szTemp, MAX_PATH) != NULL)
	{
		szDest->Append(szTemp);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::MakeDir(const char*szPathName)
{
    int iResult;

	//The process umask decides the final permissions, as the default security descriptor does on Windows.
	iResult = mkdir(szPathName, 0777);
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
    bool                bDeleted;

	szDirectory.Init(szPathName);
	RemoveLastSeparator(&szDirectory);

    pDIR = opendir(szPathName);

    if (pDIR == NULL)
    {
        //A directory that does not exist is already removed, matching Windows.
        szDirectory.Kill();
        return errno == ENOENT;
    }

	//Like Windows only the deletion of files decides the result.
	bDeleted = true;
    pDirEnt = readdir(pDIR);
    while (pDirEnt != NULL)
    {
        if (!((StringCompare(pDirEnt->d_name, ".") == 0) || (StringCompare(pDirEnt->d_name, "..") == 0)))
        {
            szTemp.Init(szDirectory);
            szTemp.Append(FILE_SEPARATOR);
            szTemp.Append(pDirEnt->d_name);

            //lstat so that a symbolic link to a directory is removed, not followed.
            iStatus = lstat(szTemp.Text(), &sBuffer);
            if ((iStatus == 0) && S_ISDIR(sBuffer.st_mode))
            {
				RemoveDir(szTemp.Text());
			}
            else
            {
                if ((unlink(szTemp.Text()) != 0) && (errno != ENOENT))
                {
                    bDeleted = false;
                }
            }
            szTemp.Kill();
		}
		pDirEnt = readdir(pDIR);
	}

    closedir(pDIR);
	rmdir(szDirectory.Text());
	szDirectory.Kill();
	return bDeleted;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::RecurseCopyDir(const char* szSource, const char* szDest)
{
	DIR*				pDIR;
	struct dirent*		pDirEnt;
	CChars				szSourceTemp;
	CChars				szDestTemp;
	CChars				szSourceDirectory;
	CChars				szDestDirectory;
	bool				bResult;

	szSourceDirectory.Init(szSource);
	RemoveLastSeparator(&szSourceDirectory);
	szDestDirectory.Init(szDest);
	RemoveLastSeparator(&szDestDirectory);

	bResult = true;
	pDIR = opendir(szSourceDirectory.Text());
	if (pDIR != NULL)
	{
		pDirEnt = readdir(pDIR);
		while ((pDirEnt != NULL) && bResult)
		{
			if (!((StringCompare(pDirEnt->d_name, ".") == 0) || (StringCompare(pDirEnt->d_name, "..") == 0)))
			{
				szSourceTemp.Init(szSourceDirectory);
				szDestTemp.Init(szDestDirectory);

				AppendToPath(&szSourceTemp, pDirEnt->d_name);
				AppendToPath(&szDestTemp, pDirEnt->d_name);

				bResult = CopyDirEntry(szSourceTemp.Text(), szDestTemp.Text(), IsDirectory(szSourceTemp.Text()));

				szSourceTemp.Kill();
				szDestTemp.Kill();
			}
			pDirEnt = readdir(pDIR);
		}
		closedir(pDIR);
	}
	else
	{
		bResult = gcLogger.Error2(__METHOD__, " Could not read directory [", szSourceDirectory.Text(), "].", NULL);
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
	struct stat		sBuffer;

	return (stat(szPathName, &sBuffer) == 0) && S_ISDIR(sBuffer.st_mode);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::Touch(const char*szFilename)
{
    int     iHandle;

	//Creates the file, or empties it if it exists, like Windows CREATE_ALWAYS.
    iHandle = open(szFilename, O_WRONLY | O_CREAT | O_TRUNC, 0666);
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
bool CFileUtil::SyncFile(const char* szFilename)
{
    int     iHandle;
    int     iResult;

    iHandle = open(szFilename, O_RDONLY);
    if (iHandle == -1)
    {
        return false;
    }

    iResult = fsync(iHandle);
    close(iHandle);
    return iResult == 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CFileUtil::SyncDirectory(const char* szDirectory)
{
    int     iHandle;
    int     iResult;

	//Makes files created, deleted or renamed in the directory survive a power loss.
    iHandle = open(szDirectory, O_RDONLY | O_DIRECTORY);
    if (iHandle == -1)
    {
        return false;
    }

    iResult = fsync(iHandle);
    close(iHandle);
    return iResult == 0;
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
    struct stat     sBuffer;

	//True for files and directories whether or not they can be read, like Windows PathFileExists.
	return stat(szFilename, &sBuffer) == 0;
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
    sz.Kill();

    //Windows cannot open a directory as a file so it has no size either.
    if ((iStatus == -1) || S_ISDIR(sBuffer.st_mode))
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
void FreeDirEnts(struct dirent** apDirEnts, size iNumDirEnts)
{
	size	i;

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
	//Match the order NTFS returns directory entries in.
	return CompareFileNamesNTFS((*ppDirEnt1)->d_name, (*ppDirEnt2)->d_name);
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
	size                i;
    CChars				szTemp;
	bool				bValid;
    CChars				szDirectory;
	int                 iStatus;
	struct stat         sBuffer;
	bool                bDirectory;

	szDirectory.Init(szInDirectory);
	FullPath(&szDirectory);

	iNumDirEnts = scandir(szDirectory.Text(), &apDirEnts, NULL, CompareDirEntsNTFS);
	if (iNumDirEnts < 0)
	{
	    szDirectory.Kill();
		return false;
	}

	for (i = 0; i < (size)iNumDirEnts; i++)
	{
		pDirEnt = apDirEnts[i];

		//Dot files are hidden on Linux; Windows uses the hidden attribute.  "." and ".." are never returned.
		if ((pDirEnt->d_name[0] == '.') && !bHidden)
		{
			continue;
		}

		szTemp.Init(szDirectory);
		szTemp.Append(FILE_SEPARATOR);
        szTemp.Append(pDirEnt->d_name);
		iStatus = stat(szTemp.Text(), &sBuffer);
		szTemp.Kill();

		//An entry that cannot be stat'ed (e.g. a broken symbolic link) is listed as a file, as Windows lists it.
        bDirectory = (iStatus == 0) && S_ISDIR(sBuffer.st_mode);
		bValid = IsFindMatch(pDirEnt->d_name, bDirectory, bDirs, szInName, szExtension);

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

	FreeDirEnts(apDirEnts, (size)iNumDirEnts);
	szDirectory.Kill();
	return true;
}


#endif //LINUX_GNU

