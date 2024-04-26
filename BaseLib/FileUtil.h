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
#ifndef __FILE_UTIL_H__
#define __FILE_UTIL_H__
#include "Numbers.h"
#include "Chars.h"
#include "ArrayChars.h"


#define FILE_BLOCK_SIZE		(1 MB)
#define MAX_DIRECTORY_LENGTH 65536


class CFileUtil
{
public:
	//Disk methods.
	bool    MakeDir(const char* szPathName);
	bool    RemoveDir(const char* szPathName);
	bool    CopyDir(const char* szSource, const char* szDest);
	bool    Touch(const char* szFilename);
	bool    TouchDir(const char* szFilename, bool bLastIsFilename = false);  //MakeDir -P
	bool    Delete(const char* szFilename);
	bool    Exists(const char* szFilename);
	bool    Compare(const char* szFilename1, const char* szFilename2);
	bool	CompareSize(const char* szFilename1, const char* szFilename2);

	bool    Copy(const char* szSource, const char* szDest);
	filePos Size(const char* szFilename);
	bool    MakeDirs(bool bRemoveFirst, const char* szPathName, ...);
	bool    RemoveDirs(const char* szPathName, ...);

	//Path name methods
	bool	IsAbsolutePath(const char* szPathName);
	void	FullPath(CChars* szPathName);
	char	GetDriveLetter(const char* szPathName);
	void	CollapsePath(CChars* szPathName);
	void	CurrentDirectory(CChars* szDest);
	void	SplitPath(const char* szPathName, CChars* szDestFilename, CChars* szDestDirectory);
	void	SplitPath(const char* szPathName, CArrayChars* paszComponents);

	void    RemoveExtension(CChars* szPathName);
	int		FindExtension(const char* szPathName);
	void    AppendToPath(CChars* szPathName, const char* szItem);
	void    PrependToPath(CChars* szPathName, const char* szItem);
	void    RemoveLastFromPath(CChars* szPathName);
	void	RemovePath(CChars* szPathName);
	bool	IsExtension(const char* szFilename, const char* szExtension);
	bool	IsRootDirectory(const char* szPathName);

	//Finder methods
	void	FindFilesWithNameContaining(const char* szPathName, const char* szFilename, CArrayChars* paszFiles, bool bIncludeSubDirs = false, bool bHidden= true);
	void	FindFilesWithExtension(const char* szPathName, const char* szExtension, CArrayChars* paszFiles, bool bIncludeSubDirs = false, bool bHidden = true);
	void	FindAllDirectories(const char* szPathName, CArrayChars* paszFiles, bool bHidden = true);
	bool	FindAllFiles(const char* szPathName, CArrayChars* paszFiles, bool bIncludeSubDirs = false, bool bHidden = true);

	//Miscellaneous methods.  I'm not sure what class this belongs in.
	void	MakeNameFromDirectory(CChars* pszName, CChars* pszFilename, CChars* pszDirectory);
	void	FixSeparators(CChars* szPathName);

private:
	bool    RecurseRemoveDir(const char* szPathName);
	bool	FindFiles(const char* szPathName, bool bDirectories, const char* szInName, const char* szExtension, CArrayChars* paszFiles, bool bHidden);
	bool	RecurseFindFiles(const char* szPathName, const char* szInName, const char* szExtension, CArrayChars* paszFiles, bool bHidden);
	bool	Compare(const char* szFilename1, const char* szFilename2, bool bSizeOnly);

	int		FindFirstSeparator(const char* szString);
	int		FindLastSeparator(const char* szPathName);
	void    RemoveLastSeparator(CChars* szPathName);
};


#endif // __FILE_UTIL_H__

