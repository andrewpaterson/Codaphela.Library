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
#include "BaseLib/Calculator.h"
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
#include "PPTokenMemory.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::Init(CConfig* pcConfig)
{
	mcDefines.Init();
	mcDefineReplacementMemory.Init();
	mcDefineStrings.Init(256);

	mcSpecialOperators.Init();
	miIncludeDepth = 0;
	miBlockReuse = 0;
	mbLogBlocks = false;
	mbLogInlucdes = false;
	mbDumpLogs = false;
	miDefineReuse = 0;
	mpszBlocksLog = NULL;
	mpszIncludesLog = NULL;

	mcConditionalStack.Init();
	mpcCurrentFile = NULL;
	mcHeadersStack.Init();  //Current file should be included in the stack.

	mcStack.Init();
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
	mszVaArgs.Init("__VA_ARGS__");

	mcArguments.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::Kill(void)
{
	mcArguments.Kill();
	mszVaArgs.Kill();
	mpcCurrentFile = NULL;
	mcStack.Kill();
	mcHeadersStack.Kill();
	mcConditionalStack.Kill();
	mcHeaderNames.Kill();
	mcSpecialOperators.Kill();

	mcDefines.Kill();
	mcDefineReplacementMemory.Kill();
	mcDefineStrings.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddIncludeDirectories(CArrayHeaderNameMap* pcHeaderNames)
{
	CHeaderFiles*		pcHeaderNameMap;
	size				i;

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
void CPreprocessor::AddIncludeDirectory(CHeaderFiles* pcHeaderMapName)
{
	mcHeaderNames.Add(&pcHeaderMapName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::GetDefine(CExternalString* pcString, bool bExact)
{
	return mcDefines.GetDefine(pcString, bExact);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::GetDefine(char* szName, bool bExact)
{
	return mcDefines.GetDefine(szName, bExact);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CPreprocessor::GetSpecialOperator(CExternalString* pcString, bool bExact)
{
	return mcSpecialOperators.GetSpecialOperator(pcString , bExact);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CPreprocessor::GetSpecialOperator(char* szName, bool bExact)
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
		pcDefine->SetSpecial(true);
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
CDefine* CPreprocessor::AddDefine(char* szDefine, char* szReplacement, CPPTokenMemory* pcTokenMemory)
{
	CDefine*				pcDefine;
	size						iLen;
	char*					sz;
	CPPText					pcText;
	CPreprocessorTokeniser	cTokeniser;

	iLen = strlen(szReplacement);
	pcDefine = mcDefines.AddDefine(szDefine);
	sz = (char*)mcDefineStrings.Add(iLen+1);
	memcpy(sz, szReplacement, iLen+1);

	cTokeniser.Init(GetFileName());
	cTokeniser.TokeniseDefine(pcDefine->GetReplacement()->GetTokenList(), sz, pcTokenMemory);
	cTokeniser.Kill();

	return pcDefine;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessHashDefine(CPreprocessorTokenParser* pcParser)
{
	CExternalString			cIdentifier;
	bool					bResult;
	CDefine*				pcDefine;
	CPPText*				pcText;
	CPPReplacement*			pcReplacement;
	size					iReplaceArg;
	CPPToken*				pcToken;
	bool					bAllocated;
	bool					bAllowWhitespace;
	CDefine*				pcExisting;
	size					iNumArguments;
	size					iReplacementNum;
	bool					bVariadic;
	SPreprocessorPosition	sPos;
	CChars					szError;
	CPPTokenMemory*			pcTokenMemory;

	MarkPositionForError(pcParser, &sPos);

	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		pcExisting = mcDefines.GetDefine(&cIdentifier, true);
		if (pcExisting)
		{
			mcDefines.RemoveDefine(&cIdentifier);
		}
		pcDefine = mcDefines.AddDefine(&cIdentifier);

		bResult = pcParser->GetExactDecorator('(', false);
		if (bResult)
		{
			bResult = ProcessHashDefineBracketted(pcParser, pcDefine);
			if (!bResult)
			{
				return false;
			}
		}
		
		pcTokenMemory = mcStack.GetTokenMemory();

		iNumArguments = pcDefine->GetArguments()->NumElements();
		iReplacementNum = 0;
		bVariadic = pcDefine->IsVariadic();
		bAllowWhitespace = false;
		while (pcParser->HasToken())
		{
			bAllocated = false;
			pcToken = pcParser->GetToken();
			if (pcToken->IsText())
			{
				pcText = (CPPText*)pcToken;
				if (pcText->meType == PPT_Identifier)
				{
					iReplaceArg = pcDefine->GetArguments()->GetIndex(pcText->mcText.msz, pcText->mcText.miLen);
					if (iReplaceArg != -1)
					{
						pcReplacement = pcTokenMemory->AddReplacement();
						pcReplacement->Init(pcDefine->GetID(), iReplaceArg, -1, -1, gszEmptyString, bVariadic && (iReplacementNum == iNumArguments - 1));

						pcToken = pcReplacement;
						bAllocated = true;
						iReplacementNum++;
					}
				}
			}

			if (!pcToken->IsWhitespace() || bAllowWhitespace)  //Stop leading whitespace from being added.
			{
				bAllowWhitespace = true;
				if (!bAllocated)
				{
					pcToken = DuplicatePPToken(pcToken);
				}
				pcDefine->AddReplacmentToken(pcToken);
			}

			pcParser->NextToken();
		}
		return true;
	}
	else
	{
		sPos.Message(&szError);
		szError.Append("Expected identifier");
		return gcUserError.Set(&szError);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessUnknownDirective(CPreprocessorTokenParser* pcParser, CPPTokenList* pcDest)
{
	CExternalString			cIdentifier;
	bool					bResult;
	SPreprocessorPosition	sPos;
	size					iBracketCount;
	bool					bHasToken;
	CPPToken*				pcToken;
	CPPText*				pcText;
	char					c;
	CChars					szError;

	MarkPositionForError(pcParser, &sPos);

	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		iBracketCount = 0;
		bResult = pcParser->GetExactDecorator('(', false);
		if (bResult)
		{
			iBracketCount++;
			bHasToken = pcParser->NextToken();
			while (bHasToken)
			{
				pcToken = pcParser->GetToken();
				if (pcToken->IsText())
				{
					pcText = (CPPText*)pcToken;
					if (pcText->meType == PPT_Decorator)
					{
						c = pcText->mcText.msz[0];
						if (c == '(')
						{
							iBracketCount++;
						}
						else if (c == ')')
						{
							iBracketCount--;
							if (iBracketCount == 0)
							{
								pcParser->NextToken();
								AddZero(pcDest);
								return true;
							}
						}
					}
				}
				bHasToken = pcParser->NextToken();
			}

			sPos.Message(&szError);
			szError.Append("Expected ')'.");
			return gcUserError.Set(&szError);
		}
		else
		{
			AddZero(pcDest);
			return true;
		}
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessHashDefineBracketted(CPreprocessorTokenParser* pcParser, CDefine* pcDefine)
{
	CExternalString			cArgument;
	bool					bResult;
	bool					bVariadicFound;
	SPreprocessorPosition	sPos;
	CChars					szError;

	bVariadicFound = false;
	pcDefine->SetBracketed(true);
	for (;;)
	{
		bResult = pcParser->GetExactDecorator(')');
		if (bResult)
		{
			break;
		}

		if (bVariadicFound)
		{
			sPos.Message(&szError);
			szError.Append("Expected ')' after \"...\".");
			return gcUserError.Set(&szError);
		}

		bResult = pcParser->GetExactDecorator("...");
		if (bResult)
		{
			cArgument.Init(mszVaArgs.Text(), mszVaArgs.Length());
			pcDefine->AddArgument(&cArgument);
			pcDefine->SetVariadic();
			bVariadicFound = true;
		}
		else
		{
			bResult = pcParser->GetIdentifier(&cArgument);
			if (bResult)
			{
				pcDefine->AddArgument(&cArgument);

				bResult = pcParser->GetExactDecorator("...");
				if (bResult)
				{
					pcDefine->SetVariadic();
					bVariadicFound = true;
				}
				else
				{
					pcParser->GetExactDecorator(',');
				}
			}
			else
			{
				sPos.Message(&szError);
				szError.Append("Expected identifier.");
				return gcUserError.Set(&szError);
			}
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessHashUndef(CPreprocessorTokenParser* pcParser)
{
	bool				bResult;
	CExternalString		cIdentifier;

	pcParser->SkipWhitespace();
	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		mcDefines.RemoveDefine(&cIdentifier);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPTokenBlockIndex CPreprocessor::ProcessHashEndif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine)
{
	mcConditionalStack.PopEndIf();

	return Condition(pcCond, iLine);  //This must always return iLine.  It's impossible that it should stop parsing after an #end.
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessHashInclude(CPreprocessorTokenParser* pcParser)
{
	bool						bResult;
	CHeaderFile*				pcIncludeFile;
	CHeaderFiles*				pcHeaderNameMap;
	CExternalString				cExternalString;
	CChars						sz;
	CHeaderNameMapDirectory*	pcNewDirectory;
	CHeaderNameMapDirectory*	pcCurrentDirectory;
	CChars						szPath;
	SPreprocessorPosition		sPos;
	CPPTokenMemory*				pcTokenMemory;
	CPPTokenList*				pcTokenList;
	CPPBlock*					pcBlock;

	pcParser->SkipWhitespace();

	MarkPositionForError(pcParser, &sPos);

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

			pcTokenMemory = mcStack.GetTokenMemory();
			pcTokenList = mcStack.GetTokenList();

			pcBlock = pcTokenMemory->AddBlock();
			pcBlock->Init(-1, -1, GetFileName());
			pcTokenList->Add(pcBlock);

			StackPush(pcBlock->GetTokenList(), pcIncludeFile->GetTokenMemory());

			bResult = PreprocessFile(pcIncludeFile, mpcCurrentFile);

			StackPop();

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
		return false;
	}

}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessIncludeFile(CPreprocessorTokenParser* pcParser, CHeaderFile** ppcCFile, CHeaderFiles** ppcHeaderNameMap)
{
	bool						bResult;
	CExternalString				cExternalString;
	SPreprocessorPosition		sPos;
	CChars						sz;
	char						cOpen;
	char						cClose;

	MarkPositionForError(pcParser, &sPos);

	cOpen = ' ';
	cClose = ' ';
	bResult = pcParser->GetStringDoubleQuoted(&cExternalString);
	if (!bResult)
	{
		bResult = pcParser->GetQuotedCharacterSequence('<', '>', &cExternalString);
		if (bResult)
		{
			cOpen = '<';
			cClose = '>';
		}
	}
	else
	{
		cOpen = '"';
		cClose = '"';
	}

	if (bResult)
	{
		FindBestInclude(&cExternalString, false, ppcCFile, ppcHeaderNameMap);
	}

	if ((!bResult) || (*ppcCFile == NULL))
	{
		sPos.Message(&sz);
		sz.Append("Could not include file ");
		sz.Append(cOpen);
		sz.AppendSubString(cExternalString.msz, cExternalString.EndInclusive() + 1);
		sz.Append(cClose);
		return gcUserError.Set(&sz);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::FindBestInclude(CExternalString* pcInclude, bool bSystemFile, CHeaderFile** ppcCFile, CHeaderFiles** ppcHeaderNameMap)
{
	CChars						szInclude;
	CHeaderFile*				pcBestFile;
	CHeaderFiles*				pcBestHeaderNameMap;
	int32						iMatch;
	int32						iBestMatch;
	size						i;
	CHeaderFiles*				pcHeaderNameMap;
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

		pcHeaderFile = pcHeaderNameMap->GetFile(szInclude.Text());
		if (pcHeaderFile)
		{
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
				if (pcCurrentDirectory->mszBaseDirectoryName.Length() > pcHeaderNameMap->GetBaseDirectoryLength())
				{
					szPath.Init(pcCurrentDirectory->mszBaseDirectoryName);
					szPath.RemoveFromStart(pcHeaderNameMap->GetBaseDirectoryLength()+1);
					szPath.Append('/');
					szPath.Append(szInclude);
					pcHeaderFile = pcHeaderNameMap->GetFile(szPath.Text());
					if (pcHeaderFile)
					{
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
void CPreprocessor::LogInclude(CSourceFile* pcFile)
{
	CChars			sz;

	if (mbLogInlucdes)
	{
		sz.Init();
		sz.Append(' ', miIncludeDepth*4);
		sz.Append(" ");
		sz.Append(pcFile->ShortName());
		sz.Append(" (Included)");
		sz.AppendNewLine();

		if (mbDumpLogs)
		{
			sz.Dump();
		}

		if (mpszIncludesLog)
		{
			mpszIncludesLog->Append(sz);
		}
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CPreprocessor::GetFileName(void)
{
	if (mpcCurrentFile)
	{
		return mpcCurrentFile->ShortName();
	}
	else
	{
		return "* No File *";
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::LogBlocks(CPPBlockSetArray* pacSourceBlockSets, SPPTokenBlockIndex sResult)
{
	CChars			szLine;
	CPPBlockSet*	pcBlocksSet;

	if (mbLogBlocks)
	{
		szLine.Init();
		szLine.Append(' ', miIncludeDepth * 4);
		szLine.Append(' ');
		szLine.Append(GetFileName());

		if (mcConditionalStack.IsParsing())
		{
			szLine.Append(" (Parsing): ");
		}
		else
		{
			szLine.Append("          : ");
		}

		sResult.Print(&szLine);

		pcBlocksSet = pacSourceBlockSets->SafeGet(sResult.iBlockIndex);
		if (!pcBlocksSet)
		{
			szLine.AppendNewLine();
		}
		else
		{
			szLine.Append(" (");
			szLine.Append(pcBlocksSet->Line() + 1);
			szLine.Append(")\n");
		}

		if (mbDumpLogs)
		{
			szLine.Dump();
		}

		if (mpszBlocksLog)
		{
			mpszBlocksLog->Append(szLine);
		}

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
		mbLogInlucdes = true;
		mpszIncludesLog = pszIncludesLog;
	}
	else
	{
		mbLogInlucdes = false;
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
		mbLogBlocks = true;
		mpszBlocksLog = pszBlocksLog;
	}
	else
	{
		mbLogInlucdes = false;
		mbLogBlocks = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::LogBlocks(bool bLogBlocks)
{
	mpszBlocksLog = NULL;
	mbLogBlocks = bLogBlocks;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::LogDumping(bool bDumpLogs)
{
	mbDumpLogs = bDumpLogs;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::PreprocessBlockSets(CPPBlockSetArray* pacSourceBlockSets)
{
	CPPBlockSet*		pcBlocksSet;
	SPPTokenBlockIndex	sResult;

	sResult.Init(0, 0);
	pcBlocksSet = pacSourceBlockSets->SafeGet(sResult.iBlockIndex);
	if (pcBlocksSet == NULL)
	{
		gcUserError.Set("No Blocks in source block set.");
		return false;
	}

	for (;;)
	{
		LogBlocks(pacSourceBlockSets, sResult);

		if (pcBlocksSet->IsDirective())
		{
			//The conditional directives need to be expanded so &pcFile->mcStack is needed.  A #define directive will be expanded too.  Write a test for it.
			sResult = PreprocessDirectiveTokens(pcBlocksSet->GetUnprocssedTokens(), sResult.iBlockIndex, sResult.iTokenIndex);
			if (sResult.iTokenIndex == -1)
			{
				return false;
			}
		}
		else
		{
			CStackMarkExtended	cMark;
			CPPBlock*			pcBlockProcessed;
			CPPBlock*			pcBlockMatching;
			CPPTokenMemory*		pcTokenMemory;
			CPPTokenList*		pcTokenList;

			pcTokenMemory = mcStack.GetTokenMemory();
			pcTokenList = mcStack.GetTokenList();
			pcTokenMemory->Mark(&cMark);

			pcBlockProcessed = pcTokenMemory->AddBlock();
			pcBlockProcessed->Init(pcBlocksSet->Line(), pcBlocksSet->Column(), GetFileName());

			StackPush(pcBlockProcessed->GetTokenList(), pcTokenMemory);
			sResult = PreprocessNormalLineTokens(pcBlocksSet->GetUnprocssedTokens(), sResult.iBlockIndex, sResult.iTokenIndex);
			StackPop();

			if (sResult.iTokenIndex == -1)
			{
				return false;
			}

			pcBlockMatching = pcBlocksSet->GetMatchingBlock(pcBlockProcessed);
			if (pcBlockMatching == NULL)
			{
				pcBlocksSet->AddBlock(pcBlockProcessed);
				pcBlockProcessed->SetNext(sResult.iTokenIndex, sResult.iBlockIndex);
				pcBlockMatching = pcBlockProcessed;
			}
			else
			{
				pcBlockProcessed->Kill();
				pcTokenMemory->Rollback(&cMark);
				miBlockReuse++;
				sResult = pcBlockMatching->GetNextTokenBlock();
			}
			cMark.Kill();

			if (sResult.iTokenIndex == -1)
			{
				return false;
			}

			pcTokenList->Add(pcBlockMatching);
		}

		pcBlocksSet = pacSourceBlockSets->SafeGet(sResult.iBlockIndex);
		if (pcBlocksSet)
		{
			if (pcBlocksSet->GetUnprocssedTokens()->NumTokens() == sResult.iTokenIndex)
			{
				sResult.iBlockIndex++;
				sResult.iTokenIndex = 0;
				pcBlocksSet = pacSourceBlockSets->SafeGet(sResult.iBlockIndex);
			}
		}

		if (pcBlocksSet == NULL)
		{
			return true;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::PreprocessFile(CSourceFile* pcFile, CSourceFile* pcFromFile)
{
	bool				bResult;
	CPPBlockSetArray*	pacSourceBlockSets;

	miIncludeDepth++;
	LogInclude(pcFile);
	mpcCurrentFile = pcFile;

	bResult = TokeniseFile(pcFile);

	if (!bResult)
	{
		return false;
	}

	bResult = true;
	if (!mpcCurrentFile->IsPragmaOnced())
	{
		pacSourceBlockSets = pcFile->GetBlockSets();
		bResult = PreprocessBlockSets(pacSourceBlockSets);
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
		LogIncludes(&pcFile->GetLogs()->szIncludesLog);
	}
	if (pcFile->LogBlocks())
	{
		LogBlocks(&pcFile->GetLogs()->szBlocksLog);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::PreprocessTranslationUnit(CTranslationUnit* pcFile)
{
	bool				bResult;

	TranslationUnitLogging(pcFile);

	pcFile->AddProcessedBlock();

	StackPush(pcFile->GetProcessedTokenList(), pcFile->GetTokenMemory());

	bResult = PreprocessFile(pcFile, NULL);

	StackPop();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::TokeniseFile(CSourceFile* pcFile)
{
	CPreprocessorTokeniser	cTokeniser;
	bool					bResult;
	CPPBlockSetArray*		pacBlockSets;
	char*					szFileContents;
	size					iFileLength;
	CPPTokenMemory*			pcTokenMemory;

	pcFile->Load();
	pacBlockSets = pcFile->GetBlockSets();

	if (!pacBlockSets->IsRawProcessed())
	{
		cTokeniser.Init(GetFileName());

		szFileContents = pcFile->GetContents();
		iFileLength = pcFile->GetContentsLength();
		pcTokenMemory = mcStack.GetTokenMemory();

		bResult = cTokeniser.TokeniseIntoBlockSets(pacBlockSets, pcTokenMemory,  szFileContents, iFileLength);
		cTokeniser.Kill();

		return bResult;
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPTokenBlockIndex CPreprocessor::ProcessHashIfndef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine)
{
	CExternalString		cIdentifier;
	CDefine*			pcDefine;
	bool				bResult;
	bool				bEvaluated;

	pcParser->SkipWhitespace();
	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		pcDefine = mcDefines.GetDefine(&cIdentifier, true);
		bEvaluated = (pcDefine == NULL);
		mcConditionalStack.PushIfNotDefined(bEvaluated);
	}	
	else
	{
		mcConditionalStack.PushIfNotDefined(true);	
	}

	return Condition(pcCond, iLine);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPTokenBlockIndex CPreprocessor::ProcessHashIfdef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine)
{
	CExternalString		cIdentifier;
	CDefine*			pcDefine;
	bool				bResult;
	bool				bEvaluated;

	pcParser->SkipWhitespace();
	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		pcDefine = mcDefines.GetDefine(&cIdentifier, true);
		bEvaluated = (pcDefine != NULL);
		mcConditionalStack.PushIfDefined(bEvaluated);
	}	
	else
	{
		mcConditionalStack.PushIfDefined(false);	
	}

	return Condition(pcCond, iLine);
}

//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPTokenBlockIndex CPreprocessor::Condition(CPPConditional* pcCond, SPPTokenBlockIndex iLine)
{
	CPPConditional*		pcNext;
	SPPTokenBlockIndex	sIndex;

	if (mcConditionalStack.IsParsing())
	{
		iLine.iTokenIndex++;
		return iLine;
	}
	else
	{
		pcNext = pcCond->GetNext();
		if (pcNext)
		{
			sIndex = pcNext->GetTokenBlock();
			return sIndex;
		}
		else if (pcCond->Is(PPD_endif))
		{
			sIndex.iBlockIndex = pcCond->GetTokenBlock().iBlockIndex;
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
SPPTokenBlockIndex CPreprocessor::ProcessHashElse(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine)
{
	mcConditionalStack.SwapForElse();

	return Condition(pcCond, iLine);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPTokenBlockIndex CPreprocessor::ProcessHashIf(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine)
{
	TRISTATE				sEvaluated;
	CPPTokenList			cTokenHolder;
	CChars					sz;
	SPPTokenBlockIndex			sIndex;
	CChars					szCaclulatorError;
	SPreprocessorPosition	sPos;
	CChars					szError;
	bool					bResult;

	MarkPositionForError(pcParser, &sPos);

	cTokenHolder.Init();
	bResult = ProcessDirectiveLine(&cTokenHolder, pcParser, 0);
	if (!bResult)
	{
		cTokenHolder.Kill();

		sIndex.Init(-1, -1);  //-1, -1 doesn't seem to strictly mean an error but it does mean stop.
		return sIndex;
	}

	sz.Init();
	cTokenHolder.Print(&sz);
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
SPPTokenBlockIndex CPreprocessor::ProcessHashElif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine)
{
	TRISTATE				sEvaluated;
	CPPTokenList			cTokenHolder;
	CChars					sz;
	SPPTokenBlockIndex			sIndex;
	SPreprocessorPosition	sPos;
	CChars					szCaclulatorError;
	CChars					szError;

	MarkPositionForError(pcParser, &sPos);

	cTokenHolder.Init();
	szCaclulatorError.Init();
	ProcessDirectiveLine(&cTokenHolder, pcParser, 0);
	sz.Init();
	cTokenHolder.Print(&sz);
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
bool CPreprocessor::ProcessHashError(CPreprocessorTokenParser* pcParser)
{
	CChars			szError;
	CPPToken*		pcToken;

	pcParser->SkipWhitespace();
	szError.Init("Error (");
	szError.Append(GetFileName());
	szError.Append("):");

	while (pcParser->HasToken())
	{
		pcToken = pcParser->GetToken();
		pcToken->Print(&szError);
		pcParser->NextToken();
	}		
	szError.AppendNewLine();
	szError.Dump();
	szError.Kill();

	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessHashPragma(CPreprocessorTokenParser* pcParser)
{
	bool			bResult;
	CHeaderFile*	pcHeader;

	pcParser->SkipWhitespace();
	bResult = pcParser->GetExactIdentifier("once", true, true);
	if (bResult == TRITRUE)
	{
		if (mpcCurrentFile)
		{
			if (mpcCurrentFile->IsHeader())
			{
				pcHeader = (CHeaderFile*)mpcCurrentFile;
				pcHeader->SetPragmaOnce();
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessNormalLine(CPreprocessorTokenParser* pcParser)
{
	CPPLine*			pcLine;
	bool				bResult;
	CPPTokenMemory*		pcTokenMemory;
	CPPTokenList*		pcTokenList;

	if (pcParser->HasTokens())
	{
		pcTokenMemory = mcStack.GetTokenMemory();
		pcLine = pcTokenMemory->AddLine();
		pcLine->Init(pcParser->Line(), pcParser->Column(), GetFileName());

		bResult = ProcessNormalLine(pcLine->GetTokenList(), pcParser, 0);
		if (bResult)
		{
			if (pcLine->TokenLength() > 0)
			{
				pcTokenList = mcStack.GetTokenList();
				pcTokenList->Add(pcLine);
			}
			return true;
		}
		return false;
	}
	else
	{
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessSingleHash(CPPTokenList* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessDoubleHash(CPPTokenList* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessIdentifierDirective(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, size iDepth)
{
	CDefine*					pcDefine;
	bool						bResult;
	CSpecialOperator*			pcSpecialOperator;
	CPPTokenListHolder*			pcHolder;
	SDefineArgument*			psArguments;
	int							iArgIndex;
	int							iReplacementTokens;
	CPreprocessorTokenParser	cParser;
	CPPTokenMemory*				pcTokenMemory;

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

	pcDefine = mcDefines.GetDefine(&pcText->mcText, true);
	if (pcDefine)
	{
		iArgIndex = -1;
		if (pcDefine->IsBacketed())
		{
			pcParser->NextToken();
			psArguments = mcArguments.Add(pcDefine->GetID());
			iArgIndex = (int)mcArguments.GetDefineArgumentsIndex(psArguments);
			bResult = FindArguments(pcParser, &psArguments->macTokenArguments);
			if (!bResult)
			{
				KillArguments(psArguments);
				mcArguments.Remove(pcDefine->GetID());

				return false;
			}
			else if (!pcDefine->CanProcessArguments(psArguments->macTokenArguments.NumElements()))
			{
				KillArguments(psArguments);
				mcArguments.Remove(pcDefine->GetID());
				return false;
			}
		}
		else
		{
			pcParser->NextToken();
		}

		iReplacementTokens = pcDefine->GetReplacement()->GetTokenList()->NumTokens();
		if (iReplacementTokens > 0)
		{
			pcTokenMemory = mcStack.GetTokenMemory();
			pcHolder = pcTokenMemory->AddTokenReplacementsHolder();
			pcHolder->Init(-1, -1, GetFileName());

			pcDest->Add(pcHolder);

			cParser.Init(pcDefine->GetReplacement());
			ProcessDirectiveLine(pcHolder->GetTokenList(), &cParser, iDepth);
			cParser.Kill();
		}
		else if (iReplacementTokens == 0)
		{
			AddZero(pcDest);
		}

		//I'm not sure if it's safe to do this anymore... another define might refer to it.
		if (iArgIndex != -1)
		{
			psArguments = mcArguments.Get(pcDefine->GetID());
			KillArguments(psArguments);
			mcArguments.Remove(pcDefine->GetID());
		}
		return true;
	}

	bResult = ProcessUnknownDirective(pcParser, pcDest);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessIdentifierNormalLine(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, size iDepth)
{
	CPPToken*					pcToken;
	CDefine*					pcDefine;
	bool						bResult;
	CPPTokenListHolder*			pcHolder;
	SDefineArgument*			psArguments;
	int							iArgIndex;
	int							iReplacementTokens;
	CPreprocessorTokenParser	cParser;
	CPPTokenMemory*				pcTokenMemory;

	pcDefine = mcDefines.GetDefine(&pcText->mcText, true);
	if (pcDefine)
	{
		iArgIndex = -1;
		if (pcDefine->IsBacketed())
		{
			pcParser->NextToken();
			psArguments = mcArguments.Add(pcDefine->GetID());
			iArgIndex = (int)mcArguments.GetDefineArgumentsIndex(psArguments);
			bResult = FindArguments(pcParser, &psArguments->macTokenArguments);
			if (!bResult)
			{
				KillArguments(psArguments);
				mcArguments.Remove(pcDefine->GetID());

				return false;
			}
			else if (!pcDefine->CanProcessArguments(psArguments->macTokenArguments.NumElements()))
			{
				KillArguments(psArguments);
				mcArguments.Remove(pcDefine->GetID());
				return false;
			}
		}
		else
		{
			pcParser->NextToken();
		}

		iReplacementTokens = pcDefine->GetReplacement()->GetTokenList()->NumTokens();
		if (iReplacementTokens > 0)
		{
			pcTokenMemory = mcStack.GetTokenMemory();
			pcHolder = pcTokenMemory->AddTokenReplacementsHolder();
			pcHolder->Init(-1, -1, GetFileName());

			pcDest->Add(pcHolder);
			
			cParser.Init(pcDefine->GetReplacement());
			ProcessNormalLine(pcHolder->GetTokenList(), &cParser, iDepth);
			cParser.Kill();
		}

		//I'm not sure if it's safe to do this anymore... another define might refer to it.
		if (iArgIndex != -1)
		{
			psArguments = mcArguments.Get(pcDefine->GetID());
			KillArguments(psArguments);
			mcArguments.Remove(pcDefine->GetID());
		}
		return true;
	}
	else
	{
		pcToken = DuplicatePPToken(pcText);
		pcDest->Add(pcToken);
		pcParser->NextToken();
		return true;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::KillArguments(SDefineArgument* psArguments)
{
	size				i;
	CPPTokenList*		pcTokenHolder;

	for (i = 0; i < psArguments->macTokenArguments.NumElements(); i++)
	{
		pcTokenHolder = psArguments->macTokenArguments.Get(i);
		pcTokenHolder->Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessDefinedIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	CPPToken*				pcToken;
	CDefine*				pcDefine;
	CPPText*				pcDecorator;
	char*					pcValue;
	bool					bOpenBracket;
	CSpecialOperator*		pcSpecial;
	bool					bResult;
	SPreprocessorPosition	sPos;
	CChars					szError;
	CPPTokenMemory*			pcTokenMemory;

	MarkPositionForError(pcParser, &sPos);

	bOpenBracket = pcParser->GetExactDecorator('(');
	pcParser->SkipWhitespace();
	pcToken = pcParser->GetToken();
	pcTokenMemory = mcStack.GetTokenMemory();

	if (pcToken)
	{
		pcValue = (char*)mcDefineStrings.Add(1);
		*pcValue = '0';

		pcDecorator = pcTokenMemory->AddText();
		pcDecorator->Init(PPT_Number, -1, -1, GetFileName(), pcValue, pcValue + 1);

		if (pcToken->IsText())
		{
			pcText = (CPPText*)pcToken;
			if (pcText->meType == PPT_Identifier)
			{
				pcSpecial = mcSpecialOperators.GetSpecialOperator(&pcText->mcText, false);
				if (pcSpecial)
				{
					*pcValue = '1';
				}
				else
				{
					pcDefine = mcDefines.GetDefine(&pcText->mcText, true);
					if (pcDefine)
					{
						*pcValue = '1';
					}
				}
				pcParser->NextToken();
			}
		}

		pcDest->Add(pcDecorator);
		if (bOpenBracket)
		{
			bResult = pcParser->GetExactDecorator(')');
			if (bResult)
			{
				return true;
			}
			else
			{
				sPos.Message(&szError);
				szError.Append("Expected ')' following \"defined (\".");
				return  gcUserError.Set(&szError);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessHasIncludeIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	CHeaderFile*			pcIncludeFile;
	CHeaderFiles*			pcHeaderNameMap;
	bool					bResult;
	CPPText*				pcDecorator;
	char*					pcValue;
	SPreprocessorPosition	sPos;
	CChars					szError;
	CPPTokenMemory*			pcTokenMemory;

	MarkPositionForError(pcParser, &sPos);

	bResult = pcParser->GetExactDecorator('(', true);
	if (bResult)
	{
		bResult = ProcessIncludeFile(pcParser, &pcIncludeFile, &pcHeaderNameMap);
		if (bResult)
		{
			pcTokenMemory = mcStack.GetTokenMemory();
			pcDecorator = pcTokenMemory->AddText();
			pcValue = (char*)mcDefineStrings.Add(1);
			*pcValue = '0';
			pcDecorator->Init(PPT_Number, -1, -1, GetFileName(), pcValue, pcValue + 1);
			if (pcIncludeFile)
			{
				*pcValue = '1';
			}

			bResult = pcParser->GetExactDecorator(')', true);
			if (bResult)
			{
				pcDest->Add(pcDecorator);
				return true;
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
			return false;
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
bool CPreprocessor::ProcessHasAttributeIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessHasCPPAttributeIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	bool					bResult;
	SPreprocessorPosition	sPos;
	CChars					szError;
	CChars					szAttribute;
	bool					bClosingBracket;
	CExternalString			pcIdentifier;

	MarkPositionForError(pcParser, &sPos);

	bResult = pcParser->GetExactDecorator('(');
	if (!bResult)
	{
		sPos.Message(&szError);
		szError.Append("Expected '(' after __has_cpp_attribute.");
		return gcUserError.Set(&szError);
	}

	szAttribute.Init();
	bClosingBracket = pcParser->GetExactDecorator(')');
	while (!bClosingBracket)
	{
		bResult = pcParser->GetIdentifier(&pcIdentifier);
		if (!bResult)
		{
			szAttribute.Kill();
			sPos.Message(&szError);
			szError.Append("Expected '(' after __has_cpp_attribute.");
			return gcUserError.Set(&szError);
		}

		szAttribute.Append(pcIdentifier.msz, pcIdentifier.miLen);

		bResult = pcParser->GetExactDecorator("::", false);
		if (bResult)
		{
			szAttribute.Append("::");
		}
		else 
		{
			bClosingBracket = pcParser->GetExactDecorator(')');
			if (!bClosingBracket)
			{
				szAttribute.Kill();
				sPos.Message(&szError);
				szError.Append("Expected ')'.");
				return gcUserError.Set(&szError);
			}
		}
	}

	//We have no attributes at all so always subsitute zero.
	AddZero(pcDest);

	szAttribute.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessHasBuiltInIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ExpandDirectiveTokenIfNecessary(CPPToken* pcToken, CPPTokenList* pcDest, CPreprocessorTokenParser* pcParser, size iDepth)
{		
	CPPToken*				pcNewToken;
	CPPText*				pcText;
	bool					bResult;
	CPPReplacement*			pcReplacement;

	if (pcToken->IsText())
	{
		pcText = (CPPText*)pcToken;
		if (pcText->meType == PPT_Identifier)
		{
			bResult = ProcessIdentifierDirective(pcDest, pcText, pcParser, iDepth);
			return bResult;
		}
		else
		{
			pcNewToken = DuplicatePPToken(pcToken);
			pcDest->Add(pcNewToken);
			pcParser->NextToken();
			return true;
		}
	}
	else if (pcToken->IsReplacement())
	{
		pcReplacement = (CPPReplacement*)pcToken;
		ExpandDirectiveReplacement(pcReplacement, pcDest, iDepth);
		pcParser->NextToken();
		return true;
	}
	else
	{
		pcNewToken = DuplicatePPToken(pcToken);
		pcDest->Add(pcNewToken);
		pcParser->NextToken();
		return true;
	}
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessor::DuplicatePPToken(CPPToken* pcSource)
{
	CPPTokenMemory*		pcTokenMemory;

	pcTokenMemory = mcStack.GetTokenMemory();
	return ::DuplicatePPToken(pcSource, pcTokenMemory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ExpandNormalLineTokenIfNecessary(CPPToken* pcToken, CPPTokenList* pcDest, CPreprocessorTokenParser* pcParser, size iDepth)
{
	CPPToken*			pcNewToken;
	CPPText*			pcText;
	bool				bResult;
	CPPReplacement*		pcReplacement;

	if (pcToken->IsText())
	{
		pcText = (CPPText*)pcToken;
		if (pcText->meType == PPT_Identifier)
		{
			bResult = ProcessIdentifierNormalLine(pcDest, pcText, pcParser, iDepth);
			return bResult;
		}
		else
		{
			pcNewToken = DuplicatePPToken(pcToken);
			pcDest->Add(pcNewToken);
			pcParser->NextToken();
			return true;
		}
	}
	else if (pcToken->IsReplacement())
	{
		pcReplacement = (CPPReplacement*)pcToken;
		ExpandReplacementNormalLine(pcReplacement, pcDest, iDepth);
		pcParser->NextToken();
		return true;
	}
	else
	{
		pcNewToken = DuplicatePPToken(pcToken);
		pcDest->Add(pcNewToken);
		pcParser->NextToken();
		return true;
	}
};


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ExpandDirectiveReplacement(CPPReplacement* pcReplacement, CPPTokenList* pcDest, size iDepth)
{
	CArrayPPTokenLists*			pcArguments;
	CPPTokenList*				pcArgument;
	CPreprocessorTokenParser	cParser;
	CPPLine						cLine;
	SDefineArgument*			psDefineArgument;
	int							i;
	bool						bFirst;
	int							iNumElements;

	psDefineArgument = mcArguments.Get(pcReplacement->mlliDefineID);
	pcArguments = &psDefineArgument->macTokenArguments;
	if (pcArguments)
	{
		if (!pcReplacement->IsVariadic())
		{
			pcArgument = pcArguments->Get(pcReplacement->miArgIndex);
			if (pcArgument)
			{
				cLine.Fake(pcArgument);
				cParser.Init(&cLine);
				ProcessDirectiveLine(pcDest, &cParser, iDepth);
				cParser.Kill();
			}
		}
		else
		{
			bFirst = true;
			iNumElements = (int)pcArguments->NumElements();
			for (i = pcReplacement->miArgIndex; i < iNumElements; i++)
			{
				pcArgument = pcArguments->Get(i);
				if (pcArgument)
				{
					if (bFirst)
					{
						bFirst = false;
					}
					else
					{
						AddComma(pcDest);
					}
					cLine.Fake(pcArgument);
					cParser.Init(&cLine);
					ProcessDirectiveLine(pcDest, &cParser, iDepth);
					cParser.Kill();
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ExpandReplacementNormalLine(CPPReplacement* pcReplacement, CPPTokenList* pcDest, size iDepth)
{
	CArrayPPTokenLists*			pcArguments;
	CPPTokenList*				pcArgument;
	CPreprocessorTokenParser	cParser;
	CPPLine						cLine;
	SDefineArgument*			psDefineArgument;
	int							i;
	bool						bFirst;
	int							iNumElements;

	psDefineArgument = mcArguments.Get(pcReplacement->mlliDefineID);
	pcArguments = &psDefineArgument->macTokenArguments;
	if (pcArguments)
	{
		if (!pcReplacement->IsVariadic())
		{
			pcArgument = pcArguments->Get(pcReplacement->miArgIndex);
			if (pcArgument)
			{
				cLine.Fake(pcArgument);
				cParser.Init(&cLine);
				ProcessNormalLine(pcDest, &cParser, iDepth);
				cParser.Kill();
			}
		}
		else
		{
			bFirst = true;
			iNumElements = (int)pcArguments->NumElements();
			for (i = pcReplacement->miArgIndex; i < iNumElements; i++)
			{
				pcArgument = pcArguments->Get(i);
				if (pcArgument)
				{
					if (bFirst)
					{
						bFirst = false;
					}
					else
					{
						AddComma(pcDest);
					}
					cLine.Fake(pcArgument);
					cParser.Init(&cLine);
					ProcessNormalLine(pcDest, &cParser, iDepth);
					cParser.Kill();
				}
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessDirectiveLine(CPPTokenList* pcDest, CPreprocessorTokenParser* pcParser, size iDepth)
{
	CPPToken*				pcToken;
	SPreprocessorPosition	sPos;
	CChars					szError;

	MarkPositionForError(pcParser, &sPos);

	while (pcParser->HasToken())
	{
		pcToken = pcParser->GetToken();
		if (pcToken->IsHash())
		{
			sPos.Message(&szError);
			szError.Append("Unexpected '#'.");
			return gcUserError.Set(&szError);
		}

		ExpandDirectiveTokenIfNecessary(pcToken, pcDest, pcParser, iDepth);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessNormalLine(CPPTokenList* pcDest, CPreprocessorTokenParser* pcParser, size iDepth)
{
	CPPHashes*					pcHash;
	CPPTokenReplacementsHolder	cHolder;
	size						iHashCount;
	CPPToken*					pcTemp;
	CPPToken*					pcPrev;
	CPPToken*					pcToken;

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
			ExpandNormalLineTokenIfNecessary(pcToken, pcDest, pcParser, iDepth);
		}
		else if (iHashCount == 1)  //# Quote following.
		{
			cHolder.Init(-1, -1, GetFileName());
			ExpandNormalLineTokenIfNecessary(pcToken, cHolder.GetTokenList(), pcParser, iDepth);
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
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessor::ConcaternateTokens(CPPTokenList* pcDest, CPPToken* pcLeft, CPPToken* pcRight)
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
			pcLeftText->Print(&szConcaternated);
			pcRightText->Print(&szConcaternated);

			szInStrings = (char*)mcDefineStrings.Add(szConcaternated.Length()+1);
			memcpy(szInStrings, szConcaternated.Text(), szConcaternated.Length()+1);

			pcLeftText->mcText.msz = szInStrings;
			pcLeftText->mcText.miLen = szConcaternated.Length();

			szConcaternated.Kill();
			return pcLeft;
		}
		else
		{
			pcDest->Add(pcRight);
			return pcRight;
		}
	}
	return pcLeft;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessor::QuoteTokens(CPPTokenList* pcDest, CPPTokenListHolder* pcHolder)
{
	int				j;
	bool			bQuoteNextReplacement;
	CChars			szQuoted;
	char*			szInStrings;
	CPPText*		pcQuoted;
	CPPToken*		pcToken;
	int				iNumTokens;
	CPPTokenMemory* pcTokenMemory;

	iNumTokens = pcHolder->GetTokenList()->NumTokens();
	szQuoted.Init();
	szQuoted.Append('"');
	for (j = 0; j < iNumTokens; j++)
	{
		pcToken = pcHolder->GetTokenList()->Get(j);
		pcToken->Print(&szQuoted);
	}
	szQuoted.Append('"');
	bQuoteNextReplacement = false;

	szInStrings = (char*)mcDefineStrings.Add(szQuoted.Length()+1);
	memcpy(szInStrings, szQuoted.Text(), szQuoted.Length()+1);

	pcTokenMemory = mcStack.GetTokenMemory();

	pcQuoted = pcTokenMemory->AddText();
	pcQuoted->Init(PPT_DoubleQuoted, -1, -1, GetFileName(), szInStrings, szInStrings + szQuoted.Length());

	pcDest->Add(pcQuoted);
	szQuoted.Kill();
	return pcQuoted;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessor::AddToken(CPPToken* pcToken, CArrayPPTokenPtrs* pacTokenPts)
{
	CPPToken**	ppcToken;

	ppcToken = pacTokenPts->Add();
	*ppcToken = pcToken;
	return pcToken;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::FindArguments(CPreprocessorTokenParser* pcParser, CArrayPPTokenLists* pacArguments)
{
	CPPTokenList*			pcArgument;
	bool					bResult;
	int						iBracketDepth;
	CPPToken*				pcSomeToken;
	bool					bReturn;
	SPreprocessorPosition	sPos;

	MarkPositionForError(pcParser, &sPos);

	bResult = pcParser->GetExactDecorator('(');
	if (!bResult)
	{
		return false;
	}

	iBracketDepth = 1;
	pcArgument = pacArguments->Add();
	pcArgument->Init();
	bReturn = false;
	for (;;)
	{
		pcSomeToken = pcParser->GetToken();
		if (pcSomeToken == NULL)
		{
			//Out of file...
			if (iBracketDepth == 1)
			{
				bReturn = true;
			}
			break;
		}
		bResult = pcParser->GetExactDecorator(')', false);
		if (bResult)
		{
			if (iBracketDepth == 1)
			{
				bReturn = true;
				break;
			}
			else 
			{
				AddTokenToArgument(pcArgument, pcSomeToken);
				iBracketDepth--;
				continue;
			}

		}

		bResult = pcParser->GetExactDecorator('(', false);
		if (bResult)
		{
			AddTokenToArgument(pcArgument, pcSomeToken);
			iBracketDepth++;
			continue;
		}

		bResult = pcParser->GetExactDecorator(',', false);
		if (bResult)
		{
			if (iBracketDepth > 1)
			{
				AddTokenToArgument(pcArgument, pcSomeToken);
				continue;
			}
			else
			{
				pcParser->SkipWhitespace();
				pcArgument = pacArguments->Add();
				pcArgument->Init();
				continue;
			}
		}

		AddTokenToArgument(pcArgument, pcSomeToken);
		pcParser->NextToken();
	}

	if (pcArgument->NumTokens() == 0)
	{
		pcArgument->Kill();
		pacArguments->RemoveTail();
	}

	return bReturn;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddTokenToArgument(CPPTokenList* pcArgument, CPPToken* pcToken)
{
	pcToken = DuplicatePPToken(pcToken);
	pcArgument->Add(pcToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPTokenBlockIndex CPreprocessor::PreprocessDirectiveTokens(CPPTokenList* pcSourceTokens, size iBlock, size iToken)
{
	SPPTokenBlockIndex			sLine;
	size						iNumLines;
	CPPToken*					pcToken;
	CPPDirective*				pcDirective;
	CPreprocessorTokenParser	cParser;
	bool						bResult;
	size							iOldLine;
	CChars						szError;
	SPreprocessorPosition		sPos;

	bResult = true;
	iNumLines = pcSourceTokens->NumTokens();
	sLine.iBlockIndex = iBlock;
	for (sLine.iTokenIndex = iToken; sLine.iTokenIndex < iNumLines; )
	{
		iOldLine = sLine.iTokenIndex;
		pcToken = pcSourceTokens->Get(sLine.iTokenIndex);
		if (pcToken->IsDirective())
		{
			pcDirective = (CPPDirective*)pcToken;
			cParser.Init(pcDirective);
			if (pcDirective->IsConditional())
			{
				if (pcDirective->Is(PPD_ifdef))
				{
					sLine = ProcessHashIfdef(&cParser, (CPPConditional*)pcDirective, sLine);
				}
				else if (pcDirective->Is(PPD_ifndef))
				{
					sLine = ProcessHashIfndef(&cParser, (CPPConditional*)pcDirective, sLine);
				}
				else if (pcDirective->Is(PPD_endif))
				{
					sLine = ProcessHashEndif(&cParser, (CPPConditional*)pcDirective, sLine);
				}
				else if (pcDirective->Is(PPD_else))
				{
					sLine = ProcessHashElse(&cParser, (CPPConditional*)pcDirective, sLine);
				}
				else if (pcDirective->Is(PPD_if))
				{
					sLine = ProcessHashIf(&cParser, (CPPConditional*)pcDirective, sLine);
				}
				else if (pcDirective->Is(PPD_elif))
				{
					sLine = ProcessHashElif(&cParser, (CPPConditional*)pcDirective, sLine);
				}
			}
			else
			{
				if (mcConditionalStack.IsParsing())
				{
					if (pcDirective->Is(PPD_define))
					{
						bResult = ProcessHashDefine(&cParser);
					}
					else if (pcDirective->Is(PPD_include))
					{
						bResult = ProcessHashInclude(&cParser);
					}
					else if (pcDirective->Is(PPD_undef))
					{
						bResult = ProcessHashUndef(&cParser);
					}
					else if (pcDirective->Is(PPD_error))
					{
						bResult = ProcessHashError(&cParser);
					}
					else if (pcDirective->Is(PPD_pragma))
					{
						bResult = ProcessHashPragma(&cParser);
					}
				}
				sLine.iTokenIndex++;
			}

			if (((sLine.iTokenIndex >= 0) && (sLine.iTokenIndex < iOldLine)) && (sLine.iBlockIndex == iBlock))
			{
				gcUserError.Set("Parsing went backwards.  WTF!");
				bResult = false;
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
		else
		{
			bResult = false;

			MarkPositionForError(pcToken->Line(), pcToken->Column(), &sPos);
			sPos.Message(&szError);
			szError.Append("Expected a Directive got [");
			szError.Append(pcToken->ClassName());
			szError.Append("].");
			gcUserError.Set(&szError);
			sLine.Init(-1, -1);
			break;
		}
	}

	return sLine;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPTokenBlockIndex CPreprocessor::PreprocessNormalLineTokens(CPPTokenList* pcSourceTokens, size iBlock, size iToken)
{
	SPPTokenBlockIndex			sLine;
	size						iNumLines;
	CPPToken*					pcToken;
	CPPLine*					pcLine;
	CPreprocessorTokenParser	cParser;
	bool						bResult;
	size						iOldLine;
	CChars						szError;
	SPreprocessorPosition		sPos;

	bResult = true;
	iNumLines = pcSourceTokens->NumTokens();
	sLine.iBlockIndex = iBlock;
	for (sLine.iTokenIndex = iToken; sLine.iTokenIndex < iNumLines; )
	{
		iOldLine = sLine.iTokenIndex;
		pcToken = pcSourceTokens->Get(sLine.iTokenIndex);
		if (pcToken->IsLine())
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
			bResult = false;

			MarkPositionForError(pcToken->Line(), pcToken->Column(), &sPos);
			sPos.Message(&szError);
			szError.Append("Expected a Normal Line got [");
			szError.Append(pcToken->ClassName());
			szError.Append("].");
			gcUserError.Set(&szError);
			sLine.Init(-1, -1);
			break;
		}
	}

	return sLine;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::Preprocess(char* szSource, CChars* pszDest, bool bShowFileAndLine)
{
	CPreprocessor			cPreprocessor;
	CPreprocessorTokeniser	cTokeniser;
	int						iLen;
	CPPTokenMemory			cTokenMemory;
	CPPTokenList			cOutput;
	CPPBlockSetArray		acBlockSets;
	bool					bResult;

	cTokeniser.Init("");
	cTokenMemory.Init();

	iLen = (int)strlen(szSource);
	acBlockSets.Init();
	bResult = cTokeniser.TokeniseIntoBlockSets(&acBlockSets, &cTokenMemory, szSource, iLen);
	cTokeniser.Kill();

	if (!bResult)
	{
		return false;
	}

	cOutput.Init();
	cPreprocessor.Init(NULL);
	cPreprocessor.StackPush(&cOutput, &cTokenMemory);
	bResult = cPreprocessor.PreprocessBlockSets(&acBlockSets);
	cPreprocessor.StackPop();

	if (pszDest)
	{
		cOutput.Print(pszDest, bShowFileAndLine);
	}

	cPreprocessor.Kill();

	acBlockSets.Kill();

	cOutput.Kill();
	cTokenMemory.Kill();

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE CPreprocessor::EvaluateEquation(char* szText, CChars* szCalculatorError)
{
	CCalculator	cCalculator;
	CNumber		cAnswer;

	cCalculator.Init(false);
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
	size		i;
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
void CPreprocessor::DeltaDefines(CArrayNamedDefines* pcDelta)
{
	size				i;
	CNamedDefine*		pcNamedDefine;
	CDefine*			pcExisting;
	CDefine*			pcDefine;
	CExternalString		cIdentifier;
	CPPTokenMemory*		pcTokenMemory;

	pcTokenMemory = mcStack.GetTokenMemory();
	for (i = 0; i < pcDelta->NumElements(); i++)
	{
		pcNamedDefine = pcDelta->Get(i);
		cIdentifier.Init(pcNamedDefine->GetName(), pcNamedDefine->GetNameLength());
		if (pcNamedDefine->IsUndeffed())
		{
			mcDefines.RemoveDefine(&cIdentifier);
		}
		else
		{
			pcExisting = mcDefines.GetDefine(&cIdentifier, true);
			if (pcExisting)
			{
				mcDefines.RemoveDefine(&cIdentifier);
			}
			pcDefine = mcDefines.AddDefine(&cIdentifier);

			pcDefine->GetArguments()->Copy(pcNamedDefine->GetArguments());
			pcDefine->GetReplacement()->Copy(pcNamedDefine->GetReplacement(), pcTokenMemory);
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
	bool							bHasNext;
	bool							bMatch;

	pcIndex = mcSpecialOperators.GetIDToSpecialOperatorIndex();
	bHasNext = pcIndex->StartIteration(&sIter, &pcSpecial, NULL);
	while (bHasNext)
	{
		bMatch = pcParser->GetExactIdentifier(pcSpecial->GetName(), true, true);
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
void CPreprocessor::MarkPositionForError(CPreprocessorTokenParser* pcParser, SPreprocessorPosition* psPos)
{
	char* szShortFileName;

	szShortFileName = GetFileName();
	psPos->Init(pcParser, szShortFileName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::MarkPositionForError(size iLine, size iColumn, SPreprocessorPosition* psPos)
{
	char* szShortFileName;

	szShortFileName = GetFileName();
	psPos->Init(iLine, iColumn, szShortFileName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::StackPush(CPPTokenList* pcTokenList, CPPTokenMemory* pcTokenMemory)
{
	mcStack.Push(pcTokenList, pcTokenMemory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::StackPop(void)
{
	mcStack.Pop();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
size CPreprocessor::GetBlockReuse(void)
{
	return miBlockReuse;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddComma(CPPTokenList* pcDest)
{
	CPPTextWithSource*	pcPPComma;
	CPPTextWithSource*	pcPPSpace;
	CPPTokenMemory*		pcTokenMemory;

	pcTokenMemory = mcStack.GetTokenMemory();
	pcPPComma = pcTokenMemory->AddTextWithSource();
	pcPPComma->Init(PPT_Decorator, -1, -1, GetFileName(), ",", 1);
	pcDest->Add(pcPPComma);

	pcPPSpace = pcTokenMemory->AddTextWithSource();
	pcPPSpace->Init(PPT_Decorator, -1, -1, GetFileName(), " ", 1);
	pcDest->Add(pcPPSpace);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddZero(CPPTokenList* pcDest)
{
	CPPTextWithSource* pcPPZero;
	CPPTokenMemory*		pcTokenMemory;

	pcTokenMemory = mcStack.GetTokenMemory();

	pcPPZero = pcTokenMemory->AddTextWithSource();
	pcPPZero->Init(PPT_Decorator, -1, -1, GetFileName(), "0", 1);
	pcDest->Add(pcPPZero);
}

