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
#include "TextFile.h"
#include "FileUtil.h"
#include "YAMLFile.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CYAMLFile::Init(void)
{
	mcMarkup.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CYAMLFile::Kill(void)
{
	mcMarkup.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CYAMLFile::Read(const char* szFilename, const char* szDirectory)
{
	return Read(szFilename, szDirectory, &gcLogger);
}


//////////////////////////////////////////////////////////////////////////
//
// The file's \r\n and \r line ends become \n before it is parsed.  Errors
// go to pcLogger, named by szFilename.
//
//////////////////////////////////////////////////////////////////////////
bool CYAMLFile::Read(const char* szFilename, const char* szDirectory, CLogger* pcLogger)
{
	CYAMLParser	cYAMLParser;
	CTextFile	cTextFile;
	bool		bResult;
	TRISTATE	tResult;
	CFileUtil	cFileUtil;
	CChars		szPath;

	szPath.Init(szDirectory);
	cFileUtil.FullPath(&szPath);
	cFileUtil.AppendToPath(&szPath, szFilename);

	cTextFile.Init();
	bResult = cTextFile.Read(szPath.Text());
	if (!bResult)
	{
		pcLogger->Error2(__METHOD__, " Could not read YAML file [", szPath.Text(), "].", NULL);
		cTextFile.Kill();
		szPath.Kill();
		return false;
	}

	cTextFile.PassifyNewlines();
	cYAMLParser.Init(cTextFile.Text(), szFilename, &mcMarkup, pcLogger);
	tResult = cYAMLParser.Parse();
	cYAMLParser.Kill();
	cTextFile.Kill();
	szPath.Kill();

	return tResult == TRITRUE;
}

