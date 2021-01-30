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
#ifndef __FILE_UTIL_H__
#define __FILE_UTIL_H__
#include "Define.h"
#include "Numbers.h"
#include "Chars.h"
#include "ArrayChars.h"


#define FILE_BLOCK_SIZE		(1 MB)
#define MAX_DIRECTORY_LENGTH 65536


class CFileUtil
{
public:
	//Disk methods.
	BOOL    MakeDir(const char* szPathName);
	BOOL    RemoveDir(const char* szPathName);
	BOOL    CopyDir(const char* szSource, const char* szDest);
	BOOL    Touch(const char* szFileName);
	BOOL    TouchDir(const char* szFileName, BOOL bLastIsFileName = FALSE);  //MakeDir -P
	BOOL    Delete(const char* szFileName);
	BOOL    Exists(const char* szFileName);
	BOOL    Compare(const char* szFileName1, const char* szFileName2);
	BOOL	CompareSize(const char* szFileName1, const char* szFileName2);

	BOOL    Copy(const char* szSource, const char* szDest);
	filePos Size(const char* szFileName);
	BOOL    MakeDirs(BOOL bRemoveFirst, const char* szPathName, ...);
	BOOL    RemoveDirs(const char* szPathName, ...);

	//Path name methods
	BOOL	IsAbsolutePath(const char* szPathName);
	void	FullPath(CChars* szPathName);
	char	GetDriveLetter(const char* szPathName);
	void	CollapsePath(CChars* szPathName);
	void	CurrentDirectory(CChars* szDest);
	void	SplitPath(const char* szPathName, CChars* szDestFileName, CChars* szDestDirectory);
	void	SplitPath(const char* szPathName, CArrayChars* paszComponents);

	void    RemoveExtension(CChars* szPathName);
	int		FindExtension(const char* szPathName);
	void    AppendToPath(CChars* szPathName, const char* szItem);
	void    PrependToPath(CChars* szPathName, const char* szItem);
	void    RemoveLastFromPath(CChars* szPathName);
	void	RemovePath(CChars* szPathName);
	BOOL	IsExtension(const char* szFileName, const char* szExtension);
	BOOL	IsRootDirectory(const char* szPathName);

	//Finder methods
	void	FindFilesWithNameContaining(const char* szPathName, const char* szFileName, CArrayChars* paszFiles, BOOL bIncludeSubDirs = FALSE, BOOL bHidden= TRUE);
	void	FindFilesWithExtension(const char* szPathName, const char* szExtension, CArrayChars* paszFiles, BOOL bIncludeSubDirs = FALSE, BOOL bHidden = TRUE);
	void	FindAllDirectories(const char* szPathName, CArrayChars* paszFiles, BOOL bHidden = TRUE);
	BOOL	FindAllFiles(const char* szPathName, CArrayChars* paszFiles, BOOL bIncludeSubDirs = FALSE, BOOL bHidden = TRUE);

	//Miscellaneous methods.  I'm not sure what class this belongs in.
	void	MakeNameFromDirectory(CChars* pszName, CChars* pszFileName, CChars* pszDirectory);
	void	FixSeparators(CChars* szPathName);

private:
	BOOL    RecurseRemoveDir(const char* szPathName);
	BOOL	FindFiles(const char* szPathName, BOOL bDirectories, const char* szInName, const char* szExtension, CArrayChars* paszFiles, BOOL bHidden);
	BOOL	RecurseFindFiles(const char* szPathName, const char* szInName, const char* szExtension, CArrayChars* paszFiles, BOOL bHidden);
	BOOL	Compare(const char* szFileName1, const char* szFileName2, BOOL bSizeOnly);

	int		FindFirstSeparator(const char* szString);
	int		FindLastSeparator(const char* szPathName);
	void    RemoveLastSeparator(CChars* szPathName);
};


#endif // __FILE_UTIL_H__

