/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "Library.h"
#include "BaseLib/FileUtil.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLibrary::Init(char* szName, char* szBaseDir, bool bIncludeSubDirectories, CHeaderFileMap* pcHeaderFileMap)
{
	CFileUtil	cFileUtil;
	CChars		szPath;

	szPath.Init(szBaseDir);
	cFileUtil.FullPath(&szPath);

	mszName.Init(szName);
	mszBaseDirectory.Init(szPath);
	szPath.Kill();
	mcConfigs.Init();
	mcTranslationUnits.Init(this);
	mcHeaderNameMap.Init(mszBaseDirectory.Text(), pcHeaderFileMap, bIncludeSubDirectories, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLibrary::Kill(void)
{
	CConfig*	pcConfig;

	mcHeaderNameMap.Kill();
	mcTranslationUnits.Kill();

	pcConfig = mcConfigs.GetHead();
	while (pcConfig)
	{
		pcConfig->Kill();
		pcConfig = mcConfigs.GetNext(pcConfig);
	}
	mcConfigs.Kill();
	mszName.Kill();
	mszBaseDirectory.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CTranslationUnit* CLibrary::AddTranslationUnit(char* szRelativeNameFile, bool bLogIncludes, bool bLogBlocks)
{
	return mcTranslationUnits.AddFile(szRelativeNameFile, bLogIncludes, bLogBlocks);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CHeaderFile* CLibrary::AddHeaderFile(char* szRelativeNameFile)
{
	return mcHeaderNameMap.AddFile(szRelativeNameFile);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CLibrary::AddAllTranslationUnitsInBaseDir(bool bLogInlucdes, bool bLogBlocks)
{
	mcTranslationUnits.AddAllFiles(&mszBaseDirectory, bLogInlucdes, bLogBlocks);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CConfig* CLibrary::AddConfiguration(char* szConfig)
{
	CConfig* pcConfig;

	pcConfig = mcConfigs.InsertAfterTail();
	pcConfig->Init(szConfig);
	return pcConfig;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CConfig* CLibrary::GetConfig(char* szConfiguration)
{
	CConfig* pcConfig;

	pcConfig = mcConfigs.GetHead();
	while (pcConfig)
	{
		if (pcConfig->mszName.Equals(szConfiguration))
		{
			return pcConfig;
		}
		pcConfig = mcConfigs.GetNext(pcConfig);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CTranslationUnit* CLibrary::GetFirstTranslationUnit(SCFileIter* psIter)
{
	psIter->pcLast = mcTranslationUnits.GetFirst();
	if (psIter->pcLast)
	{
		psIter->bValid = true;
	}
	else
	{
		psIter->bValid = false;
	}
	return psIter->pcLast;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CTranslationUnit* CLibrary::GetNextTranslationUnit(SCFileIter* psIter)
{
	if (!psIter->bValid)
	{
		return NULL;
	}
	
	psIter->pcLast = mcTranslationUnits.GetNext(psIter->pcLast);
	if (psIter->pcLast)
	{
		return psIter->pcLast;
	}
	else
	{
		psIter->bValid = false;
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CLibrary::GetBaseDir(void)
{
	return mszBaseDirectory.Text();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CLibrary::IsNamed(char* szName)
{
	return mszName.Equals(szName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CHeaderFiles* CLibrary::GetHeaderFiles(void)
{
	return &mcHeaderNameMap;
}

