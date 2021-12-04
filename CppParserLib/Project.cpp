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
#include <crtdbg.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/FileBasic.h"
#include "BaseLib/DiskFile.h"
#include "Project.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProject::Init(BOOL bDumpLogs, BOOL bLogBlocks)
{
	mcLibraries.Init();

	miNumSystemLibraries = 0;
	miBlockReuse = 0;
	mbDumpLogs = bDumpLogs;
	mbLogBlocks = bLogBlocks;
	mcIncludeFiles.Init();
	mcIncludeNames.Init(&mcIncludeFiles);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CProject::Kill(void)
{
	CLibrary*	pcLibrary;

	mcIncludeNames.Kill();
	mcIncludeFiles.Kill();

	miNumSystemLibraries = 0;

	pcLibrary = mcLibraries.GetHead();
	while (pcLibrary)
	{
		pcLibrary->Kill();
		pcLibrary = mcLibraries.GetNext(pcLibrary);
	}
	mcLibraries.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CLibrary* CProject::AddLibrary(char* szName, char* szBaseDir, BOOL bIncludeSubDirectories)
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
void CProject::AddIncludeDirectory(char* szBaseDir, BOOL bIncludeSubDirectories, BOOL bSystem)
{
	mcIncludeNames.Add(szBaseDir, bIncludeSubDirectories, bSystem);
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
//void CProject::Process(char* szConfiguration)
//{
//						cTokeniser.Init();
//						//bResult = cTokeniser.Tokenise(&cPreprocessor.mcPost);
//						//if (!bResult)
//						//{
//						//	bError = TRUE;
//						//	cTokeniser.Kill();
//						//	cParser.Kill();
//						//	cPreprocessor.Kill();
//						//	break;
//						//}
//						//cParser.BuildAST(cPreprocessor.mszPost.Text());
//
//						cTokeniser.Kill();
//}


/////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CTranslationUnit* CProject::Start(SProcessIter* psIter, char* szConfiguration)
{
	CConfig*	pcConfig;

	psIter->szConfiguration = szConfiguration;
	psIter->bError = FALSE;
	psIter->pcLibrary = mcLibraries.GetHead();

	while (psIter->pcLibrary)
	{
		pcConfig = psIter->pcLibrary->GetConfig(psIter->szConfiguration);
		if (pcConfig)
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
CTranslationUnit* CProject::Iterate(SProcessIter* psIter)
{
	CConfig*	pcConfig;

	if (psIter->bError)
	{
		return NULL;
	}

	if (!psIter->pcLibrary)
	{
		return NULL;
	}

	for (;;)
	{
		pcConfig = psIter->pcLibrary->GetConfig(psIter->szConfiguration);
		if (pcConfig)
		{
			psIter->pcFile = psIter->pcLibrary->GetNextTranslationUnit(&psIter->sIter);
			if (psIter->pcFile)
			{
				return psIter->pcFile;
			}
		}
		psIter->pcLibrary = mcLibraries.GetNext(psIter->pcLibrary);
		if (psIter->pcLibrary)
		{
			psIter->pcFile = psIter->pcLibrary->GetFirstTranslationUnit(&psIter->sIter);
		}
		else
		{
			break;
		}
	}
	return NULL;
}


/////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CProject::Process(char* szConfiguration)
{
	CTranslationUnit*			pcFile;
	CConfig*					pcConfig;
	BOOL						bResult;
	CPreprocessor				cPreprocessor;
	SProcessIter				sPreIter;

	CreatePost();

	pcFile = Start(&sPreIter, szConfiguration);
	while (pcFile)
	{
		DumpProcessedCPPFileName(pcFile);
		
		pcConfig = pcFile->GetLibrary()->GetConfig(szConfiguration);

		cPreprocessor.Init(pcConfig, pcFile->GetTokenMemory());
		cPreprocessor.AddIncludeDirectories(&mcIncludeNames.mcHeaderNames);
		cPreprocessor.AddIncludeDirectory(pcFile->GetLibrary()->GetHeaderFiles());
		cPreprocessor.LogDumping(mbDumpLogs);
		cPreprocessor.LogBlocks(mbLogBlocks);

		ClearPragmaOnceFromAllFiles();

		bResult = cPreprocessor.PreprocessTranslationUnit(pcFile);
		if (!bResult)
		{
			sPreIter.bError = TRUE;
			cPreprocessor.Kill();
			break;
		}
		miBlockReuse += cPreprocessor.GetBlockReuse();

		WritePost(pcFile);

		cPreprocessor.Kill();
		pcFile = Iterate(&sPreIter);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CProject::DumpProcessedCPPFileName(CCFile* pcFile)
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
	//	EngineOutput(sz.Text());
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
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CProject::GetBlockReuse(void)
{
	return miBlockReuse;
}


