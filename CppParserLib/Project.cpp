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
#include "BaseLib/FileUtil.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "Project.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProject::Init(bool bDumpLogs)
{
	mcLibraries.Init();

	mbDumpLogs = bDumpLogs;
	mcIncludeFiles.Init();
	mcHeaderNames.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProject::Kill(void)
{
	CLibrary*		pcLibrary;
	CHeaderFiles*	pcHeaderNameMap;
	size			i;

	for (i = 0; i < mcHeaderNames.NumElements(); i++)
	{
		pcHeaderNameMap = mcHeaderNames.Get(i);
		pcHeaderNameMap->Kill();
	}

	mcHeaderNames.Kill();



	pcLibrary = mcLibraries.GetHead();
	while (pcLibrary)
	{
		pcLibrary->Kill();
		pcLibrary = mcLibraries.GetNext(pcLibrary);
	}
	mcLibraries.Kill();

	mcIncludeFiles.Kill();

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLibrary* CProject::AddLibrary(char* szName, char* szBaseDir, bool bIncludeSubDirectories)
{
	CLibrary*	pcLibrary;

	pcLibrary = mcLibraries.InsertAfterTail();
	pcLibrary->Init(szName, szBaseDir, bIncludeSubDirectories, &mcIncludeFiles);

	return pcLibrary;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProject::AddIncludeDirectory(char* szBaseDir, bool bIncludeSubDirectories, bool bSystem)
{
	AddHeaderFiles(szBaseDir, bIncludeSubDirectories, bSystem);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CLibrary* CProject::GetLibrary(char* szLibrary)
{
	CLibrary*	pcLibrary;

	pcLibrary = mcLibraries.GetHead();
	while (pcLibrary)
	{
		if (pcLibrary->IsNamed(szLibrary))
		{
			return pcLibrary;
		}
		pcLibrary = mcLibraries.GetNext(pcLibrary);
	}
	return NULL;
}


/////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CTranslationUnit* CProject::Start(SProcessIter* psIter, char* szConfiguration)
{
	psIter->szConfiguration = szConfiguration;
	psIter->pcLibrary = mcLibraries.GetHead();
	return StartLibrary(psIter);
}


/////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CTranslationUnit* CProject::Iterate(SProcessIter* psIter)
{
	if (!psIter->pcLibrary)
	{
		return NULL;
	}

	if (psIter->pcLibrary->GetConfig(psIter->szConfiguration))
	{
		psIter->pcFile = psIter->pcLibrary->GetNextTranslationUnit(&psIter->sIter);
		if (psIter->pcFile)
		{
			return psIter->pcFile;
		}
	}
	psIter->pcLibrary = mcLibraries.GetNext(psIter->pcLibrary);
	return StartLibrary(psIter);
}


/////////////////////////////////////////////////////////////////////////
//
// Returns the first translation unit of psIter->pcLibrary or of the
// libraries after it, skipping libraries without the configuration and
// libraries with no translation units.  psIter->pcLibrary is left on the
// library the unit is from, or NULL.
//
//////////////////////////////////////////////////////////////////////////
CTranslationUnit* CProject::StartLibrary(SProcessIter* psIter)
{
	while (psIter->pcLibrary)
	{
		if (psIter->pcLibrary->GetConfig(psIter->szConfiguration))
		{
			psIter->pcFile = psIter->pcLibrary->GetFirstTranslationUnit(&psIter->sIter);
			if (psIter->pcFile)
			{
				return psIter->pcFile;
			}
		}
		psIter->pcLibrary = mcLibraries.GetNext(psIter->pcLibrary);
	}
	return NULL;
}


/////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CProject::Process(char* szConfiguration, CChars* pszErrors, CChars* pszWarnings)
{
	CTranslationUnit*	pcFile;
	CConfig*			pcConfig;
	bool				bResult;
	CPreprocessor		cPreprocessor;
	SProcessIter		sPreIter;
	bool				bTotalResult;

	CreatePost();

	bTotalResult = true;
	pcFile = Start(&sPreIter, szConfiguration);
	while (pcFile)
	{
		DumpProcessedCPPFileName(pcFile);
		
		pcConfig = pcFile->GetLibrary()->GetConfig(szConfiguration);

		cPreprocessor.Init(pcConfig);
		cPreprocessor.AddIncludeDirectories(&mcHeaderNames);
		cPreprocessor.AddIncludeDirectory(pcFile->GetLibrary()->GetHeaderFiles());
		cPreprocessor.LogDumping(mbDumpLogs);

		ClearPragmaOnceFromAllFiles();

		bResult = cPreprocessor.PreprocessTranslationUnit(pcFile);
		bTotalResult &= bResult;
		if (pszErrors)
		{
			pszErrors->Append(cPreprocessor.GetErrors()->Text());
		}
		if (pszWarnings)
		{
			pszWarnings->Append(cPreprocessor.GetWarnings()->Text());
		}

		WritePost(pcFile);

		cPreprocessor.Kill();
		pcFile = Iterate(&sPreIter);
	}

	return bTotalResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CProject::DumpProcessedCPPFileName(CSourceFile* pcFile)
{
	CChars			sz;

	sz.Init("Process (");
	sz.Append(pcFile->ShortName());
	sz.Append(")\n");
	sz.DumpKill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CProject::ClearPragmaOnceFromAllFiles(void)
{
	CHeaderFile*	pcHeader;
	SMapIterator	sIter;

	pcHeader = mcIncludeFiles.StartIteration(&sIter);
	while (pcHeader)
	{
		pcHeader->ClearPragmaOnce();
		pcHeader = mcIncludeFiles.Iterate(&sIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CProject::CreatePost(void)
{
	CFileUtil	cFileUtil;
	CChars		szPath;

	szPath.Init("Post");
	cFileUtil.FullPath(&szPath);
	cFileUtil.RemoveDir(szPath.Text());
	cFileUtil.MakeDir(szPath.Text());
	szPath.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CProject::WritePost(CTranslationUnit* pcTU)
{
	CFileUtil		cFileUtil;
	CFileBasic		cDestFile;
	CChars			sz;
	CChars			szPath;

	szPath.Init("Post");
	cFileUtil.FullPath(&szPath);
	cFileUtil.AppendToPath(&szPath, pcTU->ShortName());

	cDestFile.Init(DiskFile(szPath.Text()));
	cDestFile.Open(EFM_ReadWrite_Create);
	sz.Init();

	pcTU->Print(&sz);
	cDestFile.Write(sz.Text(), sz.Length(), 1);

	sz.Kill();
	szPath.Kill();
	cDestFile.Close();
	cDestFile.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CProject::DumpPost(CTranslationUnit* pcTU)
{
	CChars			sz;

	sz.Init();

	pcTU->Print(&sz);
	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProject::AddHeaderFiles(char* szDirectoryName, bool bIncludeSubDirectories, bool bSystem)
{
	CHeaderFiles* pcHeaderNameMap;

	pcHeaderNameMap = mcHeaderNames.Add();
	pcHeaderNameMap->Init(szDirectoryName, &mcIncludeFiles, bIncludeSubDirectories, bSystem);
}

