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
#include "ArrayString.h"


#define FILE_BLOCK_SIZE		(1 MB)


class CFileUtil
{
private:
    BOOL    RecurseRemoveDir(char* szPathName);
	BOOL	FindFiles(char* szPathName, BOOL bDirectories, char* szInName, char* szExtension, CArrayString* paszFiles, BOOL bHidden);
	BOOL	RecurseFindFiles(char* szPathName, char* szInName, char* szExtension, CArrayString* paszFiles, BOOL bHidden);

public:
	//Disk methods.
	BOOL    MakeDir(char* szPathName);
	BOOL    RemoveDir(char* szPathName);
	BOOL    Touch(char* szFileName);
	BOOL    Delete(char* szFileName);
	BOOL    Exists(char* szFileName);
	BOOL    Compare(char* szFileName1, char* szFileName2);
	BOOL    Copy(char* szSource, char* szDest);
	int	    Size(char* szFileName);

	//Path name methods
	BOOL	IsAbsolutePath(char* szPathName);
	void	FullPath(CChars* szPathName);
	void	FixSeparators(CChars* szPathName);
	char	GetDriveLetter(char* szPathName);
	void	CollapsePath(CChars* szPathName);
	void	CurrentDirectory(CChars* szDest);
	void	SplitPath(char* szPathName, CChars* szDestFileName, CChars* szDestDirectory);
	void	SplitPath(char* szPathName, CArrayString* paszComponents);

	void    RemoveExtension(CChars* szPathName);
	int		FindExtension(char* szPathName);
	int		FindLastSeparator(char* szPathName);
	void    RemoveFileSeparator(CChars* szPathName);
	void    AppendToPath(CChars* szPathName, char* szItem);
	void    RemoveLastFromPath(CChars* szPathName);
	void	RemovePath(CChars* szPathName);
	BOOL	IsExtension(char* szFileName, char* szExtension);

	//Finder methods
	void	FindFilesWithNameContaining(char* szPathName, char* szFileName, CArrayString* paszFiles, BOOL bIncludeSubDirs = FALSE, BOOL bHidden= TRUE);
	void	FindFilesWithExtension(char* szPathName, char* szExtension, CArrayString* paszFiles, BOOL bIncludeSubDirs = FALSE, BOOL bHidden = TRUE);
	void	FindAllDirectories(char* szPathName, CArrayString* paszFiles, BOOL bHidden = TRUE);
	BOOL	FindAllFiles(char* szPathName, CArrayString* paszFiles, BOOL bIncludeSubDirs = FALSE, BOOL bHidden = TRUE);

	//Miscellaneous methods.  I'm not sure what class this belongs in.
	void	MakeNameFromDirectory(CChars* pszName, CChars* pszFileName, CChars* pszDirectory);
};


#endif // __FILE_UTIL_H__

