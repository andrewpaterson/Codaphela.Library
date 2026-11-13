/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppCompilerLib

Codaphela CppCompilerLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppCompilerLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppCompilerLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __HEADER_NAME_MAP_H__
#define __HEADER_NAME_MAP_H__
#include "BaseLib/MapStringTemplate.h"
#include "BaseLib/ArrayTemplate.h"
#include "HeaderFile.h"


//The extensions of an include directory's headers, besides files with none (<new>).
#define CPP_HEADER_EXTENSIONS	"h", "hh", "hpp", "hxx", "inl", "rh"

//Include names match a file's whatever their case only where file names do (Windows).
#ifdef _WIN32
#define CPP_HEADER_NAMES_CASE_SENSITIVE	false
#else
#define CPP_HEADER_NAMES_CASE_SENSITIVE	true
#endif


typedef CMapStringTemplate<CHeaderFile*>	CHeaderFilePtrMap;


//An include directory: its headers by their names relative to it ("BaseLib/Chars.h").  One with its sub directories
//lists them all when it is made.  One without lists the headers in it, and finds one in a sub directory when it is
//first asked for, as a C++ compiler's -I does (GetFile).
class CHeaderFileMap;
class CHeaderFiles
{
protected:
	CHeaderFileMap*		mpcFileMap;  //Relative names
	CHeaderFilePtrMap	mcFileNames;		//NULL for a name looked for in a sub directory and not there.
	CChars				mszBaseDirectory;
	bool				mbSystem;
	bool				mbSubDirectories;

public:
	bool			Init(char* szBaseDirectory, CHeaderFileMap* pcFileMap, bool bIncludeSubDirectories, bool bSystem);
	void			Kill(void);

	CHeaderFile*	AddFile(char* szRelativeFileName);
	void			AddFiles(bool bIncludeSubDirectories);

	CHeaderFile*	GetFile(char* szRelativeFileName);
	bool			HasSubDirectories(void);
	
	char*			GetBaseDirectory(void);
	size			GetBaseDirectoryLength(void);

	void			Dump(void);

protected:
	CHeaderFile*	FindInSubDirectory(char* szRelativeFileName);
};


#endif // !__HEADER_NAME_MAP_H__

