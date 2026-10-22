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
#include <stdio.h>
#include <time.h>
#include "BaseLib/TextFile.h"
#include "BaseLib/Logger.h"
#include "BaseLib/FastFunctions.h"
#include "BaseLib/Numbers.h"
#include "PreprocessorExpression.h"
#include "Punctuators.h"
#include "Project.h"
#include "Preprocessor.h"
#include "PPText.h"
#include "PPReplacement.h"
#include "PPConditional.h"
#include "TokenHelper.h"
#include "GeneralToken.h"
#include "PreprocessorPosition.h"
#include "PPTokenMemory.h"


#define MAX_INCLUDE_DEPTH	200


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPPSource::Init(EPPSourceType eType)
{
	this->eType = eType;
	acTokens.Init();
	iToken = 0;
	pcFile = NULL;
	pcIncludingFile = NULL;
	iBlockSet = 0;
	iLine = 0;
	iConditionalDepth = 0;
	bHeaderDirectory = false;
	iCurrentLine = 0;
	lliDefineID = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPPSource::Kill(void)
{
	acTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::Preprocess(char* szSource, CChars* pszDest)
{
	CPreprocessor		cPreprocessor;
	CTranslationUnit	cFile;
	bool				bResult;

	cFile.Init("", NULL, false);
	cFile.SetContents(szSource);

	cPreprocessor.Init(NULL);
	bResult = cPreprocessor.PreprocessTranslationUnit(&cFile);

	if (pszDest)
	{
		cFile.Print(pszDest);
	}

	cPreprocessor.Kill();
	cFile.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::Kill(void)
{
	size	i;

	for (i = 0; i < macSources.NumElements(); i++)
	{
		macSources.Get(i)->Kill();
	}
	macSources.Kill();
	macPushedBack.Kill();
	maiDisabledDefines.Kill();
	mpcOutput = NULL;

	mszVaArgs.Kill();
	mpcCurrentFile = NULL;
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
bool CPreprocessor::PreprocessTranslationUnit(CTranslationUnit* pcFile)
{
	bool				bResult;
	SPreprocessedToken	sToken;

	TranslationUnitLogging(pcFile);

	mpcOutput = pcFile->GetTokens();
	mpcOutput->Clear();
	mbError = false;
	muiPendingFlags = 0;

	bResult = PushFile(pcFile, NULL);
	if (bResult)
	{
		LogInclude(pcFile);
		while (Next(&sToken))
		{
			EmitToken(&sToken);
		}
		bResult = !mbError;
	}

	while (macSources.NumElements() > 0)
	{
		bResult &= PopSource();
	}
	macPushedBack.Kill();
	macPushedBack.Init();

	mpcOutput = NULL;
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

	//A file is tokenised once and the tokens are read every time it is included.
	pcFile->Load();
	pacBlockSets = pcFile->GetBlockSets();

	if (!pacBlockSets->IsRawProcessed())
	{
		cTokeniser.Init(pcFile->ShortName());
		cTokeniser.SetComments(pcFile->GetComments());
		bResult = cTokeniser.TokeniseIntoBlockSets(pacBlockSets, pcFile->GetTokenMemory(), pcFile->GetContents(), (int)pcFile->GetContentsLength());
		cTokeniser.Kill();
		return bResult;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::PushFile(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap)
{
	SPPSource*					psSource;
	CHeaderNameMapDirectory*	pcNewDirectory;
	CChars						szPath;
	CChars						szError;

	if (miIncludeDepth >= MAX_INCLUDE_DEPTH)
	{
		szError.Init("#include nested too deeply including ");
		szError.Append(pcFile->ShortName());
		gcUserError.Set(&szError);
		szError.Kill();
		mbError = true;
		return false;
	}

	if (!TokeniseFile(pcFile))
	{
		mbError = true;
		return false;
	}

	psSource = macSources.Push();
	psSource->Init(PPST_File);
	psSource->pcFile = pcFile;
	psSource->pcIncludingFile = mpcCurrentFile;
	psSource->iConditionalDepth = mcConditionalStack.mcStack.NumElements();

	if (pcHeaderNameMap)
	{
		//Quoted includes in this file are also searched for relative to its directory.
		pcNewDirectory = mcHeadersStack.Add();
		pcFile->Path(&szPath);
		pcNewDirectory->Init(pcHeaderNameMap, szPath.Text());
		szPath.Kill();
		psSource->bHeaderDirectory = true;
	}

	mpcCurrentFile = pcFile;
	miIncludeDepth++;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::PushTokens(EPPSourceType eType, CArrayPreprocessedTokens* pacTokens, int64 lliDefineID)
{
	SPPSource*	psSource;

	//The source takes the tokens.  pacTokens is left empty.
	psSource = macSources.Push();
	psSource->Init(eType);
	psSource->acTokens.Kill();
	memcpy(&psSource->acTokens, pacTokens, sizeof(CArrayPreprocessedTokens));
	pacTokens->Init();

	if (eType == PPST_Macro)
	{
		psSource->lliDefineID = lliDefineID;
		maiDisabledDefines.Add(&lliDefineID);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::PopSource(void)
{
	SPPSource*					psSource;
	CHeaderNameMapDirectory*	pcDirectory;
	bool						bResult;
	CChars						szError;

	psSource = macSources.Tail();
	if (!psSource)
	{
		return false;
	}

	bResult = true;
	if (psSource->eType == PPST_File)
	{
		if (mcConditionalStack.mcStack.NumElements() > psSource->iConditionalDepth)
		{
			if (!mbError)
			{
				szError.Init("Unterminated conditional directive in ");
				szError.Append(psSource->pcFile->ShortName());
				gcUserError.Set(&szError);
				szError.Kill();
			}
			while (mcConditionalStack.mcStack.NumElements() > psSource->iConditionalDepth)
			{
				mcConditionalStack.PopEndIf();
			}
			bResult = false;
		}

		if (psSource->bHeaderDirectory)
		{
			pcDirectory = mcHeadersStack.Tail();
			pcDirectory->Kill();
			mcHeadersStack.Pop();
		}

		mpcCurrentFile = psSource->pcIncludingFile;
		miIncludeDepth--;
	}
	else if (psSource->eType == PPST_Macro)
	{
		maiDisabledDefines.Pop();
	}

	psSource->Kill();
	macSources.Pop();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::PushBack(SPreprocessedToken* psToken)
{
	macPushedBack.Push(psToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::NextRaw(SPreprocessedToken* psToken)
{
	SPPSource*	psSource;
	bool		bChanged;
	bool		bResult;

	//Reads the next token without expanding macros.  Returns false at the end of the translation unit, at the end of a
	//macro argument being expanded, or on an error.
	if (macPushedBack.NumElements() > 0)
	{
		macPushedBack.Pop(psToken);
		return true;
	}

	for (;;)
	{
		if (mbError)
		{
			return false;
		}

		psSource = macSources.Tail();
		if (!psSource)
		{
			return false;
		}

		if (psSource->iToken < psSource->acTokens.NumElements())
		{
			*psToken = *psSource->acTokens.Get(psSource->iToken);
			psSource->iToken++;
			return true;
		}

		if (psSource->eType == PPST_Argument)
		{
			return false;
		}
		else if (psSource->eType == PPST_Macro)
		{
			PopSource();
		}
		else
		{
			bResult = NextFileToken(psSource, psToken, &bChanged);
			if (!bResult)
			{
				if (mbError)
				{
					return false;
				}
				if (!PopSource())
				{
					mbError = true;
					return false;
				}
			}
			else if (!bChanged)
			{
				return true;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::NextFileToken(SPPSource* psSource, SPreprocessedToken* psToken, bool* pbChanged)
{
	CPPBlockSetArray*	pacBlockSets;
	CPPBlockSet*		pcBlockSet;
	CPPTokenList*		pcLines;
	CPPToken*			pcToken;
	CSourceFile*		pcFile;
	bool				bResult;
	size				iNumSources;

	//Moves the file on to its next line with tokens.  Directives are processed on the way.  Returns false at the end of
	//the file.  *pbChanged is true if a directive changed the sources (an #include).  psSource must be the top source
	//and is not valid after the call.
	*pbChanged = false;
	pcFile = psSource->pcFile;
	pacBlockSets = pcFile->GetBlockSets();
	for (;;)
	{
		pcBlockSet = pacBlockSets->SafeGet(psSource->iBlockSet);
		if (!pcBlockSet)
		{
			return false;
		}

		pcLines = pcBlockSet->GetUnprocssedTokens();
		if (psSource->iLine >= pcLines->NumTokens())
		{
			psSource->iBlockSet++;
			psSource->iLine = 0;
			continue;
		}

		pcToken = pcLines->Get(psSource->iLine);
		psSource->iLine++;

		if (pcToken->IsDirective())
		{
			iNumSources = macSources.NumElements();
			bResult = ProcessDirective((CPPDirective*)pcToken);
			if (!bResult)
			{
				mbError = true;
				return false;
			}
			if (macSources.NumElements() != iNumSources)
			{
				*pbChanged = true;
				return true;
			}

			//Evaluating a #if can grow the source stack and move it.
			psSource = macSources.Tail();
		}
		else if (pcToken->IsLine())
		{
			if (!mcConditionalStack.IsParsing())
			{
				mpcOutput->AddInactiveLine(pcFile, pcToken->Line());
				continue;
			}

			psSource->acTokens.Kill();
			psSource->acTokens.Init();
			psSource->iToken = 0;
			psSource->iCurrentLine = pcToken->Line();
			ConvertTokens(((CPPLine*)pcToken)->GetTokenList(), pcFile, NULL, true, &psSource->acTokens);
			if (psSource->acTokens.NumElements() > 0)
			{
				*psToken = *psSource->acTokens.Get(0);
				psSource->iToken = 1;
				return true;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
SPPSource* CPreprocessor::GetCurrentFileSource(void)
{
	size		i;
	SPPSource*	psSource;

	for (i = macSources.NumElements(); i > 0; i--)
	{
		psSource = macSources.Get(i - 1);
		if (psSource->eType == PPST_File)
		{
			return psSource;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
static bool IsStringPrefix(const char* sz, size uiLength)
{
	if (uiLength == 1)
	{
		return (sz[0] == 'L') || (sz[0] == 'u') || (sz[0] == 'U') || (sz[0] == 'R');
	}
	if (uiLength == 2)
	{
		return ((sz[0] == 'u') && (sz[1] == '8')) || (((sz[0] == 'L') || (sz[0] == 'u') || (sz[0] == 'U')) && (sz[1] == 'R'));
	}
	if (uiLength == 3)
	{
		return (sz[0] == 'u') && (sz[1] == '8') && (sz[2] == 'R');
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ConvertTokens(CPPTokenList* pcTokens, CSourceFile* pcFile, CDefine* pcDefine, bool bStartOfLine, CArrayPreprocessedTokens* pacDest)
{
	size				i;
	size				iNumTokens;
	CPPToken*			pcToken;
	CPPToken*			pcNext;
	CPPText*			pcText;
	CPPText*			pcNextText;
	CPPHashes*			pcHashes;
	CPPReplacement*		pcReplacement;
	SPreprocessedToken	sToken;
	uint8				uiFlags;
	size				iCount;
	int					iColumn;
	CChars*				pszName;
	CChars				szMerged;
	EPreprocessedTokenKind	eKind;

	//Turns the tokeniser's tokens into preprocessed tokens: whitespace becomes a flag, single character decorators become
	//punctuators, string prefixes join their strings and, in a macro body, parameters and # and ## are marked.
	uiFlags = bStartOfLine ? PTF_START_OF_LINE : 0;
	iNumTokens = pcTokens->NumTokens();
	for (i = 0; i < iNumTokens;)
	{
		pcToken = pcTokens->Get(i);
		if (pcToken->IsWhitespace())
		{
			uiFlags |= PTF_LEADING_SPACE;
			i++;
		}
		else if (pcToken->IsHash())
		{
			pcHashes = (CPPHashes*)pcToken;
			iCount = pcHashes->miCount;
			iColumn = pcToken->Column();
			while (iCount > 0)
			{
				if (iCount >= 2)
				{
					sToken.Init(PTK_Punctuator, "##", 2, pcFile, pcToken->Line(), iColumn);
					sToken.uiFlags = uiFlags | (pcDefine ? PTF_PASTE : 0);
					iCount -= 2;
					iColumn += 2;
				}
				else
				{
					sToken.Init(PTK_Punctuator, "#", 1, pcFile, pcToken->Line(), iColumn);
					sToken.uiFlags = uiFlags | (pcDefine ? PTF_STRINGIFY : 0);
					iCount--;
					iColumn++;
				}
				pacDest->Add(&sToken);
				uiFlags = 0;
			}
			i++;
		}
		else if (pcToken->IsReplacement() && pcDefine)
		{
			pcReplacement = (CPPReplacement*)pcToken;
			pszName = pcDefine->GetArguments()->Get(pcReplacement->miArgIndex);
			sToken.Init(PTK_Identifier, pszName->Text(), pszName->Length(), pcFile, pcToken->Line(), pcToken->Column());
			sToken.iParameter = (int16)pcReplacement->miArgIndex;
			sToken.uiFlags = uiFlags;
			pacDest->Add(&sToken);
			uiFlags = 0;
			i++;
		}
		else if (pcToken->IsText())
		{
			pcText = (CPPText*)pcToken;
			if (pcText->meType == PPT_Decorator)
			{
				ConvertDecorators(pcTokens, &i, pcFile, uiFlags, pacDest);
				uiFlags = 0;
				continue;
			}

			pcNext = (i + 1 < iNumTokens) ? pcTokens->Get(i + 1) : NULL;
			if ((pcText->meType == PPT_Identifier) && pcNext && pcNext->IsText() && IsStringPrefix(pcText->TextStart(), pcText->Length()))
			{
				pcNextText = (CPPText*)pcNext;
				if ((pcNextText->meType == PPT_DoubleQuoted) || (pcNextText->meType == PPT_SingleQuoted))
				{
					//The prefix of L"x" or u8"x" is part of the string.
					eKind = (pcNextText->meType == PPT_DoubleQuoted) ? PTK_String : PTK_Character;
					if (pcText->TextStart() + pcText->Length() == pcNextText->TextStart())
					{
						sToken.Init(eKind, pcText->TextStart(), pcText->Length() + pcNextText->Length(), pcFile, pcToken->Line(), pcToken->Column());
					}
					else
					{
						szMerged.Init();
						szMerged.Append(pcText->TextStart(), pcText->Length());
						szMerged.Append(pcNextText->TextStart(), pcNextText->Length());
						sToken.Init(eKind, AddString(szMerged.Text(), szMerged.Length()), szMerged.Length(), pcFile, pcToken->Line(), pcToken->Column());
						szMerged.Kill();
					}
					sToken.uiFlags = uiFlags;
					pacDest->Add(&sToken);
					uiFlags = 0;
					i += 2;
					continue;
				}
			}

			switch (pcText->meType)
			{
			case PPT_Identifier:
				eKind = PTK_Identifier;
				break;
			case PPT_Number:
				eKind = PTK_Number;
				break;
			case PPT_DoubleQuoted:
				eKind = PTK_String;
				break;
			case PPT_SingleQuoted:
				eKind = PTK_Character;
				break;
			default:
				eKind = PTK_Other;
				break;
			}
			sToken.Init(eKind, pcText->TextStart(), pcText->Length(), pcFile, pcToken->Line(), pcToken->Column());
			sToken.uiFlags = uiFlags;
			pacDest->Add(&sToken);
			uiFlags = 0;
			i++;
		}
		else
		{
			i++;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessor::ConvertDecorators(CPPTokenList* pcTokens, size* piIndex, CSourceFile* pcFile, uint8 uiFlags, CArrayPreprocessedTokens* pacDest)
{
	char				szChars[4];
	CPPText*			apcTexts[4];
	size				iNumChars;
	size				i;
	size				iLength;
	CPPToken*			pcToken;
	CPPText*			pcText;
	SPreprocessedToken	sToken;
	const char*			szText;
	bool				bContiguous;
	EPreprocessedTokenKind	eKind;

	//Decorators are single characters.  The longest punctuator they spell is taken, as in <<= or ->*.
	for (;;)
	{
		iNumChars = 0;
		for (i = *piIndex; (i < pcTokens->NumTokens()) && (iNumChars < 4); i++)
		{
			pcToken = pcTokens->Get(i);
			if (!pcToken->IsText())
			{
				break;
			}
			pcText = (CPPText*)pcToken;
			if ((pcText->meType != PPT_Decorator) || (pcText->Length() != 1))
			{
				break;
			}
			szChars[iNumChars] = pcText->TextStart()[0];
			apcTexts[iNumChars] = pcText;
			iNumChars++;
		}

		if (iNumChars == 0)
		{
			//Not a single character decorator.
			pcText = (CPPText*)pcTokens->Get(*piIndex);
			sToken.Init(PTK_Other, pcText->TextStart(), pcText->Length(), pcFile, pcText->Line(), pcText->Column());
			sToken.uiFlags = uiFlags;
			pacDest->Add(&sToken);
			(*piIndex)++;
			return NULL;
		}

		iLength = PunctuatorLength(szChars, iNumChars);
		eKind = PTK_Punctuator;
		if (iLength == 0)
		{
			iLength = 1;
			eKind = PTK_Other;
		}

		bContiguous = true;
		for (i = 1; i < iLength; i++)
		{
			if (apcTexts[i]->TextStart() != apcTexts[0]->TextStart() + i)
			{
				bContiguous = false;
			}
		}
		szText = bContiguous ? apcTexts[0]->TextStart() : AddString(szChars, iLength);

		sToken.Init(eKind, szText, iLength, pcFile, apcTexts[0]->Line(), apcTexts[0]->Column());
		sToken.uiFlags = uiFlags;
		pacDest->Add(&sToken);
		uiFlags = 0;
		*piIndex += iLength;

		//Whitespace or another kind of token ends the run of decorators.
		if (*piIndex >= pcTokens->NumTokens())
		{
			return NULL;
		}
		pcToken = pcTokens->Get(*piIndex);
		if (!pcToken->IsText() || (((CPPText*)pcToken)->meType != PPT_Decorator))
		{
			return pcToken;
		}
	}
}


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
	mbLogInlucdes = false;
	mbDumpLogs = false;
	mpszIncludesLog = NULL;

	mcConditionalStack.Init();
	mpcCurrentFile = NULL;
	mcHeadersStack.Init();  //Current file should be included in the stack.

	macSources.Init();
	macPushedBack.Init();
	maiDisabledDefines.Init();
	mpcOutput = NULL;
	muiPendingFlags = 0;
	mbInCondition = false;
	mbDefinedOperand = false;
	mbError = false;
	miCounter = 0;
	mcHeaderNames.Init();

	AddSpecialDefine("__DATE__");
	AddSpecialDefine("__TIME__");
	AddSpecialDefine("__FILE__");
	AddSpecialDefine("__LINE__");
	AddSpecialDefine("__TIMESTAMP__");
	AddSpecialDefine("__COUNTER__");

	//C++14.  __func__ and __FUNCTION__ are identifiers, not macros.  __STDC__ is left to the configuration.
	AddDefine("__cplusplus", "201402L", &mcDefineReplacementMemory);
	AddDefine("__STDC_HOSTED__", "1", &mcDefineReplacementMemory);

	AddSpecialOperator("defined", PPSO_Defined);
	AddSpecialOperator("__has_attribute", PPSO_HasAttribute);
	AddSpecialOperator("__has_cpp_attribute", PPSO_HasCppAttribute);
	AddSpecialOperator("__has_builtin", PPSO_HasBuiltIn);
	AddSpecialOperator("__has_include", PPSO_HasInclude);
	AddSpecialOperator("__has_include_next", PPSO_HasIncludeNext);
	AddSpecialOperator("__has_feature", PPSO_HasFeature);
	AddSpecialOperator("__has_extension", PPSO_HasFeature);

	InitPlatformSpecific();
	AddConfigDefines(pcConfig);
	mszVaArgs.Init("__VA_ARGS__");
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
	mcHeaderNames.Add(pcHeaderMapName);
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
		
		pcTokenMemory = GetTokenMemory();
		pcDefine->SetFile(mpcCurrentFile);

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
					if (iReplaceArg != ARRAY_ELEMENT_NOT_FOUND)
					{
						pcReplacement = pcTokenMemory->AddReplacement();
						pcReplacement->Init(pcDefine->GetID(), (int)iReplaceArg, -1, -1, gszEmptyString, bVariadic && (iReplacementNum == iNumArguments - 1));

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
static bool IsPathPrefix(char* szPath, char* szBase, size iBaseLength)
{
	size	i;
	char	c1;
	char	c2;

	//Windows paths: case is ignored and either separator matches.
	for (i = 0; i < iBaseLength; i++)
	{
		c1 = szPath[i];
		c2 = szBase[i];
		if (c1 == '\\')
		{
			c1 = '/';
		}
		if (c2 == '\\')
		{
			c2 = '/';
		}
		if ((c1 >= 'A') && (c1 <= 'Z'))
		{
			c1 += 'a' - 'A';
		}
		if ((c2 >= 'A') && (c2 <= 'Z'))
		{
			c2 += 'a' - 'A';
		}
		if (c1 != c2)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::FindBestInclude(CExternalString* pcInclude, bool bSystemFile, CHeaderFile** ppcCFile, CHeaderFiles** ppcHeaderNameMap, bool bNext)
{
	CChars						szInclude;
	CChars						szPath;
	CChars						szRelative;
	size						i;
	size						j;
	size						iFirst;
	size						iBaseLength;
	CHeaderFiles*				pcHeaderNameMap;
	CHeaderFile*				pcHeaderFile;
	CHeaderNameMapDirectory*	pcCurrentDirectory;
	SPPSource*					psSource;
	char*						szBase;

	*ppcCFile = NULL;
	*ppcHeaderNameMap = NULL;

	szInclude.Init(pcInclude->msz, 0, pcInclude->miLen);
	szInclude.Replace('\\', '/');

	//A quoted include is looked for in the directory of the file that includes it, then in the directories of the files
	//that included that (as MSVC does), then in the include directories.  [cpp.include] 3
	if (!bSystemFile && !bNext)
	{
		for (i = macSources.NumElements(); i > 0; i--)
		{
			psSource = macSources.Get(i - 1);
			if (psSource->eType != PPST_File)
			{
				continue;
			}

			psSource->pcFile->Path(&szPath);
			szPath.Replace('\\', '/');
			for (j = 0; j < mcHeaderNames.NumElements(); j++)
			{
				pcHeaderNameMap = *mcHeaderNames.Get(j);
				szBase = pcHeaderNameMap->GetBaseDirectory();
				iBaseLength = pcHeaderNameMap->GetBaseDirectoryLength();
				if ((szPath.Length() >= iBaseLength) && IsPathPrefix(szPath.Text(), szBase, iBaseLength) && ((szPath.Length() == iBaseLength) || (szPath.GetChar(iBaseLength) == '/') || (szPath.GetChar(iBaseLength) == '\\')))
				{
					szRelative.Init();
					if (szPath.Length() > iBaseLength)
					{
						szRelative.Append(szPath.Text(iBaseLength + 1));
						szRelative.Append('/');
					}
					szRelative.Append(szInclude);
					pcHeaderFile = pcHeaderNameMap->GetFile(szRelative.Text());
					szRelative.Kill();
					if (pcHeaderFile)
					{
						*ppcCFile = pcHeaderFile;
						*ppcHeaderNameMap = pcHeaderNameMap;
						szPath.Kill();
						szInclude.Kill();
						return;
					}
				}
			}
			szPath.Kill();
		}
	}

	//#include_next only searches the include directories after the one the current file was found in.
	iFirst = 0;
	if (bNext)
	{
		pcCurrentDirectory = mcHeadersStack.Tail();
		if (pcCurrentDirectory)
		{
			for (i = 0; i < mcHeaderNames.NumElements(); i++)
			{
				if (*mcHeaderNames.Get(i) == pcCurrentDirectory->mpcHeaderNameMap)
				{
					iFirst = i + 1;
					break;
				}
			}
		}
	}

	for (i = iFirst; i < mcHeaderNames.NumElements(); i++)
	{
		pcHeaderNameMap = *mcHeaderNames.Get(i);
		pcHeaderFile = pcHeaderNameMap->GetFile(szInclude.Text());
		if (pcHeaderFile)
		{
			*ppcCFile = pcHeaderFile;
			*ppcHeaderNameMap = pcHeaderNameMap;
			break;
		}
	}
	szInclude.Kill();
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
void CPreprocessor::LogDumping(bool bDumpLogs)
{
	mbDumpLogs = bDumpLogs;
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
bool CPreprocessor::ProcessHashUnknown(CPPDirective* pcDirective)
{
	CChars					szError;
	SPreprocessorPosition	sPos;

	//Unknown directives are only an error when they are not skipped by a conditional.
	MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
	sPos.Message(&szError);
	szError.Append("Unknown directive #");
	pcDirective->GetTokenList()->Print(&szError);
	szError.StripWhitespace(true);
	return gcUserError.Set(&szError);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE CPreprocessor::EvaluateEquation(char* szText, CChars* szCalculatorError)
{
	return CPreprocessorExpression::Evaluate(szText, szCalculatorError);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddConfigDefines(CConfig* pcConfig)
{
	size		i;
	CChars*		pszText;
	size		iEquals;
	CChars		szName;

	if (pcConfig)
	{
		for (i = 0; i < pcConfig->maszDefines.NumElements(); i++)
		{
			//A define can have a value, as in "NAME=VALUE".
			pszText = pcConfig->maszDefines.Get(i);
			iEquals = pszText->Find('=');
			if (iEquals == ARRAY_ELEMENT_NOT_FOUND)
			{
				szName.Init(pszText->Text());
			}
			else
			{
				szName.Init(pszText->Text(), 0, iEquals);
			}

			//The configuration replaces predefined macros.
			mcDefines.RemoveDefine(szName.Text());
			if (iEquals == ARRAY_ELEMENT_NOT_FOUND)
			{
				mcDefines.AddDefine(szName.Text());
			}
			else
			{
				AddDefine(szName.Text(), pszText->Text(iEquals + 1), &mcDefineReplacementMemory);
			}
			szName.Kill();
		}
	}
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
void CPreprocessor::TranslationUnitLogging(CTranslationUnit* pcFile)
{
	if (pcFile->LogInlucdes())
	{
		LogIncludes(&pcFile->GetLogs()->szIncludesLog);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CPreprocessor::DuplicatePPToken(CPPToken* pcSource)
{
	CPPTokenMemory*		pcTokenMemory;

	pcTokenMemory = GetTokenMemory();
	return ::DuplicatePPToken(pcSource, pcTokenMemory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::Next(SPreprocessedToken* psToken)
{
	CDefine*						pcDefine;
	CArrayPreprocessedTokenArrays	acArguments;
	bool							bResult;
	size							i;
	uint8							uiSpacing;

	//Returns the next token after macro replacement.  [cpp.replace]
	for (;;)
	{
		if (!NextRaw(psToken))
		{
			return false;
		}

		if (muiPendingFlags)
		{
			psToken->uiFlags |= muiPendingFlags;
			muiPendingFlags = 0;
		}

		if (mbInCondition)
		{
			//A defined made by a macro: its operand is not replaced, as g++ and MSVC do.  (Strictly this is undefined.)
			if (mbDefinedOperand)
			{
				if (psToken->IsPunctuator("("))
				{
					return true;
				}
				mbDefinedOperand = false;
				if (psToken->uiKind == PTK_Identifier)
				{
					psToken->uiFlags |= PTF_NO_EXPAND;
					return true;
				}
			}
			else if (psToken->Is("defined"))
			{
				mbDefinedOperand = true;
				return true;
			}
		}

		if ((psToken->uiKind != PTK_Identifier) || (psToken->uiFlags & PTF_NO_EXPAND))
		{
			return true;
		}

		pcDefine = GetDefine(psToken);
		if (!pcDefine)
		{
			if (!mbInCondition && psToken->Is("_Pragma"))
			{
				uiSpacing = psToken->uiFlags & (PTF_LEADING_SPACE | PTF_START_OF_LINE);
				if (SkipPragmaOperator())
				{
					muiPendingFlags |= uiSpacing;
					continue;
				}
			}
			return true;
		}

		if (IsDisabled(pcDefine->GetID()))
		{
			//A macro is not replaced inside its own replacement, now or ever.  [cpp.rescan]
			psToken->uiFlags |= PTF_NO_EXPAND;
			return true;
		}

		if (pcDefine->IsSpecial())
		{
			ExpandSpecial(pcDefine, psToken);
			return true;
		}

		acArguments.Init();
		if (pcDefine->IsBacketed())
		{
			//A function-like macro name not followed by ( is not an invocation.
			if (!PeekOpenParenthesis())
			{
				acArguments.Kill();
				return true;
			}

			bResult = CollectArguments(pcDefine, psToken, &acArguments);
		}
		else
		{
			bResult = true;
		}

		if (bResult)
		{
			bResult = ExpandMacro(pcDefine, psToken, &acArguments);
		}

		for (i = 0; i < acArguments.NumElements(); i++)
		{
			acArguments.Get(i)->Kill();
		}
		acArguments.Kill();

		if (!bResult)
		{
			mbError = true;
			return false;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::PeekOpenParenthesis(void)
{
	SPreprocessedToken	sToken;

	if (!NextRaw(&sToken))
	{
		return false;
	}

	if (sToken.IsPunctuator("("))
	{
		return true;
	}

	PushBack(&sToken);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::CollectArguments(CDefine* pcDefine, SPreprocessedToken* psName, CArrayPreprocessedTokenArrays* pacArguments)
{
	SPreprocessedToken			sToken;
	CArrayPreprocessedTokens*	pacArgument;
	size						iNumParameters;
	size						iNumArguments;
	bool						bVariadic;
	int							iDepth;
	CChars						szError;

	//The opening ( has been read.  Arguments are read without expanding them and can cover many lines.
	iNumParameters = pcDefine->GetArguments()->NumElements();
	bVariadic = pcDefine->IsVariadic();

	pacArgument = pacArguments->Add();
	pacArgument->Init();
	iDepth = 1;
	for (;;)
	{
		if (!NextRaw(&sToken))
		{
			if (!mbError)
			{
				szError.Init("Unterminated argument list invoking macro ");
				szError.Append(pcDefine->GetName());
				Error(psName, szError.Text());
				szError.Kill();
			}
			return false;
		}

		if (sToken.uiFlags & PTF_START_OF_LINE)
		{
			//A new line in an argument is whitespace.
			sToken.uiFlags = (sToken.uiFlags & ~PTF_START_OF_LINE) | PTF_LEADING_SPACE;
		}

		if (sToken.uiKind == PTK_Punctuator)
		{
			if (sToken.IsPunctuator("("))
			{
				iDepth++;
			}
			else if (sToken.IsPunctuator(")"))
			{
				iDepth--;
				if (iDepth == 0)
				{
					break;
				}
			}
			else if ((iDepth == 1) && sToken.IsPunctuator(",") && !(bVariadic && (pacArguments->NumElements() >= iNumParameters)))
			{
				//The variadic argument includes its commas.
				pacArgument = pacArguments->Add();
				pacArgument->Init();
				continue;
			}
		}

		pacArgument = pacArguments->Tail();
		pacArgument->Add(&sToken);
	}

	iNumArguments = pacArguments->NumElements();
	if ((iNumParameters == 0) && (iNumArguments == 1) && (pacArguments->Get(0)->NumElements() == 0))
	{
		//F() calls a macro with no parameters.
		pacArguments->Get(0)->Kill();
		pacArguments->Pop();
		iNumArguments = 0;
	}

	if (bVariadic && (iNumArguments + 1 == iNumParameters))
	{
		//The variadic argument may be left out.
		pacArgument = pacArguments->Add();
		pacArgument->Init();
		iNumArguments++;
	}

	if (iNumArguments != iNumParameters)
	{
		szError.Init("Macro ");
		szError.Append(pcDefine->GetName());
		szError.Append(" takes ");
		szError.Append((int)iNumParameters);
		szError.Append(" arguments but ");
		szError.Append((int)iNumArguments);
		szError.Append(" were given.");
		Error(psName, szError.Text());
		szError.Kill();
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ExpandMacro(CDefine* pcDefine, SPreprocessedToken* psName, CArrayPreprocessedTokenArrays* pacArguments)
{
	CArrayPreprocessedTokens*		pacBody;
	CArrayPreprocessedTokens		acResult;
	CArrayPreprocessedTokens		acPasted;
	CArrayPreprocessedTokens		acFinal;
	CArrayPreprocessedTokenArrays	acExpanded;
	CArrayPreprocessedTokens*		pacArgument;
	SPreprocessedToken*				psBody;
	SPreprocessedToken*				psToken;
	SPreprocessedToken*				psLeft;
	SPreprocessedToken				sToken;
	size							i;
	size							j;
	size							iNumBody;
	size							iNumParameters;
	int								iExpansion;
	int								iParameter;
	bool							bFunctionLike;
	bool							bRaw;

	pacBody = GetBody(pcDefine);
	iNumBody = pacBody->NumElements();
	iNumParameters = pacArguments->NumElements();
	bFunctionLike = pcDefine->IsBacketed();
	iExpansion = mpcOutput->AddExpansion(psName->szText, psName->uiLength, psName->pcFile, psName->iLine, psName->iColumn, psName->iExpansion);

	//Arguments are macro replaced once, when first needed.
	acExpanded.Init();
	for (i = 0; i < iNumParameters; i++)
	{
		acExpanded.Add()->Init();
	}

	//Substitute the parameters.  [cpp.subst]
	acResult.Init();
	for (i = 0; i < iNumBody; i++)
	{
		psBody = pacBody->Get(i);
		if (bFunctionLike && (psBody->uiFlags & PTF_STRINGIFY) && (i + 1 < iNumBody) && (pacBody->Get(i + 1)->iParameter >= 0))
		{
			//#parameter.  [cpp.stringize]
			Stringify(pacArguments->Get(pacBody->Get(i + 1)->iParameter), &sToken);
			sToken.uiFlags = psBody->uiFlags & PTF_LEADING_SPACE;
			sToken.iExpansion = iExpansion;
			acResult.Add(&sToken);
			i++;
			continue;
		}

		if ((psBody->uiFlags & PTF_PASTE) && pcDefine->IsVariadic() && (i > 0) && (i + 1 < iNumBody) && pacBody->Get(i - 1)->IsPunctuator(",") && (pacBody->Get(i - 1)->iParameter < 0) && (pacBody->Get(i + 1)->iParameter == (int)iNumParameters - 1))
		{
			//GNU and Microsoft: in ", ## __VA_ARGS__" the comma is removed when there are no variadic arguments.
			pacArgument = pacArguments->Get(iNumParameters - 1);
			if (pacArgument->NumElements() == 0)
			{
				acResult.Pop();
			}
			else
			{
				for (j = 0; j < pacArgument->NumElements(); j++)
				{
					sToken = *pacArgument->Get(j);
					if (j == 0)
					{
						sToken.uiFlags = (sToken.uiFlags & ~(PTF_LEADING_SPACE | PTF_START_OF_LINE)) | (pacBody->Get(i + 1)->uiFlags & PTF_LEADING_SPACE);
					}
					acResult.Add(&sToken);
				}
			}
			i++;
			continue;
		}

		if (psBody->iParameter >= 0)
		{
			iParameter = psBody->iParameter;

			//An argument next to ## is not macro replaced.  [cpp.concat]
			bRaw = ((i + 1 < iNumBody) && (pacBody->Get(i + 1)->uiFlags & PTF_PASTE)) || ((i > 0) && (pacBody->Get(i - 1)->uiFlags & PTF_PASTE));
			if (bRaw)
			{
				pacArgument = pacArguments->Get(iParameter);
			}
			else
			{
				pacArgument = acExpanded.Get(iParameter);
				if ((pacArgument->NumElements() == 0) && (pacArguments->Get(iParameter)->NumElements() > 0))
				{
					ExpandArgument(pacArguments->Get(iParameter), pacArgument);
					if (mbError)
					{
						break;
					}
				}
			}

			if (pacArgument->NumElements() == 0)
			{
				sToken.Init(PTK_Placemarker, "", 0, NULL, -1, -1);
				sToken.uiFlags = psBody->uiFlags & PTF_LEADING_SPACE;
				sToken.iExpansion = iExpansion;
				acResult.Add(&sToken);
			}
			else
			{
				for (j = 0; j < pacArgument->NumElements(); j++)
				{
					sToken = *pacArgument->Get(j);
					if (j == 0)
					{
						sToken.uiFlags = (sToken.uiFlags & ~(PTF_LEADING_SPACE | PTF_START_OF_LINE)) | (psBody->uiFlags & PTF_LEADING_SPACE);
					}
					acResult.Add(&sToken);
				}
			}
			continue;
		}

		sToken = *psBody;
		sToken.iExpansion = iExpansion;
		sToken.iParameter = -1;
		acResult.Add(&sToken);
	}

	for (i = 0; i < acExpanded.NumElements(); i++)
	{
		acExpanded.Get(i)->Kill();
	}
	acExpanded.Kill();

	if (mbError)
	{
		acResult.Kill();
		return false;
	}

	//Paste.  ## is only an operator if it came from the body.  [cpp.concat]
	acPasted.Init();
	for (i = 0; i < acResult.NumElements(); i++)
	{
		psToken = acResult.Get(i);
		if (psToken->uiFlags & PTF_PASTE)
		{
			if ((acPasted.NumElements() > 0) && (i + 1 < acResult.NumElements()))
			{
				psLeft = acPasted.Tail();
				if (Paste(psLeft, acResult.Get(i + 1), &sToken))
				{
					*psLeft = sToken;
				}
				else
				{
					//Not a valid token.  The compilers that allow it keep the two tokens.
					acPasted.Add(acResult.Get(i + 1));
				}
				i++;
			}
			continue;
		}
		acPasted.Add(psToken);
	}
	acResult.Kill();

	acFinal.Init();
	for (i = 0; i < acPasted.NumElements(); i++)
	{
		psToken = acPasted.Get(i);
		if (psToken->uiKind != PTK_Placemarker)
		{
			sToken = *psToken;
			sToken.uiFlags &= ~(PTF_PASTE | PTF_STRINGIFY | PTF_START_OF_LINE);
			acFinal.Add(&sToken);
		}
	}
	acPasted.Kill();

	if (mbInCondition && (acFinal.NumElements() == 0) && !bFunctionLike)
	{
		//An object-like macro with no replacement is 0 in #if.  (Strictly this is an error.)
		AddNumber(0, psName, &acFinal);
		acFinal.Tail()->iExpansion = iExpansion;
	}

	//The expansion is spaced like the macro name was.
	if (acFinal.NumElements() > 0)
	{
		psToken = acFinal.Get(0);
		psToken->uiFlags = (psToken->uiFlags & ~PTF_LEADING_SPACE) | (psName->uiFlags & (PTF_LEADING_SPACE | PTF_START_OF_LINE));
	}
	else
	{
		muiPendingFlags |= psName->uiFlags & (PTF_LEADING_SPACE | PTF_START_OF_LINE);
	}

	//Rescan.  [cpp.rescan]
	PushTokens(PPST_Macro, &acFinal, pcDefine->GetID());
	acFinal.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ExpandArgument(CArrayPreprocessedTokens* pacRaw, CArrayPreprocessedTokens* pacExpanded)
{
	CArrayPreprocessedTokens	acCopy;

	acCopy.Init();
	acCopy.Copy(pacRaw);
	ExpandTokens(&acCopy, pacExpanded);
	acCopy.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ExpandTokens(CArrayPreprocessedTokens* pacSource, CArrayPreprocessedTokens* pacDest)
{
	CArrayPreprocessedTokens	acPushedBack;
	SPreprocessedToken			sToken;
	size						iNumSources;
	uint8						uiPendingFlags;
	bool						bDefinedOperand;

	//Macro replaces the tokens on their own, "as if they formed the rest of the preprocessing file".  [cpp.subst]
	//pacSource is taken (left empty).
	memcpy(&acPushedBack, &macPushedBack, sizeof(CArrayPreprocessedTokens));
	macPushedBack.Init();
	uiPendingFlags = muiPendingFlags;
	muiPendingFlags = 0;
	bDefinedOperand = mbDefinedOperand;
	mbDefinedOperand = false;

	iNumSources = macSources.NumElements();
	PushTokens(PPST_Argument, pacSource, -1);
	while (Next(&sToken))
	{
		pacDest->Add(&sToken);
	}

	while (macSources.NumElements() > iNumSources)
	{
		PopSource();
	}

	macPushedBack.Kill();
	memcpy(&macPushedBack, &acPushedBack, sizeof(CArrayPreprocessedTokens));
	muiPendingFlags = uiPendingFlags;
	mbDefinedOperand = bDefinedOperand;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::Stringify(CArrayPreprocessedTokens* pacArgument, SPreprocessedToken* psResult)
{
	CChars				sz;
	size				i;
	size				j;
	SPreprocessedToken*	psToken;
	char				c;

	//Whitespace between tokens becomes one space.  " and \ in string and character literals are escaped.  [cpp.stringize]
	sz.Init("\"");
	for (i = 0; i < pacArgument->NumElements(); i++)
	{
		psToken = pacArgument->Get(i);
		if ((i > 0) && (psToken->uiFlags & (PTF_LEADING_SPACE | PTF_START_OF_LINE)))
		{
			sz.Append(' ');
		}

		if ((psToken->uiKind == PTK_String) || (psToken->uiKind == PTK_Character))
		{
			for (j = 0; j < psToken->uiLength; j++)
			{
				c = psToken->szText[j];
				if ((c == '"') || (c == '\\'))
				{
					sz.Append('\\');
				}
				sz.Append(c);
			}
		}
		else
		{
			psToken->Append(&sz);
		}
	}
	sz.Append('"');

	psResult->Init(PTK_String, mpcOutput->AddString(sz.Text(), sz.Length()), sz.Length(), NULL, -1, -1);
	psResult->uiFlags = PTF_OUTPUT_TEXT;
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
static bool IsIdentifierStart(char c)
{
	return ((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')) || (c == '_') || (c == '$');
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
static bool IsIdentifierCharacter(char c)
{
	return IsIdentifierStart(c) || ((c >= '0') && (c <= '9'));
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
static size QuotedLength(const char* sz, size uiLength)
{
	size	i;
	char	cQuote;

	//The length of the literal starting at sz[0], 0 if it is not terminated.
	cQuote = sz[0];
	for (i = 1; i < uiLength; i++)
	{
		if (sz[i] == '\\')
		{
			i++;
		}
		else if (sz[i] == cQuote)
		{
			return i + 1;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
static int LexSingleToken(const char* sz, size uiLength)
{
	size	i;
	char	c;

	//The kind of the one preprocessing token sz spells, -1 if it is not exactly one token.
	if (uiLength == 0)
	{
		return -1;
	}

	c = sz[0];
	if (IsIdentifierStart(c))
	{
		for (i = 1; (i < uiLength) && IsIdentifierCharacter(sz[i]); i++)
		{
		}
		if (i == uiLength)
		{
			return PTK_Identifier;
		}
		if (IsStringPrefix(sz, i) && ((sz[i] == '"') || (sz[i] == '\'')))
		{
			if (i + QuotedLength(&sz[i], uiLength - i) == uiLength)
			{
				return (sz[i] == '"') ? PTK_String : PTK_Character;
			}
		}
		return -1;
	}

	if (((c >= '0') && (c <= '9')) || ((c == '.') && (uiLength > 1) && (sz[1] >= '0') && (sz[1] <= '9')))
	{
		for (i = 1; i < uiLength; i++)
		{
			c = sz[i];
			if (IsIdentifierCharacter(c) || (c == '.') || (c == '\''))
			{
				continue;
			}
			if (((c == '+') || (c == '-')) && ((sz[i - 1] == 'e') || (sz[i - 1] == 'E') || (sz[i - 1] == 'p') || (sz[i - 1] == 'P')))
			{
				continue;
			}
			return -1;
		}
		return PTK_Number;
	}

	if ((c == '"') || (c == '\''))
	{
		if (QuotedLength(sz, uiLength) == uiLength)
		{
			return (c == '"') ? PTK_String : PTK_Character;
		}
		return -1;
	}

	if (PunctuatorLength(sz, uiLength) == uiLength)
	{
		return PTK_Punctuator;
	}
	return -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::Paste(SPreprocessedToken* psLeft, SPreprocessedToken* psRight, SPreprocessedToken* psResult)
{
	CChars	sz;
	int		iKind;

	if (psLeft->uiKind == PTK_Placemarker)
	{
		*psResult = *psRight;
		psResult->uiFlags = (psRight->uiFlags & ~PTF_LEADING_SPACE) | (psLeft->uiFlags & PTF_LEADING_SPACE);
		return true;
	}

	if (psRight->uiKind == PTK_Placemarker)
	{
		*psResult = *psLeft;
		return true;
	}

	sz.Init();
	psLeft->Append(&sz);
	psRight->Append(&sz);
	iKind = LexSingleToken(sz.Text(), sz.Length());
	if (iKind < 0)
	{
		sz.Kill();
		return false;
	}

	//A pasted token is a new token; it may be a macro name.
	*psResult = *psLeft;
	psResult->szText = mpcOutput->AddString(sz.Text(), sz.Length());
	psResult->uiLength = (uint32)sz.Length();
	psResult->uiKind = (uint8)iKind;
	psResult->uiFlags = (psLeft->uiFlags & PTF_LEADING_SPACE) | PTF_OUTPUT_TEXT;
	sz.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::ExpandSpecial(CDefine* pcDefine, SPreprocessedToken* psToken)
{
	SPPSource*		psSource;
	CChars			sz;
	char*			szName;
	char*			szFileName;
	time_t			tNow;
	struct tm*		psTime;
	char			szTime[64];
	size			i;
	static char*	aszMonths[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static char*	aszDays[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };
	EPreprocessedTokenKind	eKind;
	SMacroExpansion*		psExpansion;
	int						iLine;

	//[cpp.predefined]  Macros the compiler would make, like __FUNCSIG__, are left as identifiers.
	szName = pcDefine->GetName();
	sz.Init();
	eKind = PTK_Number;
	if (strcmp(szName, "__LINE__") == 0)
	{
		//The line of the outermost macro name written in the source, or of __LINE__ itself, as g++ does.
		iLine = psToken->iLine;
		psExpansion = mpcOutput->GetExpansion(psToken->iExpansion);
		while (psExpansion)
		{
			iLine = psExpansion->iLine;
			psExpansion = mpcOutput->GetExpansion(psExpansion->iParent);
		}
		if (iLine < 0)
		{
			psSource = GetCurrentFileSource();
			iLine = psSource ? psSource->iCurrentLine : 0;
		}
		sz.Append(iLine + 1);
	}
	else if (strcmp(szName, "__COUNTER__") == 0)
	{
		sz.Append(miCounter);
		miCounter++;
	}
	else if (strcmp(szName, "__FILE__") == 0)
	{
		eKind = PTK_String;
		szFileName = mpcCurrentFile ? mpcCurrentFile->FullName() : gszEmptyString;
		sz.Append('"');
		for (i = 0; szFileName[i] != '\0'; i++)
		{
			if ((szFileName[i] == '\\') || (szFileName[i] == '"'))
			{
				sz.Append('\\');
			}
			sz.Append(szFileName[i]);
		}
		sz.Append('"');
	}
	else if ((strcmp(szName, "__DATE__") == 0) || (strcmp(szName, "__TIME__") == 0) || (strcmp(szName, "__TIMESTAMP__") == 0))
	{
		eKind = PTK_String;
		tNow = time(NULL);
		psTime = localtime(&tNow);
		if (strcmp(szName, "__DATE__") == 0)
		{
			snprintf(szTime, 64, "\"%s %2d %d\"", aszMonths[psTime->tm_mon], psTime->tm_mday, psTime->tm_year + 1900);
		}
		else if (strcmp(szName, "__TIME__") == 0)
		{
			snprintf(szTime, 64, "\"%02d:%02d:%02d\"", psTime->tm_hour, psTime->tm_min, psTime->tm_sec);
		}
		else
		{
			snprintf(szTime, 64, "\"%s %s %2d %02d:%02d:%02d %d\"", aszDays[psTime->tm_wday], aszMonths[psTime->tm_mon], psTime->tm_mday, psTime->tm_hour, psTime->tm_min, psTime->tm_sec, psTime->tm_year + 1900);
		}
		sz.Append(szTime);
	}
	else
	{
		sz.Kill();
		psToken->uiFlags |= PTF_NO_EXPAND;
		return;
	}

	psToken->szText = mpcOutput->AddString(sz.Text(), sz.Length());
	psToken->uiLength = (uint32)sz.Length();
	psToken->uiKind = (uint8)eKind;
	psToken->uiFlags |= PTF_OUTPUT_TEXT;
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::SkipPragmaOperator(void)
{
	SPreprocessedToken	sToken;
	int					iDepth;

	//_Pragma("...") is the operator form of #pragma.  [cpp.pragma.op]  The pragma is ignored.
	if (!PeekOpenParenthesis())
	{
		return false;
	}

	iDepth = 1;
	while (NextRaw(&sToken))
	{
		if (sToken.IsPunctuator("("))
		{
			iDepth++;
		}
		else if (sToken.IsPunctuator(")"))
		{
			iDepth--;
			if (iDepth == 0)
			{
				return true;
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CArrayPreprocessedTokens* CPreprocessor::GetBody(CDefine* pcDefine)
{
	if (!pcDefine->IsBodyConverted())
	{
		ConvertTokens(pcDefine->GetReplacement()->GetTokenList(), pcDefine->GetFile(), pcDefine, false, pcDefine->GetBody());
		pcDefine->SetBodyConverted();
	}
	return pcDefine->GetBody();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CPreprocessor::GetDefine(SPreprocessedToken* psToken)
{
	CExternalString		cName;

	cName.Init((char*)psToken->szText, (int)psToken->uiLength);
	return mcDefines.GetDefine(&cName, true);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::IsDisabled(int64 lliDefineID)
{
	size	i;

	for (i = maiDisabledDefines.NumElements(); i > 0; i--)
	{
		if (*maiDisabledDefines.Get(i - 1) == lliDefineID)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::IsDefined(SPreprocessedToken* psToken)
{
	CExternalString		cName;

	//The special operators, like __has_include, count as defined so that headers can test for them.
	cName.Init((char*)psToken->szText, (int)psToken->uiLength);
	if (mcDefines.GetDefine(&cName, true))
	{
		return true;
	}
	return mcSpecialOperators.GetSpecialOperator(&cName, true) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::AddNumber(int64 lliValue, SPreprocessedToken* psPosition, CArrayPreprocessedTokens* pacDest)
{
	SPreprocessedToken	sToken;
	CChars				sz;

	sz.Init();
	sz.Append(lliValue);
	sToken = *psPosition;
	sToken.uiKind = PTK_Number;
	sToken.szText = mpcOutput->AddString(sz.Text(), sz.Length());
	sToken.uiLength = (uint32)sz.Length();
	sToken.uiFlags |= PTF_OUTPUT_TEXT;
	sToken.iParameter = -1;
	pacDest->Add(&sToken);
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::Error(SPreprocessedToken* psToken, char* szMessage)
{
	CChars	sz;

	sz.Init();
	if (psToken && psToken->pcFile)
	{
		sz.Append(psToken->pcFile->ShortName());
		sz.Append(" (");
		sz.Append(psToken->iLine + 1);
		sz.Append(", ");
		sz.Append(psToken->iColumn + 1);
		sz.Append("): ");
	}
	else
	{
		sz.Append(GetFileName());
		sz.Append(": ");
	}
	sz.Append(szMessage);
	gcUserError.Set(&sz);
	sz.Kill();

	mbError = true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CPreprocessor::EmitToken(SPreprocessedToken* psToken)
{
	char*	szContents;
	size	iLength;

	//The output can outlive the preprocessor and #undef'd macros, so its tokens' text must be in a source file or in
	//the output.
	if (!(psToken->uiFlags & PTF_OUTPUT_TEXT))
	{
		szContents = psToken->pcFile ? psToken->pcFile->GetContents() : NULL;
		iLength = psToken->pcFile ? psToken->pcFile->GetContentsLength() : 0;
		if (!szContents || (psToken->szText < szContents) || (psToken->szText + psToken->uiLength > szContents + iLength))
		{
			psToken->szText = mpcOutput->AddString(psToken->szText, psToken->uiLength);
			psToken->uiFlags |= PTF_OUTPUT_TEXT;
		}
	}
	mpcOutput->Add(psToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
char* CPreprocessor::AddString(const char* szText, size uiLength)
{
	char*	sz;

	//Text that lasts as long as the preprocessor.
	sz = (char*)mcDefineStrings.Add(uiLength + 1);
	memcpy(sz, szText, uiLength);
	sz[uiLength] = '\0';
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPTokenMemory* CPreprocessor::GetTokenMemory(void)
{
	if (mpcCurrentFile)
	{
		return mpcCurrentFile->GetTokenMemory();
	}
	return &mcDefineReplacementMemory;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessDirective(CPPDirective* pcDirective)
{
	CPreprocessorTokenParser	cParser;
	bool						bResult;

	if (pcDirective->IsConditional())
	{
		mpcOutput->AddDirectiveLine(mpcCurrentFile, pcDirective->Line(), true);
		return ProcessConditional(pcDirective);
	}

	if (!mcConditionalStack.IsParsing())
	{
		mpcOutput->AddDirectiveLine(mpcCurrentFile, pcDirective->Line(), false);
		mpcOutput->AddInactiveLine(mpcCurrentFile, pcDirective->Line());
		return true;
	}
	mpcOutput->AddDirectiveLine(mpcCurrentFile, pcDirective->Line(), true);

	bResult = true;
	cParser.Init(pcDirective);
	if (pcDirective->Is(PPD_define))
	{
		bResult = ProcessHashDefine(&cParser);
	}
	else if (pcDirective->Is(PPD_undef))
	{
		bResult = ProcessHashUndef(&cParser);
	}
	else if (pcDirective->Is(PPD_include))
	{
		bResult = ProcessHashInclude(pcDirective, false);
	}
	else if (pcDirective->Is(PPD_include_next))
	{
		bResult = ProcessHashInclude(pcDirective, true);
	}
	else if (pcDirective->Is(PPD_error))
	{
		bResult = ProcessHashError(&cParser);
	}
	else if (pcDirective->Is(PPD_pragma))
	{
		bResult = ProcessHashPragma(&cParser);
	}
	else if (pcDirective->Is(PPD_unknown))
	{
		bResult = ProcessHashUnknown(pcDirective);
	}
	cParser.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessConditional(CPPDirective* pcDirective)
{
	CPreprocessorTokenParser	cParser;
	CExternalString				cIdentifier;
	SPreprocessedToken			sToken;
	TRISTATE					tResult;
	bool						bDefined;
	SPPSource*					psSource;
	size						iFileDepth;
	SPreprocessorPosition		sPos;
	CChars						szError;

	//A conditional is inactive if the group it is in is skipped.  #elif, #else and #endif are in their #if's group.
	if (pcDirective->Is(PPD_if) || pcDirective->Is(PPD_ifdef) || pcDirective->Is(PPD_ifndef))
	{
		if (!mcConditionalStack.IsParsing())
		{
			mpcOutput->AddInactiveLine(mpcCurrentFile, pcDirective->Line());
		}
	}
	else if (!mcConditionalStack.IsParentParsing())
	{
		mpcOutput->AddInactiveLine(mpcCurrentFile, pcDirective->Line());
	}

	psSource = GetCurrentFileSource();
	iFileDepth = psSource ? psSource->iConditionalDepth : 0;

	if (pcDirective->Is(PPD_ifdef) || pcDirective->Is(PPD_ifndef))
	{
		if (!mcConditionalStack.IsParsing())
		{
			mcConditionalStack.PushIf(false);
			return true;
		}

		cParser.Init(pcDirective);
		cParser.SkipWhitespace();
		if (!cParser.GetIdentifier(&cIdentifier))
		{
			cParser.Kill();
			MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
			sPos.Message(&szError);
			szError.Append("Expected an identifier.");
			gcUserError.Set(&szError);
			szError.Kill();
			return false;
		}
		cParser.Kill();

		sToken.Init(PTK_Identifier, cIdentifier.msz, cIdentifier.miLen, NULL, -1, -1);
		bDefined = IsDefined(&sToken);
		if (pcDirective->Is(PPD_ifdef))
		{
			mcConditionalStack.PushIfDefined(bDefined);
		}
		else
		{
			mcConditionalStack.PushIfNotDefined(!bDefined);
		}
		return true;
	}

	if (pcDirective->Is(PPD_if))
	{
		if (!mcConditionalStack.IsParsing())
		{
			//The expressions of conditionals in skipped groups are not evaluated.
			mcConditionalStack.PushIf(false);
			return true;
		}

		tResult = EvaluateCondition(pcDirective);
		if (tResult == TRIERROR)
		{
			return false;
		}
		mcConditionalStack.PushIf(tResult == TRITRUE);
		return true;
	}

	if (mcConditionalStack.mcStack.NumElements() <= iFileDepth)
	{
		MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
		sPos.Message(&szError);
		szError.Append("#elif, #else or #endif without #if.");
		gcUserError.Set(&szError);
		szError.Kill();
		return false;
	}

	if (pcDirective->Is(PPD_elif))
	{
		if (!mcConditionalStack.IsElseIfEvaluated())
		{
			mcConditionalStack.SwapForElseIf(false);
			return true;
		}

		tResult = EvaluateCondition(pcDirective);
		if (tResult == TRIERROR)
		{
			return false;
		}
		mcConditionalStack.SwapForElseIf(tResult == TRITRUE);
		return true;
	}

	if (pcDirective->Is(PPD_else))
	{
		mcConditionalStack.SwapForElse();
		return true;
	}

	if (pcDirective->Is(PPD_endif))
	{
		mcConditionalStack.PopEndIf();
		return true;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE CPreprocessor::EvaluateCondition(CPPDirective* pcDirective)
{
	CArrayPreprocessedTokens	acRaw;
	CArrayPreprocessedTokens	acReplaced;
	CArrayPreprocessedTokens	acExpanded;
	CArrayPreprocessedTokens	acFinal;
	CChars						sz;
	CChars						szCalculatorError;
	CChars						szError;
	TRISTATE					tResult;
	size						i;
	SPreprocessorPosition		sPos;
	bool						bResult;

	//[cpp.cond]  defined and the __has_ operators are evaluated before macro replacement and again after it, for
	//macros that make them.  Identifiers left over are 0, except true and false.
	acRaw.Init();
	acReplaced.Init();
	acExpanded.Init();
	acFinal.Init();
	ConvertTokens(pcDirective->GetTokenList(), mpcCurrentFile, NULL, false, &acRaw);

	bResult = ReplaceConditionOperators(&acRaw, &acReplaced);
	if (bResult)
	{
		mbInCondition = true;
		ExpandTokens(&acReplaced, &acExpanded);
		mbInCondition = false;
		bResult = !mbError;
	}
	if (bResult)
	{
		bResult = ReplaceConditionOperators(&acExpanded, &acFinal);
	}

	if (!bResult)
	{
		acRaw.Kill();
		acReplaced.Kill();
		acExpanded.Kill();
		acFinal.Kill();
		return TRIERROR;
	}

	sz.Init();
	for (i = 0; i < acFinal.NumElements(); i++)
	{
		if (i > 0)
		{
			sz.Append(' ');
		}
		acFinal.Get(i)->Append(&sz);
	}

	szCalculatorError.Init();
	tResult = EvaluateEquation(sz.Text(), &szCalculatorError);
	if (tResult == TRIERROR)
	{
		MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
		sPos.Message(&szError);
		szError.Append("Cannot evaluate \"");
		szError.Append(sz);
		szError.Append("\".  ");
		szError.Append(szCalculatorError);
		gcUserError.Set(szError.Text());
		szError.Kill();
	}

	szCalculatorError.Kill();
	sz.Kill();
	acRaw.Kill();
	acReplaced.Kill();
	acExpanded.Kill();
	acFinal.Kill();
	return tResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
static bool SkipParenthesised(CArrayPreprocessedTokens* pacTokens, size* piIndex)
{
	size	i;
	int		iDepth;

	//pacTokens[*piIndex] is '('.  Moves *piIndex past the matching ')'.
	iDepth = 0;
	for (i = *piIndex; i < pacTokens->NumElements(); i++)
	{
		if (pacTokens->Get(i)->IsPunctuator("("))
		{
			iDepth++;
		}
		else if (pacTokens->Get(i)->IsPunctuator(")"))
		{
			iDepth--;
			if (iDepth == 0)
			{
				*piIndex = i + 1;
				return true;
			}
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ReplaceConditionOperators(CArrayPreprocessedTokens* pacSource, CArrayPreprocessedTokens* pacDest)
{
	size					i;
	size					iNumTokens;
	SPreprocessedToken*		psToken;
	SPreprocessedToken*		psNext;
	CSpecialOperator*		pcSpecial;
	CExternalString			cName;
	bool					bParenthesis;
	CChars					szName;
	bool					bSystem;
	CHeaderFile*			pcIncludeFile;
	CHeaderFiles*			pcHeaderNameMap;
	EPreprocessorSpecialOperator	eType;

	iNumTokens = pacSource->NumElements();
	for (i = 0; i < iNumTokens;)
	{
		psToken = pacSource->Get(i);
		if (psToken->uiKind != PTK_Identifier)
		{
			pacDest->Add(psToken);
			i++;
			continue;
		}

		cName.Init((char*)psToken->szText, (int)psToken->uiLength);
		pcSpecial = mcSpecialOperators.GetSpecialOperator(&cName, true);
		psNext = pacSource->SafeGet(i + 1);
		if (pcSpecial)
		{
			eType = pcSpecial->GetType();
			if (eType == PPSO_Defined)
			{
				//defined X or defined ( X )
				i++;
				bParenthesis = psNext && psNext->IsPunctuator("(");
				if (bParenthesis)
				{
					i++;
				}
				psNext = pacSource->SafeGet(i);
				if (!psNext || (psNext->uiKind != PTK_Identifier))
				{
					return Error(psToken, "Expected an identifier after defined.");
				}
				AddNumber(IsDefined(psNext) ? 1 : 0, psToken, pacDest);
				i++;
				if (bParenthesis)
				{
					psNext = pacSource->SafeGet(i);
					if (!psNext || !psNext->IsPunctuator(")"))
					{
						return Error(psToken, "Expected ')' after defined.");
					}
					i++;
				}
				continue;
			}

			if (!psNext || !psNext->IsPunctuator("("))
			{
				return Error(psToken, "Expected '(' after __has_ operator.");
			}

			if ((eType == PPSO_HasInclude) || (eType == PPSO_HasIncludeNext))
			{
				i += 2;
				szName.Init();
				if (!GetIncludeName(pacSource, &i, &szName, &bSystem))
				{
					szName.Kill();
					return Error(psToken, "Expected a header name after __has_include.");
				}
				cName.Init(szName.Text(), (int)szName.Length());
				FindBestInclude(&cName, bSystem, &pcIncludeFile, &pcHeaderNameMap, eType == PPSO_HasIncludeNext);
				szName.Kill();
				AddNumber(pcIncludeFile ? 1 : 0, psToken, pacDest);

				psNext = pacSource->SafeGet(i);
				if (!psNext || !psNext->IsPunctuator(")"))
				{
					return Error(psToken, "Expected ')' after __has_include.");
				}
				i++;
				continue;
			}

			//Attributes, builtins and features are not known.
			i++;
			if (!SkipParenthesised(pacSource, &i))
			{
				return Error(psToken, "Expected ')'.");
			}
			AddNumber(0, psToken, pacDest);
			continue;
		}

		if (psNext && psNext->IsPunctuator("(") && !GetDefine(psToken))
		{
			//An unknown function-like name, as in __has_feature(x) without a definition, is 0.
			i++;
			if (!SkipParenthesised(pacSource, &i))
			{
				return Error(psToken, "Expected ')'.");
			}
			AddNumber(0, psToken, pacDest);
			continue;
		}

		pacDest->Add(psToken);
		i++;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::GetIncludeName(CArrayPreprocessedTokens* pacTokens, size* piIndex, CChars* pszName, bool* pbSystem)
{
	SPreprocessedToken*		psToken;
	size					i;

	//"name" or <name>.  A <name> made of several tokens is spelled as they were, with their spacing.
	psToken = pacTokens->SafeGet(*piIndex);
	if (!psToken)
	{
		return false;
	}

	if ((psToken->uiKind == PTK_String) && (psToken->szText[0] == '"') && (psToken->uiLength >= 2))
	{
		pszName->Append(psToken->szText + 1, psToken->uiLength - 2);
		*pbSystem = false;
		(*piIndex)++;
		return true;
	}

	if (psToken->IsPunctuator("<"))
	{
		for (i = *piIndex + 1; i < pacTokens->NumElements(); i++)
		{
			psToken = pacTokens->Get(i);
			if (psToken->IsPunctuator(">"))
			{
				*pbSystem = true;
				*piIndex = i + 1;
				return true;
			}
			if ((i > *piIndex + 1) && psToken->HasLeadingSpace())
			{
				pszName->Append(' ');
			}
			psToken->Append(pszName);
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CPreprocessor::ProcessHashInclude(CPPDirective* pcDirective, bool bNext)
{
	CArrayPreprocessedTokens	acRaw;
	CArrayPreprocessedTokens	acExpanded;
	CArrayPreprocessedTokens*	pacName;
	SPreprocessedToken*			psFirst;
	CHeaderFile*				pcIncludeFile;
	CHeaderFiles*				pcHeaderNameMap;
	CExternalString				cName;
	CChars						szName;
	bool						bSystem;
	bool						bResult;
	size						i;
	SPreprocessorPosition		sPos;
	CChars						szError;

	acRaw.Init();
	acExpanded.Init();
	ConvertTokens(pcDirective->GetTokenList(), mpcCurrentFile, NULL, false, &acRaw);

	//#include MACRO is macro replaced first.  [cpp.include]
	pacName = &acRaw;
	psFirst = acRaw.SafeGet(0);
	if (psFirst && !psFirst->IsPunctuator("<") && (psFirst->uiKind != PTK_String))
	{
		ExpandTokens(&acRaw, &acExpanded);
		pacName = &acExpanded;
	}

	i = 0;
	szName.Init();
	bResult = GetIncludeName(pacName, &i, &szName, &bSystem);
	acRaw.Kill();
	acExpanded.Kill();

	MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
	if (!bResult)
	{
		szName.Kill();
		sPos.Message(&szError);
		szError.Append("Expected \"file\" or <file> after #include.");
		gcUserError.Set(&szError);
		szError.Kill();
		return false;
	}

	cName.Init(szName.Text(), (int)szName.Length());
	FindBestInclude(&cName, bSystem, &pcIncludeFile, &pcHeaderNameMap, bNext);
	if (!pcIncludeFile)
	{
		sPos.Message(&szError);
		szError.Append("Could not include file ");
		szError.Append(bSystem ? '<' : '"');
		szError.Append(szName);
		szError.Append(bSystem ? '>' : '"');
		gcUserError.Set(&szError);
		szError.Kill();
		szName.Kill();
		return false;
	}
	szName.Kill();

	if (pcIncludeFile->IsPragmaOnced())
	{
		miIncludeDepth++;
		LogInclude(pcIncludeFile);
		miIncludeDepth--;
		return true;
	}

	bResult = PushFile(pcIncludeFile, pcHeaderNameMap);
	if (bResult)
	{
		LogInclude(pcIncludeFile);
	}
	return bResult;
}

