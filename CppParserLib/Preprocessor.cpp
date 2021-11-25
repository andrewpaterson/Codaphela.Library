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
#include "PreprocessorPosition.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::Init(CConfig* pcConfig, CMemoryStackExtended* pcStack)
{
	mcDefines.Init();
	mcSpecialOperators.Init();
	mpcPost = NULL;
	mpcUnit = NULL;
	miIncludeDepth = 0;
	miBlockReuse = 0;
	mbLogBlocks = FALSE;
	mbLogInlucdes = FALSE;
	mbDumpLogs = FALSE;
	miDefineReuse = 0;

	mcConditionalStack.Init();
	mpcCurrentFile = NULL;
	mpcCurrentDirective = NULL;
	mpcCurrentLineParser = NULL;
	mcHeadersStack.Init();

	mpcStack = pcStack;
	mcHeaderNames.Init();

	AddSpecialDefine("__DATE__");
	AddSpecialDefine("__TIME__");
	AddSpecialDefine("__FILE__");
	AddSpecialDefine("__LINE__");
	AddSpecialDefine("__TIMESTAMP__");
	AddSpecialDefine("__FUNCTION__");
	AddSpecialDefine("__func__");
	AddSpecialDefine("__STDC__");
	AddSpecialDefine("__STDC_VERSION__");
	AddSpecialDefine("__STDC_HOSTED__");
	AddSpecialDefine("__cplusplus");
	AddSpecialDefine("__OBJC__");
	AddSpecialDefine("__ASSEMBLER__");


	AddSpecialOperator("defined", PPSO_Defined);
	AddSpecialOperator("__has_attribute", PPSO_HasAttribute);
	AddSpecialOperator("__has_cpp_attribute", PPSO_HasCppAttribute);
	AddSpecialOperator("__has_builtin", PPSO_HasBuiltIn);
	AddSpecialOperator("__has_include", PPSO_HasInclude);

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
	mcSpecialOperators.Kill();
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
CDefine* CPreprocessor::GetDefine(CExternalString* pcString, BOOL bExact)
{
	return mcDefines.GetDefine(pcString, bExact);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::GetDefine(char* szName, BOOL bExact)
{
	return mcDefines.GetDefine(szName, bExact);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CPreprocessor::GetSpecialOperator(CExternalString* pcString, BOOL bExact)
{
	return mcSpecialOperators.GetSpecialOperator(pcString , bExact);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CPreprocessor::GetSpecialOperator(char* szName, BOOL bExact)
{
	return mcSpecialOperators.GetSpecialOperator(szName, bExact);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::AddSpecialDefine(char* szDefine)
{
	CDefine* pcDefine;

	pcDefine = mcDefines.AddDefine(szDefine);
	if (pcDefine)
	{
		pcDefine->SetSpecial(TRUE);
		return pcDefine;
	}
	else
	{
		return NULL;
	}
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
CSpecialOperator* CPreprocessor::AddSpecialOperator(char* szSpecialOperator, EPreprocessorSpecialOperator eType)
{
	return mcSpecialOperators.AddSpecialOperator(szSpecialOperator, eType);
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
	pcDefine = mcDefines.AddDefine(szDefine);
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

	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		pcExisting = mcDefines.GetDefine(&cIdentifier, TRUE);
		if (pcExisting)
		{
			mcDefines.RemoveDefine(&cIdentifier);
		}
		pcDefine = mcDefines.AddDefine(&cIdentifier);

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
					bResult = pcParser->GetExactDecorator("...");
					if (bResult)
					{
						pcDefine->AddVariadic();
					}
					else
					{
						return gcUserError.Set("Expected an Identifier.");
					}
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
						pcReplacement->Init(pcDefine->GetID(), iReplaceArg, -1, -1);
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
		return TRUE;
	}
	else
	{
		return gcUserError.Set("Could not get an identifier for a #define.  Add one.");
	}
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
	CHeaderNameMap*				pcHeaderNameMap;
	CExternalString				cExternalString;
	CChars						sz;
	CHeaderNameMapDirectory*	pcNewDirectory;
	CHeaderNameMapDirectory*	pcCurrentDirectory;
	CChars						szPath;
	SPreprocessorPosition		sPos;

	pcParser->SkipWhiteSpace();

	MarkPositionForError(&sPos);

	bResult = ProcessIncludeFile(pcParser, &pcIncludeFile, &pcHeaderNameMap);
	if (bResult)
	{
		if (pcIncludeFile)
		{
			pcNewDirectory = mcHeadersStack.Add();
			pcCurrentDirectory = mcHeadersStack.Get(mcHeadersStack.NumElements() - 2);

			pcIncludeFile->Path(&szPath);
			pcNewDirectory->Init(pcHeaderNameMap, szPath.Text());
			szPath.Kill();

			bResult = PreprocessBlockSets(pcIncludeFile, mpcCurrentFile);

			pcNewDirectory = mcHeadersStack.Tail();
			pcNewDirectory->Kill();
			mcHeadersStack.Pop();
			return bResult;
		}
		else
		{
			sPos.Message(&sz);
			sz.Append("Could not include file ");
			sz.AppendSubString(cExternalString.msz, cExternalString.EndInclusive() + 1);
			return gcUserError.Set(&sz);
		}
	}
	else
	{
		return FALSE;
	}

}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessIncludeFile(CPreprocessorTokenParser* pcParser, CHeaderFile** ppcCFile, CHeaderNameMap** ppcHeaderNameMap)
{
	BOOL						bResult;
	CExternalString				cExternalString;
	SPreprocessorPosition		sPos;
	CChars						sz;

	MarkPositionForError(&sPos);

	bResult = pcParser->GetStringDoubleQuoted(&cExternalString);
	if (!bResult)
	{
		bResult = pcParser->GetQuotedCharacterSequence('<', '>', &cExternalString);
		if (!bResult)
		{
			sPos.Message(&sz);
			sz.Append("Could not include file ");
			sz.AppendSubString(cExternalString.msz, cExternalString.EndInclusive() + 1);
			return gcUserError.Set(&sz);
		}
	}

	FindBestInclude(&cExternalString, FALSE, ppcCFile, ppcHeaderNameMap);
	return TRUE;
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

		ppcHeaderFile = pcHeaderNameMap->mcFileNames.Get(szInclude.Text());
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
					ppcHeaderFile = pcHeaderNameMap->mcFileNames.Get(szPath.Text());
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

	bResult = TokeniseFile(pcFile);
	if (!bResult)
	{
		return FALSE;
	}

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
BOOL CPreprocessor::TokeniseFile(CCFile* pcFile)
{
	CPreprocessorTokeniser	cTokeniser;
	BOOL					bResult;

	pcFile->Load();
	cTokeniser.Init();

	bResult = cTokeniser.Tokenise(&pcFile->macBlockSets, pcFile->mszContents.Text(), pcFile->mszContents.Length(), !pcFile->IsSystemFile());
	cTokeniser.Kill();

	return bResult;
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
		pcDefine = mcDefines.GetDefine(&cIdentifier, TRUE);
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
		pcDefine = mcDefines.GetDefine(&cIdentifier, TRUE);
		bEvaluated = (pcDefine != NULL);
		mcConditionalStack.PushIfDefined(bEvaluated);
	}	
	else
	{
		mcConditionalStack.PushIfDefined(FALSE);	
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
	TRISTATE				sEvaluated;
	CPPTokenHolder			cTokenHolder;
	CChars					sz;
	SCTokenBlock			sIndex;
	SPreprocessorPosition	sPos;
	CChars					szCaclulatorError;
	CChars					szError;

	MarkPositionForError(&sPos);

	cTokenHolder.Init();
	ProcessLine(&cTokenHolder, pcParser, TRUE, 0);
	sz.Init();
	cTokenHolder.Append(&sz);
	szCaclulatorError.Init();
	sEvaluated = EvaluateEquation(sz.Text(), &szCaclulatorError);
	if (sEvaluated != TRIERROR)
	{
		szCaclulatorError.Kill();
		mcConditionalStack.PushIf(sEvaluated == TRITRUE);
		sz.Kill();
		cTokenHolder.Kill();

		return Condition(pcCond, iLine);
	}
	else
	{
		cTokenHolder.Kill();

		sPos.Message(&szError);
		szError.Append("Calculator error parsing \"");
		szError.Append(sz);
		szError.Append("\".  ");
		sz.Kill();
		szError.Append(szCaclulatorError);
		szCaclulatorError.Kill();
		gcUserError.Set(szError.Text());
		szError.Kill();

		sIndex.Init(-1, -1);  //-1, -1 doesn't seem to strictly mean an error but it does mean stop.
		return sIndex;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SCTokenBlock CPreprocessor::ProcessHashElif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine)
{
	TRISTATE				sEvaluated;
	CPPTokenHolder			cTokenHolder;
	CChars					sz;
	SCTokenBlock			sIndex;
	SPreprocessorPosition	sPos;
	CChars					szCaclulatorError;
	CChars					szError;

	MarkPositionForError(&sPos);

	cTokenHolder.Init();
	szCaclulatorError.Init();
	ProcessLine(&cTokenHolder, pcParser, TRUE, 0);
	sz.Init();
	cTokenHolder.Append(&sz);
	sEvaluated = EvaluateEquation(sz.Text(), &szCaclulatorError);;
	if (sEvaluated != TRIERROR)
	{
		szCaclulatorError.Kill();
		mcConditionalStack.SwapForElseIf(sEvaluated == TRITRUE);
		sz.Kill();
		cTokenHolder.Kill();

		return Condition(pcCond, iLine);
	}
	else
	{
		sz.Kill();
		cTokenHolder.Kill();

		sPos.Message(&szError);
		szError.Append(szCaclulatorError);
		gcUserError.Set(szError.Text());
		szCaclulatorError.Kill();
		szError.Kill();

		sIndex.Init(-1, -1);  //-1, -1 doesn't seem to strictly mean an error but it does mean stop.
		return sIndex;

	}
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
BOOL CPreprocessor::ProcessIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, BOOL bAllowConditional, int iDepth)
{
	CPPToken*				pcToken;
	CDefine*				pcDefine;
	BOOL					bResult;
	CSpecialOperator*		pcSpecialOperator;
	CPPAbstractHolder*		pcHolder;
	int						i;
	CPPTokenHolder*			pcTokenHolder;
	SDefineArgument*		psArguments;
	int						iArgIndex;

	pcDefine = mcDefines.GetDefine(&pcText->mcText, FALSE);
	if (pcDefine)
	{
		iArgIndex = -1;
		if (pcDefine->IsBacketed())
		{
			pcParser->NextToken();
			psArguments = mcArguments.Add(pcDefine->GetID());
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
				mcArguments.Remove(pcDefine->GetID());
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
			pcHolder->Init(-1, -1);
			ExpandDefined(pcHolder, pcDefine, bAllowConditional, iDepth + 1);
		}

		//I'm not sure if it's safe to do this anymore... another define might refer to it.
		if (iArgIndex != -1)
		{
			psArguments = mcArguments.Get(pcDefine->GetID());
			for (i = 0; i < psArguments->mcArguments.NumElements(); i++)
			{
				pcTokenHolder = psArguments->mcArguments.Get(i);
				pcTokenHolder->Kill();
			}
			mcArguments.Remove(pcDefine->GetID());
		}
		return TRUE;
	}
	else if (bAllowConditional)
	{
		pcSpecialOperator = ProcessSpecialOperator(pcParser);

		if (pcSpecialOperator)
		{
			if (pcSpecialOperator->GetType() == PPSO_Defined)
			{
				bResult = ProcessDefinedIdentifier(pcDest, pcText, pcParser);
				return bResult;
			}
			else if (pcSpecialOperator->GetType() == PPSO_HasAttribute)
			{
				bResult = ProcessHasAttributeIdentifier(pcDest, pcText, pcParser);
				return bResult;
			}
			else if (pcSpecialOperator->GetType() == PPSO_HasCppAttribute)
			{
				bResult = ProcessHasCPPAttributeIdentifier(pcDest, pcText, pcParser);
				return bResult;
			}
			else if (pcSpecialOperator->GetType() == PPSO_HasBuiltIn)
			{
				bResult = ProcessHasBuiltInIdentifier(pcDest, pcText, pcParser);
				return bResult;
			}
			else if (pcSpecialOperator->GetType() == PPSO_HasInclude)
			{
				bResult = ProcessHasIncludeIdentifier(pcDest, pcText, pcParser);
				return bResult;
			}
		}

		pcToken = DuplicatePPToken(pcText, mpcStack);
		pcDest->Add(&pcToken);
		pcParser->NextToken();
		return TRUE;
	}
	else
	{
		pcToken = DuplicatePPToken(pcText, mpcStack);
		pcDest->Add(&pcToken);
		pcParser->NextToken();
		return TRUE;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessDefinedIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	CPPToken*				pcToken;
	CDefine*				pcDefine;
	CPPText*				pcDecorator;
	char*					pcValue;
	BOOL					bOpenBracket;
	CSpecialOperator*		pcSpecial;
	BOOL					bResult;
	SPreprocessorPosition	sPos;
	CChars					szError;

	MarkPositionForError(&sPos);

	bOpenBracket = pcParser->GetExactDecorator('(');
	pcParser->SkipWhiteSpace();
	pcToken = pcParser->GetToken();
	pcDecorator = CPPText::Construct(mpcStack->Add(sizeof(CPPText)));
	pcValue = (char*)gcTokenStrings.Add(1);
	*pcValue = '0';
	pcDecorator->Init(PPT_Number, -1, -1, pcValue, pcValue + 1);
	if (pcToken)
	{
		if (pcToken->IsText())
		{
			pcText = (CPPText*)pcToken;
			if (pcText->meType == PPT_Identifier)
			{
				pcSpecial = mcSpecialOperators.GetSpecialOperator(&pcText->mcText, FALSE);
				if (pcSpecial)
				{
					*pcValue = '1';
				}
				else
				{
					pcDefine = mcDefines.GetDefine(&pcText->mcText, FALSE);
					if (pcDefine)
					{
						*pcValue = '1';
					}
				}
				pcParser->NextToken();
			}
		}
		pcDest->Add((CPPToken**)&pcDecorator);
		if (bOpenBracket)
		{
			bResult = pcParser->GetExactDecorator(')');
			if (bResult)
			{
				return TRUE;
			}
			else
			{
				sPos.Message(&szError);
				szError.Append("Expected ')' following \"defined (\".");
				return  gcUserError.Set(&szError);
			}
		}
	}
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessHasIncludeIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	CHeaderFile*			pcIncludeFile;
	CHeaderNameMap*			pcHeaderNameMap;
	BOOL					bResult;
	CPPText*				pcDecorator;
	char*					pcValue;
	SPreprocessorPosition	sPos;
	CChars					szError;

	MarkPositionForError(&sPos);

	bResult = pcParser->GetExactDecorator('(', TRUE);
	if (bResult)
	{
		bResult = ProcessIncludeFile(pcParser, &pcIncludeFile, &pcHeaderNameMap);
		if (bResult)
		{
			pcDecorator = CPPText::Construct(mpcStack->Add(sizeof(CPPText)));
			pcValue = (char*)gcTokenStrings.Add(1);
			*pcValue = '0';
			pcDecorator->Init(PPT_Number, -1, -1, pcValue, pcValue + 1);
			if (pcIncludeFile)
			{
				*pcValue = '1';
			}

			bResult = pcParser->GetExactDecorator(')', TRUE);
			if (bResult)
			{
				pcDest->Add((CPPToken**)&pcDecorator);
				return TRUE;
			}
			else
			{
				sPos.Message(&szError);
				szError.Append("Expected ')' following \"__has_include\".");
				return  gcUserError.Set(&szError);
			}
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		sPos.Message(&szError);
		szError.Append("Expected '(' following \"__has_include\".");
		return  gcUserError.Set(&szError);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessHasAttributeIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessHasCPPAttributeIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
BOOL CPreprocessor::ProcessHasBuiltInIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	return FALSE;
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

	psDefineArgument = mcArguments.Get(pcReplacement->mlliDefineID);
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
	pcTemp = NULL;
	pcPrev = NULL;
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
			cHolder.Init(-1, -1);
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
			szConcaternated.Init();
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

	szQuoted.Init();
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
	pcArgument->Init();
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
				pcArgument->Init();
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
		mpcCurrentDirective = NULL;
		mpcCurrentLineParser = NULL;
		iOldLine = sLine.iTokenIndex;
		pcToken = *(pcSourceTokens->mcArray.Get(sLine.iTokenIndex));
		if (pcToken->IsDirective())
		{
			pcDirective = (CPPDirective*)pcToken;
			mpcCurrentDirective = pcDirective;
			mpcCurrentLineParser = &cParser;
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

			if (((sLine.iTokenIndex == -1) && (sLine.iBlockIndex == -1)) || (!bResult))
			{
				cParser.Kill();
				sLine.Init(-1, -1);
				break;
			}
			cParser.Kill();

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
	cRawTokens.Init();
	iLen = (int)strlen(szSource);
	cTokeniser.Tokenise(&cRawTokens, &cStack, szSource, iLen, TRUE, 0, 0);
	cTokeniser.Kill();

	cProcessedTokens.Init();
	cPreprocessor.Init(NULL, &cStack);
	cPreprocessor.PreprocessTokens(&cProcessedTokens, &cStack, &cRawTokens, 0, 0);

	szDest->Init();
	cProcessedTokens.Append(szDest);

	cProcessedTokens.Kill();
	cRawTokens.Kill();
	cStack.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE CPreprocessor::EvaluateEquation(char* szText, CChars* szCalculatorError)
{
	CCalculator	cCalculator;
	CNumber		cAnswer;

	cCalculator.Init(FALSE);
	cAnswer = cCalculator.Eval(szText);
	if (cCalculator.HasError())
	{
		szCalculatorError->Append(cCalculator.GetError());
		cCalculator.Kill();
		return TRIERROR;
	}
	else
	{
		cCalculator.Kill();
		if (cAnswer.IsZero())
		{
			return TRIFALSE;
		}
		else
		{
			return TRITRUE;
		}
	}
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
			mcDefines.AddDefine(pszText->Text());
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
	CExternalString		cIdentifier;

	for (i = 0; i < pcDelta->NumElements(); i++)
	{
		pcNamedDefine = pcDelta->Get(i);
		cIdentifier.Init(pcNamedDefine->mszName.Text(), pcNamedDefine->mszName.Length());
		if (pcNamedDefine->miFlags & NAMED_DEFINE_FLAGS_UNDEFFED)
		{
			mcDefines.RemoveDefine(&cIdentifier);
		}
		else
		{
			pcExisting = mcDefines.GetDefine(&cIdentifier, TRUE);
			if (pcExisting)
			{
				mcDefines.RemoveDefine(&cIdentifier);
			}
			pcDefine = mcDefines.AddDefine(&cIdentifier);

			pcDefine->mcArguments.Copy(&pcNamedDefine->mcArguments);
			pcDefine->mcReplacement.Copy(&pcNamedDefine->mcReplacement, pcStack);
			pcDefine->SetBracketed(pcNamedDefine->IsBacketed());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CPreprocessor::ProcessSpecialOperator(CPreprocessorTokenParser* pcParser)
{
	CSpecialOperatorsIndex*			pcIndex;
	SIndexTreeMemoryUnsafeIterator	sIter;
	CSpecialOperator*				pcSpecial;
	BOOL							bHasNext;
	BOOL							bMatch;

	pcIndex = mcSpecialOperators.GetIDToSpecialOperatorIndex();
	bHasNext = pcIndex->StartIteration(&sIter, &pcSpecial, NULL);
	while (bHasNext)
	{
		bMatch = pcParser->GetExactIdentifier(pcSpecial->GetName(), TRUE, TRUE);
		if (bMatch)
		{
			return pcSpecial;
		}
		bHasNext = pcIndex->Iterate(&sIter, &pcSpecial, NULL);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::MarkPositionForError(SPreprocessorPosition* psPos)
{
	char* szShortFileName;

	if (mpcCurrentFile)
	{
		szShortFileName = mpcCurrentFile->ShortName();
	}
	else
	{
		szShortFileName = "";
	}
	if (mpcCurrentLineParser)
	{
		if (mpcCurrentLineParser->Line() == -1)
		{
			if (mpcCurrentDirective)
			{
				psPos->Init(mpcCurrentDirective->Line(), mpcCurrentDirective->Column(), szShortFileName);
			}
			else
			{
				psPos->Init(-1, -1, szShortFileName);
			}
		}
		else
		{
			psPos->Init(mpcCurrentLineParser, mpcCurrentFile);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
int CPreprocessor::GetBlockReuse(void)
{
	return miBlockReuse;
}

