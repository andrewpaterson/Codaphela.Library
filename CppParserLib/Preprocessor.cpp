/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#include "CoreLib/Calculator.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Numbers.h"
#include "Project.h"
#include "Preprocessor.h"
#include "PPText.h"
#include "PPReplacement.h"
#include "PPConditional.h"
#include "TokenHelper.h"
#include "GeneralToken.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::Init(CConfig* pcConfig, CMemoryStackExtended* pcStack)
{
	mcDefines.Init(32);
	mpcPost = NULL;
	mpcUnit = NULL;
	miIncludeDepth = 0;
	miBlockReuse = 0;
	mbLogBlocks = FALSE;
	mbLogInlucdes = FALSE;
	mbDumpLogs = FALSE;
	miDefineReuse = 0;

	mcConditionalStack.Init();
	mcAnnotations.Init(128);
	mpcCurrentFile = NULL;
	mcHeadersStack.Init(8);

	mpcStack = pcStack;
	mcHeaderNames.Init(4);

	AddSpecialDefine("__DATE__");
	AddSpecialDefine("__FILE__");
	AddSpecialDefine("__LINE__");
	AddSpecialDefine("__TIME__");
	AddSpecialDefine("__TIMESTAMP__");

#ifdef __STDC__ 
	AddDefine("__STDC__", "1");
#else
	AddDefine("__STDC__", "0");
#endif

	InitPlatformSpecific();
	AddConfigDefines(pcConfig);
	mpcStack = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::Kill(void)
{
	mpcPost = NULL;
	mpcCurrentFile = NULL;
	mpcUnit = NULL;
	mpcStack = NULL;
	mcHeadersStack.Kill();
	mcConditionalStack.Kill();
	mcHeaderNames.Kill();
	mcAnnotations.Kill();
	mcDefines.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddIncludeDirectories(CArrayHeaderNameMap* pcHeaderNames)
{
	CHeaderNameMap*		pcHeaderNameMap;
	int					i;

	for (i = 0; i < pcHeaderNames->NumElements(); i++)
	{
		pcHeaderNameMap = pcHeaderNames->Get(i);
		AddIncludeDirectory(pcHeaderNameMap);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddIncludeDirectory(CHeaderNameMap* pcHeaderMapName)
{
	mcHeaderNames.Add(&pcHeaderMapName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::AddDefine(CExternalString* pcString)
{
	return mcDefines.AddDefine(pcString);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::GetDefine(CExternalString* pcString)
{
	return mcDefines.GetDefine(pcString);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::GetDefine(char* szName)
{
	return mcDefines.GetDefine(szName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::RemoveDefine(CExternalString* pcString)
{
	mcDefines.RemoveDefine(pcString);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::AddDefine(char* szDefine)
{
	return mcDefines.AddDefine(szDefine);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::AddSpecialDefine(char* szDefine)
{
	CDefine* pcDefine;

	pcDefine = AddDefine(szDefine);
	pcDefine->SetSpecial(TRUE);
	return pcDefine;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::AddDefine(char* szDefine, char* szReplacement)
{
	CDefine*				pcDefine;
	int						iLen;
	char*					sz;
	CPPText					pcText;
	CPreprocessorTokeniser	cTokeniser;

	iLen = (int)strlen(szReplacement);
	pcDefine = AddDefine(szDefine);
	sz = (char*)gcTokenStrings.Add(iLen+1);
	memcpy(sz, szReplacement, iLen+1);

	cTokeniser.Init();
	cTokeniser.TokeniseDefine(&pcDefine->mcReplacement.mcTokens, sz, mpcStack);
	cTokeniser.Kill();

	return pcDefine;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::AddDefine(CExternalString* pcString, CDefine* pcDefine)
{
	return mcDefines.AddDefine(pcString, pcDefine);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessHashDefine(CPreprocessorTokenParser* pcParser)
{
	CExternalString		cIdentifier;
	CExternalString		cArgument;
	BOOL				bResult;
	CDefine*			pcDefine;
	CPPText*			pcText;
	CPPReplacement*		pcReplacement;
	int					iReplaceArg;
	CPPToken*			pcToken;
	BOOL				bAllocated;
	BOOL				bAllowWhiteSpace;
	CDefine*			pcExisting;
	CDefine*			pcNew;
	CChars				sz;

	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		pcDefine = AddDefine(&cIdentifier);
		if (pcDefine)
		{
			pcExisting = NULL;
		}
		else
		{
			pcExisting = GetDefine(&cIdentifier);
			pcDefine = (CDefine*)malloc(sizeof(CDefine));
			pcDefine->Init(mcDefines.mcDefinesTree.GetIndexForNew(cIdentifier.msz, cIdentifier.miLen), mcDefines.muiID, NULL);
		}

		bResult = pcParser->GetExactDecorator('(', FALSE);
		if (bResult)
		{
			pcDefine->SetBracketed(TRUE);
			for (;;)
			{
				bResult = pcParser->GetExactDecorator(')');
				if (bResult)
				{
					break;
				}
				bResult = pcParser->GetIdentifier(&cArgument);
				if (bResult)
				{
					pcDefine->AddArgument(&cArgument);
					pcParser->GetExactDecorator(',');
				}
				else
				{
					//Expected an Identifier.
				}
			}
		}

		bAllowWhiteSpace = FALSE;
		while (pcParser->HasToken())
		{
			bAllocated = FALSE;
			pcToken = pcParser->GetToken();
			if (pcToken->IsText())
			{
				pcText = (CPPText*)pcToken;
				if (pcText->meType == PPT_Identifier)
				{
					iReplaceArg = pcDefine->mcArguments.GetIndex(pcText->mcText.msz, pcText->mcText.miLen);
					if (iReplaceArg != -1)
					{
						pcReplacement = CPPReplacement::Construct(mpcStack->Add(sizeof(CPPReplacement)));
						pcReplacement->Init(pcDefine->miIndex, iReplaceArg, -1, -1);
						pcToken = pcReplacement;
						bAllocated = TRUE;
					}
				}
			}

			if (!pcToken->IsWhiteSpace() || bAllowWhiteSpace)  //Stop leading whitespace from being added.
			{
				bAllowWhiteSpace = TRUE;
				if (!bAllocated)
				{
					pcToken = DuplicatePPToken(pcToken, mpcStack);
				}
				pcDefine->AddReplacmentToken(pcToken);
			}

			pcParser->NextToken();
		}

		bResult = TRUE;
		if (pcExisting)
		{
			if (!pcExisting->Equals(pcDefine))
			{
				//Removing a define will not change the index or the ID of our new define.  Phew.
				RemoveDefine(&cIdentifier);
				pcNew = AddDefine(&cIdentifier, pcDefine);
				if (!pcNew)
				{
					sz.Init("Could not redefine #define ");
					sz.AppendSubString(cIdentifier.msz, cIdentifier.EndInclusive()+1);
					sz.Append(".  Probably index or ID is different.");
					gcUserError.Set(sz.Text());
					sz.Kill();
					bResult = FALSE;
				}
			}
			else
			{
				//Only kill the define if the existing one is identical.
				//pcNew depends on the allocations in the non-identical define case.
				pcDefine->Kill();
			}
			free(pcDefine);
		}
	}
	else
	{
		gcUserError.Set("Could not get an identifier for a #define.  Add one.");
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ProcessHashUndef(CPreprocessorTokenParser* pcParser)
{
	BOOL				bResult;
	CExternalString		cIdentifier;

	pcParser->SkipWhiteSpace();
	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		mcDefines.RemoveDefine(&cIdentifier);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SCTokenBlock CPreprocessor::ProcessHashEndif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine)
{
	mcConditionalStack.PopEndIf();

	return Condition(pcCond, iLine);  //This must always return iLine.  It's impossible that it should stop parsing after an #end.
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessHashInclude(CPreprocessorTokenParser* pcParser)
{
	BOOL						bResult;
	CHeaderFile*				pcIncludeFile;
	CExternalString				cExternalString;
	CChars						sz;
	CHeaderNameMap*				pcHeaderNameMap;
	CHeaderNameMapDirectory*	pcNewDirectory;
	CHeaderNameMapDirectory*	pcCurrentDirectory;
	CChars						szPath;

	pcParser->SkipWhiteSpace();
	bResult = pcParser->GetStringDoubleQuoted(&cExternalString);
	if (!bResult)
	{
		bResult = pcParser->GetQuotedCharacterSequence('<', '>', &cExternalString);
		if (!bResult)
		{
			gcUserError.Set("Error in #include");
			return FALSE;
		}
	}

	FindBestInclude(&cExternalString, FALSE, &pcIncludeFile, &pcHeaderNameMap);
	if (pcIncludeFile)
	{
		pcNewDirectory = mcHeadersStack.Add();
		pcCurrentDirectory = mcHeadersStack.Get(mcHeadersStack.NumElements()-2);

		pcIncludeFile->Path(&szPath);
		pcNewDirectory->Init(pcHeaderNameMap, szPath.Text());
		szPath.Kill();

		bResult = PreprocessBlockSets(pcIncludeFile, mpcCurrentFile);

		pcNewDirectory = mcHeadersStack.Tail();
		pcNewDirectory->Kill();
		mcHeadersStack.Pop();
		return bResult;
	}

	sz.Init("Could not include file ");
	sz.AppendSubString(cExternalString.msz, cExternalString.EndInclusive()+1);
	gcUserError.Set(sz.Text());
	sz.Kill();
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::FindBestInclude(CExternalString* pcInclude, BOOL bSystemFile, CHeaderFile** ppcCFile, CHeaderNameMap** ppcHeaderNameMap)
{
	CChars						szInclude;
	CHeaderFile*				pcBestFile;
	CHeaderNameMap*				pcBestHeaderNameMap;
	int							iMatch;
	int							iBestMatch;
	int							i;
	CHeaderNameMap*				pcHeaderNameMap;
	CHeaderFile**				ppcHeaderFile;
	CHeaderFile*				pcHeaderFile;
	CHeaderNameMapDirectory*	pcCurrentDirectory;
	CChars						szPath;

	szInclude.Init(pcInclude->msz, 0, pcInclude->miLen);
	szInclude.Replace('\\', '/');

	iBestMatch = 0;
	pcBestFile = NULL;
	pcBestHeaderNameMap = NULL;

	for (i = 0; i < mcHeaderNames.NumElements(); i++)
	{
		pcHeaderNameMap = *mcHeaderNames.Get(i);

		ppcHeaderFile = pcHeaderNameMap->mcFileNames.GetWithKey(&szInclude);
		if (ppcHeaderFile)
		{
			pcHeaderFile = *ppcHeaderFile;
			iMatch = 2;

			if (pcHeaderFile->IsSystemFile() && bSystemFile)
			{
				iMatch += 2;
			}

			if (iMatch > iBestMatch)
			{
				iBestMatch = iMatch;
				pcBestFile = pcHeaderFile;
				pcBestHeaderNameMap = pcHeaderNameMap;
			}
		}
	}

	pcCurrentDirectory = mcHeadersStack.Tail();
	if (pcCurrentDirectory)
	{
		for (i = 0; i < mcHeaderNames.NumElements(); i++)
		{
			pcHeaderNameMap = *mcHeaderNames.Get(i);

			if (pcHeaderNameMap == pcCurrentDirectory->mpcHeaderNameMap)
			{
				if (pcCurrentDirectory->mszBaseDirectoryName.Length() > pcHeaderNameMap->mszBaseDirectory.Length())
				{
					szPath.Init(pcCurrentDirectory->mszBaseDirectoryName);
					szPath.RemoveFromStart(pcHeaderNameMap->mszBaseDirectory.Length()+1);
					szPath.Append('/');
					szPath.Append(szInclude);
					ppcHeaderFile = pcHeaderNameMap->mcFileNames.GetWithKey(&szPath);
					if (ppcHeaderFile)
					{
						pcHeaderFile = *ppcHeaderFile;
						iMatch = 2;

						if (pcHeaderFile->IsSystemFile() && bSystemFile)
						{
							iMatch += 2;
						}

						if (iMatch > iBestMatch)
						{
							iBestMatch = iMatch;
							pcBestFile = pcHeaderFile;
							pcBestHeaderNameMap = pcHeaderNameMap;
						}
					}

					szPath.Kill();
				}
			}
		}
	}

	szInclude.Kill();

	*ppcCFile = pcBestFile;
	*ppcHeaderNameMap = pcBestHeaderNameMap;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::LogIncludes(CCFile* pcFile)
{
	CChars			sz;

	if (mbLogInlucdes)
	{
		sz.Init();
		sz.Append(' ', miIncludeDepth*4);
		sz.Append(" ");
		sz.Append(pcFile->ShortName());
		sz.Append("\n");

		if (mbDumpLogs)
		{
			sz.Dump();
		}
		mpszIncludesLog->Append(sz);
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::LogBlocks(CCFile* pcFile, SCTokenBlock sResult)
{
	CChars			szLine;
	CCBlockSet*		pcBlocksSet;

	if (mbLogBlocks)
	{
		szLine.Init();

		szLine.Append(mpcCurrentFile->ShortName());
		if (mcConditionalStack.IsParsing())
		{
			szLine.Append(" (Parsing): ");
		}
		else
		{
			szLine.Append("          : ");
		}
		sResult.Append(&szLine);

		pcBlocksSet = pcFile->macBlockSets.SafeGet(sResult.iBlockIndex);
		if (!pcBlocksSet)
		{
			szLine.AppendNewLine();
		}
		else
		{
			szLine.Append(" (");
			szLine.Append(pcBlocksSet->miLine);
			szLine.Append(")\n");
		}

		if (mbDumpLogs)
		{
			szLine.Dump();
		}

		mpszBlocksLog->Append(szLine);
		szLine.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::LogIncludes(CChars* pszIncludesLog)
{
	if (pszIncludesLog)
	{
		mbLogInlucdes = TRUE;
		mpszIncludesLog = pszIncludesLog;
	}
	else
	{
		mbLogInlucdes = FALSE;
		mpszIncludesLog = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::LogBlocks(CChars* pszBlocksLog)
{
	if (pszBlocksLog)
	{
		mbLogBlocks = TRUE;
		mpszBlocksLog = pszBlocksLog;
	}
	else
	{
		mbLogInlucdes = FALSE;
		mbLogBlocks = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::LogDumping(BOOL bDumpLogs)
{
	mbDumpLogs = bDumpLogs;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::PreprocessBlockSets(CCFile* pcFile, CCFile* pcFromFile)
{
	CCBlockSet*			pcBlocksSet;
	SCTokenBlock		sResult;
	BOOL				bResult;

	miIncludeDepth++;
	LogIncludes(pcFile);
	mpcCurrentFile = pcFile;

	LoadFile(pcFile);

	bResult = TRUE;
	if (!mpcCurrentFile->IsPragmaOnced())
	{
		sResult.Init(0, 0);
		for (;;)
		{
			LogBlocks(pcFile, sResult);
			
			pcBlocksSet = pcFile->macBlockSets.SafeGet(sResult.iBlockIndex);
			if (!pcBlocksSet)
			{
				break;
			}

			if (pcBlocksSet->IsDirective())
			{
				//The conditional directives need to be expanded so &pcFile->mcStack is needed.  I suspect a #define directive will be expanded too.  I don't know if this is good or bad.  It's bad.  Write a test for it.
				sResult = PreprocessTokens(NULL, &pcFile->mcStack, &pcBlocksSet->mcRawTokens, sResult.iBlockIndex, sResult.iTokenIndex);
				if (sResult.iTokenIndex == -1)	
				{
					bResult = FALSE; 
					break;	
				}
			}
			else
			{
				CStackMarkExtended	cMark;
				CCBlock*			pcBlockProcessed;
				CCBlock*			pcBlockMatching;

				pcFile->mcStack.Mark(&cMark);

				pcBlockProcessed = pcBlocksSet->CreateBlock();
				sResult = PreprocessTokens(&pcBlockProcessed->mcTokens, &pcFile->mcStack, &pcBlocksSet->mcRawTokens, sResult.iBlockIndex, sResult.iTokenIndex);

				pcBlockMatching = pcBlocksSet->GetMatchingBlock(pcBlockProcessed);
				if (!pcBlockMatching)
				{	
					pcBlocksSet->AddBlock(pcBlockProcessed);
					pcBlockProcessed->SetNext(sResult.iTokenIndex, sResult.iBlockIndex);
					pcBlockMatching = pcBlockProcessed;
				}
				else
				{
					pcBlockProcessed->Kill();
					pcFile->mcStack.Rollback(&cMark);
					miBlockReuse++;
					sResult = pcBlockMatching->msNext;
				}
				cMark.Kill();

				if (sResult.iTokenIndex == -1)	
				{	
					bResult = FALSE;
					break;	
				}

				mpcUnit->mcTokens.mcTokens.mcArray.Add((CPPToken**)&pcBlockMatching);
			}

			pcBlocksSet = pcFile->macBlockSets.SafeGet(sResult.iBlockIndex);
			if (pcBlocksSet)
			{
				if (pcBlocksSet->mcRawTokens.mcArray.NumElements() == sResult.iTokenIndex)
				{
					sResult.iBlockIndex++;
					sResult.iTokenIndex = 0;
				}
			}
		}
	}

	mpcCurrentFile = pcFromFile;
	miIncludeDepth--;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::TranslationUnitLogging(CTranslationUnit* pcFile)
{
	if (pcFile->LogInlucdes())
	{
		LogIncludes(&pcFile->mpcLogs->szIncludesLog);
	}
	if (pcFile->LogBlocks())
	{
		LogBlocks(&pcFile->mpcLogs->szBlocksLog);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::PreprocessTranslationUnit(CTranslationUnit* pcFile)
{
	BOOL bResult;
	
	TranslationUnitLogging(pcFile);
	mpcUnit = pcFile;
	bResult = PreprocessBlockSets(pcFile, NULL);
	mpcUnit = NULL;
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::LoadFile(CCFile* pcFile)
{
	CPreprocessorTokeniser		cTokeniser;

	pcFile->Load();
	cTokeniser.Init();

	cTokeniser.Tokenise(&pcFile->macBlockSets, pcFile->mszContents.Text(), pcFile->mszContents.Length(), !pcFile->IsSystemFile());
	cTokeniser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SCTokenBlock CPreprocessor::ProcessHashIfndef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine)
{
	CExternalString		cIdentifier;
	CDefine*			pcDefine;
	BOOL				bResult;
	BOOL				bEvaluated;

	pcParser->SkipWhiteSpace();
	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		pcDefine = mcDefines.GetDefine(&cIdentifier);
		bEvaluated = (pcDefine == NULL);
		mcConditionalStack.PushIfNotDefined(bEvaluated);
	}	
	else
	{
		mcConditionalStack.PushIfNotDefined(TRUE);	
	}

	return Condition(pcCond, iLine);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SCTokenBlock CPreprocessor::ProcessHashIfdef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine)
{
	CExternalString		cIdentifier;
	CDefine*			pcDefine;
	BOOL				bResult;
	BOOL				bEvaluated;

	pcParser->SkipWhiteSpace();
	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		pcDefine = mcDefines.GetDefine(&cIdentifier);
		bEvaluated = (pcDefine != NULL);
		mcConditionalStack.PushIfDefined(bEvaluated);
	}	
	else
	{
		mcConditionalStack.PushIfDefined(TRUE);	
	}

	return Condition(pcCond, iLine);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SCTokenBlock CPreprocessor::Condition(CPPConditional* pcCond, SCTokenBlock iLine)
{
	CPPConditional*	pcNext;
	SCTokenBlock	sIndex;

	if (mcConditionalStack.IsParsing())
	{
		iLine.iTokenIndex++;
		return iLine;
	}
	else
	{
		pcNext = pcCond->mpcNext;
		if (pcNext)
		{
			sIndex.iBlockIndex = pcNext->msIndex.iBlockIndex;
			sIndex.iTokenIndex = pcNext->msIndex.iTokenIndex;
			return sIndex;
		}
		else if (pcCond->meType == PPD_endif)
		{
			sIndex.iBlockIndex = pcCond->msIndex.iBlockIndex;
			sIndex.iTokenIndex = -1;
			return sIndex;
		}
		else
		{
			gcUserError.Set("Missing #endif");
			sIndex.iBlockIndex = -1;
			sIndex.iTokenIndex = -1;
			return sIndex;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SCTokenBlock CPreprocessor::ProcessHashElse(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine)
{
	mcConditionalStack.SwapForElse();

	return Condition(pcCond, iLine);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SCTokenBlock CPreprocessor::ProcessHashIf(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine)
{
	BOOL			bEvaluated;
	CPPTokenHolder	cTokenHolder;
	CChars			sz;

	cTokenHolder.Init(32);
	ProcessLine(&cTokenHolder, pcParser, TRUE, 0);
	sz.Init(128);
	cTokenHolder.Append(&sz);
	bEvaluated = Evaluate(sz.Text());;
	mcConditionalStack.PushIf(bEvaluated);
	sz.Kill();
	cTokenHolder.Kill();

	return Condition(pcCond, iLine);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SCTokenBlock CPreprocessor::ProcessHashElif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine)
{
	BOOL			bEvaluated;
	CPPTokenHolder	cTokenHolder;
	CChars			sz;

	cTokenHolder.Init(32);
	ProcessLine(&cTokenHolder, pcParser, TRUE, 0);
	sz.Init(128);
	cTokenHolder.Append(&sz);
	bEvaluated = Evaluate(sz.Text());;
	mcConditionalStack.SwapForElseIf(bEvaluated);
	sz.Kill();
	cTokenHolder.Kill();

	return Condition(pcCond, iLine);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ProcessHashError(CPreprocessorTokenParser* pcParser)
{
	CChars			szError;
	CPPToken*		pcToken;

	pcParser->SkipWhiteSpace();
	szError.Init("Error (");
	szError.Append(mpcCurrentFile->ShortName());
	szError.Append("):");

	while (pcParser->HasToken())
	{
		pcToken = pcParser->GetToken();
		pcToken->Append(&szError);
		pcParser->NextToken();
	}		
	szError.AppendNewLine();
	szError.Dump();
	szError.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessHashPragma(CPreprocessorTokenParser* pcParser)
{
	BOOL			bResult;
	CHeaderFile*	pcHeader;

	pcParser->SkipWhiteSpace();
	bResult = pcParser->GetExactIdentifier("once", TRUE, TRUE);
	if (bResult == TRITRUE)
	{
		if (mpcCurrentFile->IsHeader())
		{
			pcHeader = (CHeaderFile*)mpcCurrentFile;
			pcHeader->mbPragmaOnce = TRUE;
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessNormalLine(CPreprocessorTokenParser* pcParser)
{
	CPPLine*	pcLine;
	BOOL		bResult;

	if (pcParser->HasTokens())
	{
		pcLine = CPPLine::Construct(mpcStack->Add(sizeof(CPPLine)));
		pcLine->Init(pcParser->Line(), pcParser->Column());
		bResult = ProcessLine(&pcLine->mcTokens, pcParser, FALSE, 0);
		if (bResult)
		{
			if (pcLine->TokenLength() > 0)
			{
				mpcPost->Add((CPPToken**)&pcLine);
			}
			return TRUE;
		}
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessSingleHash(CPPTokenHolder* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessDoubleHash(CPPTokenHolder* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, BOOL bAllowDefined, int iDepth)
{
	CPPToken*				pcToken;
	CDefine*				pcDefine;
	BOOL					bResult;
	CPPAbstractHolder*				pcHolder;
	CPPText*				pcDecorator;
	char*					pcValue;
	int						i;
	CPPTokenHolder*			pcTokenHolder;
	SDefineArgument*		psArguments;
	int						iArgIndex;

	pcDefine = mcDefines.GetDefine(&pcText->mcText);
	if (pcDefine)
	{
		iArgIndex = -1;
		if (pcDefine->IsBacketed())
		{
			pcParser->NextToken();
			psArguments = mcArguments.Add(pcDefine->miIndex);
			iArgIndex = mcArguments.mcDefineToArguments.GetIndex(psArguments);
			bResult = FindArguments(pcParser, &psArguments->mcArguments);
			if ((!bResult) || (psArguments->mcArguments.NumElements() != pcDefine->mcArguments.NumElements()))
			{
				//Expected arguments but there weren't any.
				for (i = 0; i < psArguments->mcArguments.NumElements(); i++)
				{
					pcTokenHolder = psArguments->mcArguments.Get(i);
					pcTokenHolder->Kill();
				}
				mcArguments.Remove(pcDefine->miIndex);
				return FALSE;
			}
		}
		else
		{
			pcParser->NextToken();
		}

		if (pcDefine->mcReplacement.mcTokens.mcArray.NumElements() > 0)
		{
			pcHolder = ADD_TOKEN(CPPHolder, &pcDest->mcArray, mpcStack->Add(sizeof(CPPHolder)));
			pcHolder->Init(4, -1, -1);
			ExpandDefined(pcHolder, pcDefine, bAllowDefined, iDepth+1);
		}

		//I'm not sure if it's safe to do this anymore... another define might refer to it.
		if (iArgIndex != -1)
		{
			psArguments = mcArguments.Get(pcDefine->miIndex);
			for (i = 0; i < psArguments->mcArguments.NumElements(); i++)
			{
				pcTokenHolder = psArguments->mcArguments.Get(i);
				pcTokenHolder->Kill();
			}
			mcArguments.Remove(pcDefine->miIndex);
		}
		return TRUE;
	}
	else if (bAllowDefined)
	{
		bResult = pcParser->GetExactIdentifier("defined", TRUE, TRUE);
		if (bResult)
		{
			pcParser->GetExactDecorator('(');
			pcParser->SkipWhiteSpace();
			pcToken = pcParser->GetToken();
			pcDecorator = CPPText::Construct(mpcStack->Add(sizeof(CPPText)));
			pcValue = (char*)gcTokenStrings.Add(1);
			*pcValue = '0';
			pcDecorator->Init(PPT_Number, -1, -1, pcValue, pcValue+1);
			if (pcToken)
			{
				if (pcToken->IsText())
				{
					pcText = (CPPText*)pcToken;
					if (pcText->meType == PPT_Identifier)
					{
						pcDefine = mcDefines.GetDefine(&pcText->mcText);
						pcParser->NextToken();
						if (pcDefine)
						{
							*pcValue = '1';
						}
					}
				}
				pcDest->Add((CPPToken**)&pcDecorator);
				pcParser->SkipWhiteSpace();
				pcParser->GetExactDecorator(')');
			}
			return TRUE;
		}
	}

	pcToken = DuplicatePPToken(pcText, mpcStack);
	pcDest->Add(&pcToken);
	pcParser->NextToken();
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ExpandTokenIfNecessary(CPPToken* pcToken, CPPTokenHolder* pcDest, CPreprocessorTokenParser* pcParser, BOOL bAllowDefined, int iDepth)
{		
	CPPToken*				pcNewToken;
	CPPText*				pcText;
	BOOL					bResult;
	CPPReplacement*			pcReplacement;

	if (pcToken->IsText())
	{
		pcText = (CPPText*)pcToken;
		if (pcText->meType == PPT_Identifier)
		{
			bResult = ProcessIdentifier(pcDest, pcText, pcParser, bAllowDefined, iDepth);
			return bResult;
		}
		else
		{
			pcNewToken = DuplicatePPToken(pcToken, mpcStack);
			pcDest->Add(&pcNewToken);
			pcParser->NextToken();
			return TRUE;
		}
	}
	else if (pcToken->IsReplacement())
	{
		pcReplacement = (CPPReplacement*)pcToken;
		ExpandReplacement(pcReplacement, pcDest, bAllowDefined, iDepth);
		pcParser->NextToken();
		return TRUE;
	}
	else
	{
		pcNewToken = DuplicatePPToken(pcToken, mpcStack);
		pcDest->Add(&pcNewToken);
		pcParser->NextToken();
		return TRUE;
	}
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ExpandReplacement(CPPReplacement* pcReplacement, CPPTokenHolder* pcDest, BOOL bAllowDefined, int iDepth)
{
	CArrayPPTokenHolders*		pcArguments;
	CPPTokenHolder*				pcArgument;
	CPreprocessorTokenParser	cParser;
	CPPLine						cLine;
	SDefineArgument*			psDefineArgument;

	psDefineArgument = mcArguments.Get(pcReplacement->miDefineIndex);
	pcArguments = &psDefineArgument->mcArguments;
	if (pcArguments)
	{
		pcArgument = pcArguments->Get(pcReplacement->miArgIndex);
		if (pcArgument)
		{
			cLine.Fake(pcArgument);
			cParser.Init(&cLine);
			ProcessLine(pcDest, &cParser, bAllowDefined, iDepth);
			cParser.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessLine(CPPTokenHolder* pcDest, CPreprocessorTokenParser* pcParser, BOOL bAllowDefined, int iDepth)
{
	CPPHashes*				pcHash;
	CPPHolder				cHolder;
	int						iHashCount;
	CPPToken*				pcTemp;
	CPPToken*				pcPrev;
	CPPToken*				pcToken;

	iHashCount = 0;
	while (pcParser->HasToken())
	{
		pcToken = pcParser->GetToken();
		if (pcToken->IsHash())
		{
			pcHash = (CPPHashes*)pcToken;
			iHashCount = pcHash->miCount;
			pcParser->NextToken();
			continue;
		}

		if (iHashCount == 0)
		{
			pcTemp = pcToken;
			ExpandTokenIfNecessary(pcToken, pcDest, pcParser, bAllowDefined, iDepth);
		}
		else if (iHashCount == 1)  //# Quote following.
		{
			cHolder.Init(100, -1, -1);
			ExpandTokenIfNecessary(pcToken, &cHolder.mcTokens, pcParser, bAllowDefined, iDepth);
			pcTemp = QuoteTokens(pcDest, &cHolder);
			cHolder.Kill();
		}
		else if (iHashCount == 2)  //## Concaternate.
		{
			pcTemp = ConcaternateTokens(pcDest, pcPrev, pcToken);
		}

		pcPrev = pcTemp;
		iHashCount = 0;
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessor::ConcaternateTokens(CPPTokenHolder* pcDest, CPPToken* pcLeft, CPPToken* pcRight)
{
	CPPText*	pcLeftText;
	CPPText*	pcRightText;
	CChars		szConcaternated;
	char*		szInStrings;

	if (pcLeft->IsText() && pcRight->IsText())
	{
		pcLeftText = (CPPText*)pcLeft;
		pcRightText = (CPPText*)pcRight;

		if (((pcLeftText->meType == PPT_Identifier) && (pcRightText->meType == PPT_Identifier))  ||
			((pcLeftText->meType == PPT_Number) && (pcRightText->meType == PPT_Number)))
		{
			szConcaternated.Init(100);
			pcLeftText->Append(&szConcaternated);
			pcRightText->Append(&szConcaternated);

			szInStrings = (char*)gcTokenStrings.Add(szConcaternated.Length()+1);
			memcpy(szInStrings, szConcaternated.Text(), szConcaternated.Length()+1);

			pcLeftText->mcText.msz = szInStrings;
			pcLeftText->mcText.miLen = szConcaternated.Length();

			szConcaternated.Kill();
			return pcLeft;
		}
		else
		{
			pcDest->Add(&pcRight);
			return pcRight;
		}
	}
	return pcLeft;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessor::QuoteTokens(CPPTokenHolder* pcDest, CPPAbstractHolder* pcHolder)
{
	int						j;
	BOOL					bQuoteNextReplacement;
	CChars					szQuoted;
	char*					szInStrings;
	CPPText*				pcQuoted;
	CPPToken*				pcToken;

	szQuoted.Init(100);
	szQuoted.Append('"');
	for (j = 0; j < pcHolder->mcTokens.mcArray.NumElements(); j++)
	{
		pcToken = *(pcHolder->mcTokens.mcArray.Get(j));
		pcToken->Append(&szQuoted);
	}
	szQuoted.Append('"');
	bQuoteNextReplacement = FALSE;

	szInStrings = (char*)gcTokenStrings.Add(szQuoted.Length()+1);
	memcpy(szInStrings, szQuoted.Text(), szQuoted.Length()+1);
	pcQuoted = CPPText::Construct(mpcStack->Add(sizeof(CPPText)));
	pcQuoted->Init(PPT_DoubleQuoted, -1, -1, szInStrings, szInStrings+szQuoted.Length());
	pcDest->Add((CPPToken**)&pcQuoted);

	szQuoted.Kill();

	return pcQuoted;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ExpandDefined(CPPAbstractHolder* pcHolder, CDefine* pcDefine, BOOL bAllowDefined, int iDepth)
{
	CPreprocessorTokenParser	cParser;

	cParser.Init(&pcDefine->mcReplacement);
	ProcessLine(&pcHolder->mcTokens, &cParser, bAllowDefined, iDepth);
	cParser.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessor::AddToken(CPPToken* pcToken, CArrayPPTokens* pcTokens)
{
	CPPToken**	ppcToken;

	ppcToken = pcTokens->Add();
	*ppcToken = pcToken;
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::FindArguments(CPreprocessorTokenParser* pcParser, CArrayPPTokenHolders* pacArguments)
{
	CPPTokenHolder*		pcArgument;
	BOOL				bResult;
	int					iBracketDepth;
	CPPToken*			pcSomeToken;
	BOOL				bReturn;

	bResult = pcParser->GetExactDecorator('(');
	if (!bResult)
	{
		//Expected (
		return FALSE;
	}

	iBracketDepth = 1;
	pcArgument = pacArguments->Add();
	pcArgument->Init(10);
	bReturn = FALSE;
	for (;;)
	{
		pcSomeToken = pcParser->GetToken();
		if (pcSomeToken == NULL)
		{
			//Out of file...
			if (iBracketDepth == 1)
			{
				bReturn = TRUE;
			}
			break;
		}
		bResult = pcParser->GetExactDecorator(')', FALSE);
		if (bResult)
		{
			if (iBracketDepth == 1)
			{
				bReturn = TRUE;
				break;
			}
			else 
			{
				AddTokenToArgument(pcArgument, pcSomeToken);
				iBracketDepth--;
				continue;
			}

		}

		bResult = pcParser->GetExactDecorator('(', FALSE);
		if (bResult)
		{
			AddTokenToArgument(pcArgument, pcSomeToken);
			iBracketDepth++;
			continue;
		}

		bResult = pcParser->GetExactDecorator(',', FALSE);
		if (bResult)
		{
			if (iBracketDepth > 1)
			{
				AddTokenToArgument(pcArgument, pcSomeToken);
				continue;
			}
			else
			{
				pcParser->SkipWhiteSpace();
				pcArgument = pacArguments->Add();
				pcArgument->Init(10);
				continue;
			}
		}

		AddTokenToArgument(pcArgument, pcSomeToken);
		pcParser->NextToken();
	}

	if (pcArgument->mcArray.NumElements() == 0)
	{
		pcArgument->Kill();
		pacArguments->RemoveTail();
	}

	//We're done!  Yay.
	return bReturn;

}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddTokenToArgument(CPPTokenHolder* pcArgument, CPPToken* pcToken)
{
	pcToken = DuplicatePPToken(pcToken, mpcStack);
	pcArgument->Add(&pcToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessAnnotation(CPreprocessorTokenParser* pcParser)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ProcessAnnotation(CTextParser* pcLineParser, int iSourceLine)
{
	//CPreprocessorAnnotation*	pcPreprocessorAnnotation;
	//char*			szStart;
	//int				iLen;

	//pcPreprocessorAnnotation = mcAnnotations.Add();
	//pcPreprocessorAnnotation->Init();
	//pcPreprocessorAnnotation->msSource.pcSourceFile = mpcCurrentFile;
	//pcPreprocessorAnnotation->msSource.iLine = iSourceLine;
	//pcPreprocessorAnnotation->msSource.iColumn = 0;
	//pcPreprocessorAnnotation->miStartOfLine = mszPost.Length();
	//szStart = pcLineParser->mszParserPos;
	//pcLineParser->FindEndOfLine();
	//iLen = (int)(pcLineParser->mszParserPos - szStart)+1;
	//pcPreprocessorAnnotation->mszText.Append(szStart, iLen);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SCTokenBlock CPreprocessor::PreprocessTokens(CPPTokenHolder* pcDestTokens, CMemoryStackExtended* pcStack, CPPTokenHolder* pcSourceTokens, int iBlock, int iToken)
{
	SCTokenBlock				sLine;
	int							iNumLines;
	CPPToken*					pcToken;
	CPPDirective*				pcDirective;
	CPPLine*					pcLine;
	CPreprocessorTokenParser	cParser;
	BOOL						bResult;
	int							iOldLine;

	mpcPost = pcDestTokens;
	mpcStack = pcStack;

	mcArguments.Init();

	bResult = TRUE;
	iNumLines = pcSourceTokens->mcArray.NumElements();
	sLine.iBlockIndex = iBlock;
	for (sLine.iTokenIndex = iToken; sLine.iTokenIndex < iNumLines; )
	{
		iOldLine = sLine.iTokenIndex;
		pcToken = *(pcSourceTokens->mcArray.Get(sLine.iTokenIndex));
		if (pcToken->IsDirective())
		{
			pcDirective = (CPPDirective*)pcToken;
			cParser.Init(pcDirective);
			if (pcDirective->meType <= PPD_elif)
			{
				switch (pcDirective->meType)
				{
				case PPD_ifdef:
					sLine = ProcessHashIfdef(&cParser, (CPPConditional*)pcDirective, sLine);
					break;
				case PPD_ifndef:
					sLine = ProcessHashIfndef(&cParser, (CPPConditional*)pcDirective, sLine);
					break;
				case PPD_endif:
					sLine = ProcessHashEndif(&cParser, (CPPConditional*)pcDirective, sLine);
					break;
				case PPD_else:
					sLine = ProcessHashElse(&cParser, (CPPConditional*)pcDirective, sLine);
					break;
				case PPD_if:
					sLine = ProcessHashIf(&cParser, (CPPConditional*)pcDirective, sLine);
					break;
				case PPD_elif:
					sLine = ProcessHashElif(&cParser, (CPPConditional*)pcDirective, sLine);
					break;
				}
			}
			else
			{
				if (mcConditionalStack.IsParsing())
				{
					switch (pcDirective->meType)
					{
					case PPD_define:
						bResult = ProcessHashDefine(&cParser);
						break;
					case PPD_include:
						bResult = ProcessHashInclude(&cParser);
						break;
					case PPD_undef:
						ProcessHashUndef(&cParser);
						break;
					case PPD_error:
						ProcessHashError(&cParser);
						bResult = FALSE;
						break;
					case PPD_pragma:
						bResult = ProcessHashPragma(&cParser);
						break;
					}
				}
				sLine.iTokenIndex++;
			}

			if (((sLine.iTokenIndex >= 0) && (sLine.iTokenIndex < iOldLine)) && (sLine.iBlockIndex == iBlock))
			{
				gcUserError.Set("Parsing went backwards.  WTF!");
				bResult = FALSE;
			}
			cParser.Kill();

			if (((sLine.iTokenIndex == -1) && (sLine.iBlockIndex == -1)) || (!bResult))
			{
				sLine.Init(-1, -1);
				break;
			}

			if (sLine.iTokenIndex == -1)
			{
				//This can only happen on an #endif
				sLine.iTokenIndex = iNumLines;
				sLine.iBlockIndex = iBlock;
				break;
			}
			else if (sLine.iBlockIndex != iBlock)
			{
				break;
			}
		}
		else if (pcToken->IsLine())
		{
			if (mcConditionalStack.IsParsing())
			{
				pcLine = (CPPLine*)pcToken;
				cParser.Init(pcLine);
				ProcessNormalLine(&cParser);
				cParser.Kill();
			}
			sLine.iTokenIndex++;
		}
		else
		{
			bResult = FALSE;
			gcUserError.Set("Got a token which wasn't a line");
			sLine.Init(-1, -1);
			break;
		}
	}

	mcArguments.Kill();

	//Seriously, do something about this.  They should be passed as parameters, not global variables.
	//mpcPost = NULL;
	//mpcStack = NULL;
	return sLine;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::Preprocess(char* szSource, CChars* szDest)
{
	CPreprocessor				cPreprocessor;
	CPreprocessorTokeniser		cTokeniser;
	int							iLen;
	CMemoryStackExtended		cStack;
	CPPTokenHolder				cRawTokens;
	CPPTokenHolder				cProcessedTokens;

	cTokeniser.Init();
	cStack.Init(4 KB);
	cRawTokens.Init(256);
	iLen = (int)strlen(szSource);
	cTokeniser.Tokenise(&cRawTokens, &cStack, szSource, iLen, TRUE, 0, 0);
	cTokeniser.Kill();

	cProcessedTokens.Init(256);
	cPreprocessor.Init(NULL, &cStack);
	cPreprocessor.PreprocessTokens(&cProcessedTokens, &cStack, &cRawTokens, 0, 0);

	szDest->Init(1024);
	cProcessedTokens.Append(szDest);

	cProcessedTokens.Kill();
	cRawTokens.Kill();
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::Evaluate(char* szText)
{
	CCalculator	cCalculator;
	CNumber		cAnswer;

	cCalculator.Init();
	cAnswer = cCalculator.Eval(szText);
	cCalculator.Kill();

	if (cAnswer.IsZero())
		return FALSE;
	else
		return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddConfigDefines(CConfig* pcConfig)
{
	int			i;
	CChars*		pszText;

	if (pcConfig)
	{
		for (i = 0; i < pcConfig->maszDefines.NumElements(); i++)
		{
			pszText = pcConfig->maszDefines.Get(i);
			AddDefine(pszText->Text());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::DeltaDefines(CArrayNamedDefines* pcDelta, CMemoryStackExtended* pcStack)
{
	int					i;
	CNamedDefine*		pcNamedDefine;
	CDefine*			pcExisting;
	CDefine*			pcDefine;
	CDefine*			pcNew;
	BOOL				bResult;
	CExternalString		cIdentifier;
	CChars				sz;

	for (i = 0; i < pcDelta->NumElements(); i++)
	{
		pcNamedDefine = pcDelta->Get(i);
		cIdentifier.Init(pcNamedDefine->mszName.Text(), pcNamedDefine->mszName.Length());
		if (pcNamedDefine->miFlags & NAMED_DEFINE_FLAGS_UNDEFFED)
		{
			RemoveDefine(&cIdentifier);
		}
		else
		{
			pcDefine = AddDefine(&cIdentifier);
			if (pcDefine)
			{
				pcExisting = NULL;
			}
			else
			{
				pcExisting = GetDefine(pcNamedDefine->mszName.Text());
				pcDefine = (CDefine*)malloc(sizeof(CDefine));
				pcDefine->Init(mcDefines.mcDefinesTree.GetIndexForNew(cIdentifier.msz, cIdentifier.miLen), mcDefines.muiID, NULL);
			}

			pcDefine->mcArguments.Copy(&pcNamedDefine->mcArguments);
			pcDefine->mcReplacement.Copy(&pcNamedDefine->mcReplacement, pcStack);
			pcDefine->SetBracketed(pcNamedDefine->IsBacketed());

			bResult = TRUE;
			if (pcExisting)
			{
				if (!pcExisting->Equals(pcDefine))
				{
					//Removing a define will not change the index or the ID of our new define.  Phew.
					RemoveDefine(&cIdentifier);
					pcNew = AddDefine(&cIdentifier, pcDefine);
					if (!pcNew)
					{
						sz.Init("Could not redefine #define ");
						sz.AppendSubString(cIdentifier.msz, cIdentifier.EndInclusive()+1);
						sz.Append(".  Probably index or ID is different.");
						gcUserError.Set(sz.Text());
						sz.Kill();
						bResult = FALSE;
					}
				}
				else
				{
					//Only kill the define if the existing one is identical.
					//pcNew depends on the allocations in the non-identical define case.
					pcDefine->Kill();
				}
				free(pcDefine);
			}
		}
	}
}

