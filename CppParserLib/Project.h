/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2022 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __PROJECT_H__
#define __PROJECT_H__
#include "BaseLib/TextParser.h"
#include "BaseLib/Chars.h"
#include "Library.h"
#include "Preprocessor.h"
#include "CppParser.h"
#include "CppTokeniser.h"
#include "SourceFile.h"
#include "TranslationUnit.h"
#include "HeaderFileMap.h"

struct SProcessIter
{
	char*				szConfiguration;
	CLibrary*			pcLibrary;
	SCFileIter			sIter;
	CTranslationUnit*	pcFile;
};


class CProject
{
protected:
	CListLibraries			mcLibraries;
	int						miNumSystemLibraries;
	int						miBlockReuse;
	bool					mbDumpLogs;
	bool					mbLogBlocks;
	CArrayHeaderNameMap		mcHeaderNames;
	CHeaderFileMap			mcIncludeFiles;

public:
	void				Init(bool bDumpLogs = false, bool bLogBlocks = false);
	void				Kill(void);
	CLibrary*			AddLibrary(char* szName, char* szBaseDir, bool bIncludeSubDirectories);
	void				AddIncludeDirectory(char* szBaseDir, bool bIncludeSubDirectories, bool bSystem);
	CLibrary*			GetLibrary(char* szLibrary);
	bool				Process(char* szConfiguration);
	void				CreatePost(void);
	void				WritePost(CTranslationUnit* pcTU);
	void				DumpPost(CTranslationUnit* pcTU);
	void				ClearPragmaOnceFromAllFiles(void);
	CTranslationUnit*	Start(SProcessIter* psIter, char* szConfiguration);
	CTranslationUnit*	Iterate(SProcessIter* psIter);
	void				DumpProcessedCPPFileName(CSourceFile* pcFile);
	int					GetBlockReuse(void);

	void				AddHeaderFiles(char* szDirectoryName, bool bIncludeSubDirectories, bool bSystem);
};


#endif // !__PROJECT_H__

