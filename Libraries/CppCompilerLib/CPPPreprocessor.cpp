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
#include <stdio.h>
#include <time.h>
#include "BaseLib/FileUtil.h"
#include "BaseLib/TextFile.h"
#include "BaseLib/Logger.h"
#include <string.h>
#include "BaseLib/Numbers.h"
#include "BaseLib/StringHelper.h"
#include "PreprocessorExpression.h"
#include "Punctuators.h"
#include "Project.h"
#include "CPPPreprocessor.h"
#include "PPText.h"
#include "PPReplacement.h"
#include "PPConditional.h"
#include "TokenHelper.h"
#include "GeneralToken.h"
#include "PreprocessorPosition.h"
#include "PPTokenMemory.h"
#include "CPPNameCharacters.h"
#include "LiteralHelper.h"


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
	iLineOffset = 0;
	szLineFile.Init();
	uiDirectory = 0;
	uiDirectories = 0;
	bRecorded = false;
	lliDefineID = -1;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void SPPSource::Kill(void)
{
	szLineFile.Kill();
	acTokens.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::Preprocess(char* szSource, CChars* pszDest)
{
	CCPPPreprocessor		cPreprocessor;
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
void CCPPPreprocessor::Kill(void)
{
	size	i;

	for (i = 0; i < macSources.NumElements(); i++)
	{
		macSources.Get(i)->Kill();
	}
	macSources.Kill();
	for (i = 0; i < masRecorders.NumElements(); i++)
	{
		KillRecorder(masRecorders.Get(i), false);
	}
	masRecorders.Kill();
	if (mpcHeaderCache)
	{
		mpcHeaderCache->Count(muiHeaderHits, muiHeaderMisses, muiReplayedTokens);
	}
	macPushedBack.Kill();
	maiDisabledDefines.Kill();
	mpcOutput = NULL;

	mszVaArgs.Kill();
	mpcCurrentFile = NULL;
	mcHeadersStack.Kill();
	mcConditionalStack.Kill();
	mcPragmaOnce.Kill();
	mcHeaderNames.Kill();
	mcSpecialOperators.Kill();

	mcDefines.Kill();
	mcDefineReplacementMemory.Kill();
	mcDefineStrings.Kill();
	mcErrors.Kill();
	mcWarnings.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::PreprocessTranslationUnit(CTranslationUnit* pcFile)
{
	bool				bResult;
	SPreprocessedToken	sToken;

	TranslationUnitLogging(pcFile);

	mpcOutput = pcFile->GetTokens();
	mpcOutput->SetFullPathErrors(mbFullPathErrors);
	mpcOutput->Clear();
	mbError = false;
	mbSkippedDirective = false;
	muiPendingFlags = 0;

	bResult = PushFile(pcFile, NULL);
	if (bResult)
	{
		LogInclude(pcFile);
		while (Next(&sToken))
		{
			EmitToken(&sToken);
		}
		bResult = !mbError && !mbSkippedDirective;
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
//
// With bSkip a directive that fails (an #include that can't be found, an
// #if that can't be evaluated, an #error) is skipped and preprocessing
// goes on after it, for a code model, which wants as much of the unit as
// it can get.  Its error is still reported and the unit still fails.
// Without it (as a compiler wants) the unit ends there.
//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::SetSkipFailedDirectives(bool bSkip)
{
	mbSkipFailedDirectives = bSkip;
}


//////////////////////////////////////////////////////////////////////////
//
// With bFullPaths errors and warnings name the file they are in by its
// full path rather than its name: the same name can be in more than one
// include directory (stdarg.h is in MinGW's and in GCC's), and a code
// model opens the one it means.  A #line's name is still what it says.
// The unit's tokens are made so too, so the parser's and the check's
// errors at them name their files as these do.
//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::SetFullPathErrors(bool bFullPaths)
{
	mbFullPathErrors = bFullPaths;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::TokeniseFile(CSourceFile* pcFile)
{
	CPreprocessorTokeniser	cTokeniser;
	bool					bResult;
	CPPBlockSetArray*		pacBlockSets;

	//A file is tokenised once and the tokens are read every time it is included.  Units preprocessed at the same time
	//(CCPP65Builder::SetJobs) include the same headers, so it is locked while it is.
	bResult = true;
	pcFile->Lock();
	if (!pcFile->Load())
	{
		pcFile->Unlock();
		mcErrors.Add(ErrorFileName(pcFile), -1, -1, -1, "Could not read the file.");
		return false;
	}
	pacBlockSets = pcFile->GetBlockSets();
	if (!pacBlockSets->IsRawProcessed())
	{
		//Its trigraphs are replaced first.  Its lines are found now (NumLines finds them the first time), not when the
		//generator first asks, so they are only read after.  [lex.phases] 1.1
		pcFile->ReplaceTrigraphs();
		pcFile->NumLines();
		cTokeniser.Init(pcFile->ShortName());
		cTokeniser.SetComments(pcFile->GetComments());
		bResult = cTokeniser.TokeniseIntoBlockSets(pacBlockSets, pcFile->GetTokenMemory(), pcFile->GetContents(), (int)pcFile->GetContentsLength());
		cTokeniser.Kill();
	}
	pcFile->Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::PushFile(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap)
{
	SPPSource*					psSource;
	CHeaderNameMapDirectory*	pcNewDirectory;
	CChars						szError;
	uint64						uiIncludingDirectories;
	uint64						uiDirectory;

	if (miIncludeDepth >= MAX_INCLUDE_DEPTH)
	{
		szError.Init("#include nested too deeply including ");
		szError.Append(pcFile->ShortName());
		mcErrors.Add(GetErrorFileName(), -1, -1, -1, szError.Text());
		szError.Kill();
		mbError = true;
		return false;
	}

	if (!TokeniseFile(pcFile))
	{
		mbError = true;
		return false;
	}

	uiIncludingDirectories = 0;
	uiDirectory = 0;
	if (mpcHeaderCache)
	{
		psSource = GetCurrentFileSource();
		uiDirectory = DirectoriesHash(pcFile, 0);
		if (!psSource)
		{
			uiIncludingDirectories = uiDirectory;
		}
		else if (psSource->uiDirectory == uiDirectory)
		{
			//Looking in a directory again finds nothing new.
			uiIncludingDirectories = psSource->uiDirectories;
		}
		else
		{
			uiIncludingDirectories = DirectoriesHash(pcFile, psSource->uiDirectories);
		}
	}

	psSource = macSources.Push();
	psSource->Init(PPST_File);
	psSource->pcFile = pcFile;
	psSource->pcIncludingFile = mpcCurrentFile;
	psSource->uiDirectories = uiIncludingDirectories;
	psSource->uiDirectory = uiDirectory;
	psSource->iConditionalDepth = mcConditionalStack.mcStack.NumElements();

	if (pcHeaderNameMap)
	{
		//The include directory the file was found in, where #include_next in it starts after (FindBestInclude).
		pcNewDirectory = mcHeadersStack.Add();
		pcNewDirectory->Init(pcHeaderNameMap);
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
void CCPPPreprocessor::PushTokens(EPPSourceType eType, CArrayPreprocessedTokens* pacTokens, int64 lliDefineID)
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
bool CCPPPreprocessor::PopSource(void)
{
	SPPSource*					psSource;
	CHeaderNameMapDirectory*	pcDirectory;
	bool						bResult;

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
				mcErrors.Add(ErrorFileName(psSource->pcFile), -1, -1, -1, "Unterminated conditional directive.");
			}
			while (mcConditionalStack.mcStack.NumElements() > psSource->iConditionalDepth)
			{
				mcConditionalStack.PopEndIf();
			}
			bResult = false;
		}

		//A header being kept for the header cache is finished (not kept if it ended in an error).
		if (psSource->bRecorded)
		{
			if (!bResult || mbError)
			{
				InvalidateRecordings();
			}
			EndRecording();
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
void CCPPPreprocessor::PushBack(SPreprocessedToken* psToken)
{
	macPushedBack.Push(psToken);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::NextRaw(SPreprocessedToken* psToken)
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
bool CCPPPreprocessor::NextFileToken(SPPSource* psSource, SPreprocessedToken* psToken, bool* pbChanged)
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
				if (!mbSkipFailedDirectives)
				{
					mbError = true;
					return false;
				}

				//Its error stays reported and the unit carries on after it.  The sources are read again from the top,
				//as the directive may have moved them.
				mbError = false;
				mbInCondition = false;
				mbDefinedOperand = false;
				mbSkippedDirective = true;
				*pbChanged = true;
				return true;
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
				AddInactiveLine(pcFile, pcToken->Line());
				continue;
			}

			//The line's tokens replace the last line's in the same memory (the array is emptied, not freed).  Not a
			//CReuseAllocator: it keeps one block, and sources nest (an #include's lines are read while the including
			//file's array still holds its own).
			psSource->acTokens.FakeSetUsedElements(0);
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
SPPSource* CCPPPreprocessor::GetCurrentFileSource(void)
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
//
// How much of a string or character token is the literal, when what
// follows its closing quote is not its user-defined suffix ("text"_s) but
// a macro: as g++ has it, a macro that doesn't start with an underscore,
// so that "%" PRId64 can be written without the space, as it was before
// there were suffixes.  0 when the token is all one literal.  [lex.ext]
//
//////////////////////////////////////////////////////////////////////////
size CCPPPreprocessor::LiteralBeforeMacro(CPPText* pcLiteral)
{
	size				uiLength;
	size				uiTokenLength;
	CExternalString		cName;

	uiTokenLength = (size)pcLiteral->Length();
	uiLength = QuotedLiteralLength(pcLiteral->TextStart(), uiTokenLength);
	if ((uiLength == 0) || (uiLength == uiTokenLength) || (pcLiteral->TextStart()[uiLength] == '_'))
	{
		return 0;
	}
	cName.Init(pcLiteral->TextStart() + uiLength, uiTokenLength - uiLength);
	return mcDefines.Get(&cName) ? uiLength : 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::ConvertTokens(CPPTokenList* pcTokens, CSourceFile* pcFile, CDefine* pcDefine, bool bStartOfLine, CArrayPreprocessedTokens* pacDest)
{
	size				i;
	size				iNumTokens;
	CPPToken*			pcToken;
	CPPText*			pcText;
	CPPHashes*			pcHashes;
	CPPReplacement*		pcReplacement;
	SPreprocessedToken	sToken;
	uint8				uiFlags;
	size				iCount;
	size				uiLiteral;
	int					iColumn;
	CChars*				pszName;
	EPreprocessedTokenKind	eKind;

	//Turns the tokeniser's tokens into preprocessed tokens: whitespace becomes a flag, single character decorators become
	//punctuators, a macro touching the end of a string is parted from it and, in a macro body, parameters and # and ##
	//are marked.
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
					sToken.Init(PTK_Punctuator, pcHashes->mbDigraph ? "%:%:" : "##", pcHashes->mbDigraph ? 4 : 2, pcFile, pcToken->Line(), iColumn);
					sToken.uiFlags = uiFlags | (pcDefine ? PTF_PASTE : 0);
					iCount -= 2;
					iColumn += sToken.uiLength;
				}
				else
				{
					sToken.Init(PTK_Punctuator, pcHashes->mbDigraph ? "%:" : "#", pcHashes->mbDigraph ? 2 : 1, pcFile, pcToken->Line(), iColumn);
					sToken.uiFlags = uiFlags | (pcDefine ? PTF_STRINGIFY : 0);
					iCount--;
					iColumn += sToken.uiLength;
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
			uiLiteral = ((eKind == PTK_String) || (eKind == PTK_Character)) ? LiteralBeforeMacro(pcText) : 0;
			if (uiLiteral != 0)
			{
				//The literal, then the macro's name as a token of its own.
				sToken.uiLength = (uint32)uiLiteral;
				pacDest->Add(&sToken);
				sToken.Init(PTK_Identifier, pcText->TextStart() + uiLiteral, pcText->Length() - uiLiteral, pcFile, pcToken->Line(), pcToken->Column() + (int)uiLiteral);
			}
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
CPPToken* CCPPPreprocessor::ConvertDecorators(CPPTokenList* pcTokens, size* piIndex, CSourceFile* pcFile, uint8 uiFlags, CArrayPreprocessedTokens* pacDest)
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
void CCPPPreprocessor::Init(CConfig* pcConfig)
{
	mcDefines.Init();
	mcDefineReplacementMemory.Init();
	mcDefineStrings.Init(256);

	mcSpecialOperators.Init();
	miIncludeDepth = 0;
	mbLogIncludes = false;
	mbDumpLogs = false;
	mpszIncludesLog = NULL;

	mcConditionalStack.Init();
	mpcCurrentFile = NULL;
	mpcSkipFile = NULL;
	miSkipStartLine = 0;
	mcHeadersStack.Init();  //Current file should be included in the stack.

	macSources.Init();
	macPushedBack.Init();
	maiDisabledDefines.Init();
	mpcOutput = NULL;
	muiPendingFlags = 0;
	mbInCondition = false;
	mbDefinedOperand = false;
	mbError = false;
	mbSkipFailedDirectives = false;
	mbFullPathErrors = false;
	mbSkippedDirective = false;
	mbKeepPragmaPack = pcConfig ? pcConfig->mbKeepPragmaPack : false;
	miCounter = 0;
	mcErrors.Init();
	mcWarnings.Init();
	mcHeaderNames.Init();
	mcPragmaOnce.Init();
	mpcHeaderCache = NULL;
	masRecorders.Init();
	muiIncludeDirectories = 0;
	miLookAhead = 0;
	mbReplaying = false;
	muiHeaderHits = 0;
	muiHeaderMisses = 0;
	muiReplayedTokens = 0;

	AddSpecialDefine("__DATE__");
	AddSpecialDefine("__TIME__");
	AddSpecialDefine("__FILE__");
	AddSpecialDefine("__LINE__");
	AddSpecialDefine("__TIMESTAMP__");
	AddSpecialDefine("__COUNTER__");

	//C++14.  __func__ and __FUNCTION__ are identifiers, not macros.  __STDC__ is left to the configuration.
	AddDefine("__cplusplus", "201402L", &mcDefineReplacementMemory);
	AddDefine("__STDC_HOSTED__", "1", &mcDefineReplacementMemory);

	//The features of the language's literals, which are all the tokeniser's and whose strings the generators make.
	//The others are defined as the generators come to have them.  [cpp.predefined], SD-6
	AddDefine("__cpp_binary_literals", "201304", &mcDefineReplacementMemory);
	AddDefine("__cpp_digit_separators", "201309", &mcDefineReplacementMemory);
	AddDefine("__cpp_hex_float", "201603", &mcDefineReplacementMemory);
	AddDefine("__cpp_raw_strings", "200710", &mcDefineReplacementMemory);
	AddDefine("__cpp_unicode_characters", "200704", &mcDefineReplacementMemory);
	AddDefine("__cpp_unicode_literals", "200710", &mcDefineReplacementMemory);
	if (!pcConfig || pcConfig->mbHostDefines)
	{
		//What the host's operator new aligns to, as the Microsoft compiler has it (a size_t).  The other targets
		//define their own (CCPPTarget::AddDefines).  [cpp.predefined]
		AddDefine("__STDCPP_DEFAULT_NEW_ALIGNMENT__", (sizeof(void*) == 8) ? "16ull" : "8u", &mcDefineReplacementMemory);
	}

	AddSpecialOperator("defined", PPSO_Defined);
	AddSpecialOperator("__has_attribute", PPSO_HasAttribute);
	AddSpecialOperator("__has_cpp_attribute", PPSO_HasCppAttribute);
	AddSpecialOperator("__has_builtin", PPSO_HasBuiltIn);
	AddSpecialOperator("__has_include", PPSO_HasInclude);
	AddSpecialOperator("__has_include_next", PPSO_HasIncludeNext);
	AddSpecialOperator("__has_feature", PPSO_HasFeature);
	AddSpecialOperator("__has_extension", PPSO_HasFeature);

	if (!pcConfig || pcConfig->mbHostDefines)
	{
		InitPlatformSpecific();
	}
	AddConfigDefines(pcConfig);
	mszVaArgs.Init("__VA_ARGS__");
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::AddIncludeDirectories(CArrayHeaderNameMap* pcHeaderNames)
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
void CCPPPreprocessor::AddIncludeDirectory(CHeaderFiles* pcHeaderMapName)
{
	mcHeaderNames.Add(pcHeaderMapName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CCPPPreprocessor::GetDefine(CExternalString* pcString)
{
	return mcDefines.Get(pcString);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CCPPPreprocessor::GetDefine(char* szName)
{
	return mcDefines.Get(szName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CCPPPreprocessor::AddSpecialDefine(char* szDefine)
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
//
// szDefine as the number lliValue (a compiler's numeric macro, such as
// _MSC_VER).
//
//////////////////////////////////////////////////////////////////////////
CDefine* CCPPPreprocessor::AddNumberDefine(char* szDefine, int64 lliValue)
{
	CChars		sz;
	CDefine*	pcDefine;

	sz.Init();
	sz.Append(lliValue);
	pcDefine = AddDefine(szDefine, sz.Text(), &mcDefineReplacementMemory);
	sz.Kill();
	return pcDefine;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CCPPPreprocessor::AddDefine(const char* szDefine)
{
	return mcDefines.AddDefine(szDefine);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CSpecialOperator* CCPPPreprocessor::AddSpecialOperator(char* szSpecialOperator, EPreprocessorSpecialOperator eType)
{
	return mcSpecialOperators.AddSpecialOperator(szSpecialOperator, eType);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CDefine* CCPPPreprocessor::AddDefine(const char* szDefine, const char* szReplacement, CPPTokenMemory* pcTokenMemory)
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
//
// A macro's definition as two definitions of it are compared: function
// like or not, its parameters, and its replacement list's tokens with any
// white space between two of them as one space.  [cpp.replace] 1
//
//////////////////////////////////////////////////////////////////////////
static bool IsPaste(CPPToken* pcToken)
{
	return pcToken->IsHash() && (((CPPHashes*)pcToken)->miCount == 2);
}


static void DefineSpelling(CDefine* pcDefine, CChars* psz)
{
	CPPTokenList*	pcTokens;
	CPPToken*		pcToken;
	size			i;
	bool			bSpace;

	psz->Append(pcDefine->IsBracketed() ? "(" : "");
	for (i = 0; i < pcDefine->GetArguments()->NumElements(); i++)
	{
		psz->Append(pcDefine->GetArguments()->GetText(i));
		psz->Append(',');
	}
	psz->Append(pcDefine->IsVariadic() ? "...)" : (pcDefine->IsBracketed() ? ")" : ""));
	pcTokens = pcDefine->GetReplacement()->GetTokenList();
	bSpace = false;
	for (i = 0; i < pcTokens->NumTokens(); i++)
	{
		pcToken = pcTokens->Get(i);
		if (pcToken->IsWhitespace())
		{
			bSpace = true;
			continue;
		}
		if (bSpace)
		{
			psz->Append(' ');
		}
		bSpace = false;
		pcToken->Print(psz);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The rules of a replacement list: # in a function-like macro is followed
// by a parameter, ## is neither first nor last, __VA_ARGS__ is only in a
// variadic macro's.  An error if one is broken.  [cpp.replace] 5,
// [cpp.stringize] 1, [cpp.concat] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::CheckReplacementList(CDefine* pcDefine, SPreprocessorPosition* psPos)
{
	CPPTokenList*	pcTokens;
	CPPToken*		pcToken;
	CPPToken*		pcNext;
	CPPText*		pcText;
	size			i;
	size			j;
	size			uiFirst;
	size			uiLast;

	pcTokens = pcDefine->GetReplacement()->GetTokenList();
	uiFirst = ARRAY_ELEMENT_NOT_FOUND;
	uiLast = ARRAY_ELEMENT_NOT_FOUND;
	for (i = 0; i < pcTokens->NumTokens(); i++)
	{
		pcToken = pcTokens->Get(i);
		if (pcToken->IsWhitespace())
		{
			continue;
		}
		if (uiFirst == ARRAY_ELEMENT_NOT_FOUND)
		{
			uiFirst = i;
		}
		uiLast = i;
		if (pcToken->IsText())
		{
			pcText = (CPPText*)pcToken;
			if ((pcText->meType == PPT_Identifier) && (pcText->mcText.miLen == mszVaArgs.Length()) && (memcmp(pcText->mcText.msz, mszVaArgs.Text(), mszVaArgs.Length()) == 0))
			{
				return Error(psPos, "__VA_ARGS__ is only in the replacement list of a macro with ...", NULL);
			}
		}
		if (pcToken->IsHash() && (((CPPHashes*)pcToken)->miCount == 1) && pcDefine->IsBracketed())
		{
			pcNext = NULL;
			for (j = i + 1; j < pcTokens->NumTokens(); j++)
			{
				pcNext = pcTokens->Get(j);
				if (!pcNext->IsWhitespace())
				{
					break;
				}
				pcNext = NULL;
			}
			if (!pcNext || !pcNext->IsReplacement())
			{
				return Error(psPos, "# in a function-like macro is followed by a parameter", NULL);
			}
		}
	}
	if ((uiFirst != ARRAY_ELEMENT_NOT_FOUND) && ((IsPaste(pcTokens->Get(uiFirst))) || (IsPaste(pcTokens->Get(uiLast)))))
	{
		return Error(psPos, "## is between two tokens: it isn't first or last in a replacement list", NULL);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ProcessHashDefine(CPPDirective* pcDirective, CPreprocessorTokenParser* pcParser)
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
	CPPTokenMemory*			pcTokenMemory;
	CChars					szExisting;
	CChars					szNew;
	bool					bRedefined;

	MarkPositionForError(pcParser, &sPos);

	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		if ((cIdentifier.miLen == 7) && (memcmp(cIdentifier.msz, "defined", 7) == 0))
		{
			//[cpp.replace] 8
			return Error(&sPos, "defined can't be a macro's name");
		}
		pcExisting = mcDefines.Get(&cIdentifier);
		szExisting.Init();
		bRedefined = pcExisting != NULL;
		if (pcExisting)
		{
			DefineSpelling(pcExisting, &szExisting);
			mcDefines.Remove(&cIdentifier);
		}
		pcDefine = mcDefines.AddDefine(&cIdentifier);
		if (masRecorders.NumElements() > 0)
		{
			NoteDefined(cIdentifier.msz, cIdentifier.miLen);
			NoteEvent(PPHE_Define, mpcCurrentFile, pcDirective);
		}

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
		pcDefine->SetPosition(pcDirective->Line(), pcDirective->Column());

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
						pcReplacement = pcTokenMemory->AddToken<CPPReplacement>();
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
					if (pcToken == NULL)
					{
						return Error(&sPos, "Cannot duplicate token");
					}
				}
				pcDefine->AddReplacementToken(pcToken);
			}

			pcParser->NextToken();
		}

		bResult = CheckReplacementList(pcDefine, &sPos);
		if (bResult && bRedefined)
		{
			//A macro defined again is defined the same.  [cpp.replace] 2
			szNew.Init();
			DefineSpelling(pcDefine, &szNew);
			if (!szNew.Equals(&szExisting))
			{
				bResult = Error(&sPos, "The macro is defined again differently: a macro is only defined again as it was (#undef it first)");
			}
			szNew.Kill();
		}
		szExisting.Kill();
		return bResult;
	}
	else
	{
		return Error(&sPos, "Expected identifier");
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ProcessHashDefineBracketted(CPreprocessorTokenParser* pcParser, CDefine* pcDefine)
{
	CExternalString			cArgument;
	bool					bVariadicFound;
	SPreprocessorPosition	sPos;

	bVariadicFound = false;
	pcDefine->SetBracketed(true);
	MarkPositionForError(pcParser, &sPos);
	if (pcParser->GetExactDecorator(')'))
	{
		return true;
	}
	for (;;)
	{
		MarkNextPositionForError(pcParser, &sPos);
		if (pcParser->GetExactDecorator("..."))
		{
			cArgument.Init(mszVaArgs.Text(), mszVaArgs.Length());
			pcDefine->AddArgument(&cArgument);
			pcDefine->SetVariadic();
			bVariadicFound = true;
		}
		else if (pcParser->GetIdentifier(&cArgument))
		{
			if (pcDefine->GetArguments()->GetIndex(cArgument.msz, cArgument.miLen) != ARRAY_ELEMENT_NOT_FOUND)
			{
				//[cpp.replace] 6
				return Error(&sPos, "Two of the macro's parameters have the same name.");
			}
			pcDefine->AddArgument(&cArgument);
			if (pcParser->GetExactDecorator("..."))
			{
				pcDefine->SetVariadic();
				bVariadicFound = true;
			}
		}
		else
		{
			return Error(&sPos, "Expected identifier.");
		}

		//Parameters are separated by commas, and "..." is the last.  [cpp.replace] 1
		MarkNextPositionForError(pcParser, &sPos);
		if (pcParser->GetExactDecorator(')'))
		{
			return true;
		}
		if (bVariadicFound)
		{
			return Error(&sPos, "Expected ')' after \"...\".");
		}
		if (!pcParser->GetExactDecorator(','))
		{
			return Error(&sPos, "Expected ',' or ')'.");
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// #line N or #line N "name": the next line of the current file is line N
// for __LINE__, and __FILE__ is name.  N is a digit sequence, 1 to
// 2147483647.  A header with one is preprocessed each time it is included
// (its lines depend on it).  [cpp.line]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ProcessHashLine(CPPDirective* pcDirective, CPreprocessorTokenParser* pcParser)
{
	SPreprocessorPosition		sPos;
	CArrayPreprocessedTokens	acRaw;
	CArrayPreprocessedTokens	acExpanded;
	CArrayPreprocessedTokens*	pacTokens;
	SPreprocessedToken*			psNumber;
	SPreprocessedToken*			psName;
	CChars						szName;
	SPPSource*					psSource;
	int64						lliLine;
	size						i;
	bool						bResult;
	bool						bNameWrong;

	//#line's tokens are macro replaced unless they are already a number.  [cpp.line] 6
	MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
	acRaw.Init();
	acExpanded.Init();
	ConvertTokens(pcDirective->GetTokenList(), mpcCurrentFile, NULL, false, &acRaw);
	pacTokens = &acRaw;
	psNumber = acRaw.SafeGet(0);
	if (psNumber && (psNumber->uiKind != PTK_Number))
	{
		ExpandTokens(&acRaw, &acExpanded);
		pacTokens = &acExpanded;
	}

	psNumber = pacTokens->SafeGet(0);
	psName = pacTokens->SafeGet(1);
	lliLine = 0;
	bResult = psNumber && (psNumber->uiKind == PTK_Number) && (psNumber->uiLength <= 10);
	for (i = 0; bResult && (i < psNumber->uiLength); i++)
	{
		bResult = (psNumber->szText[i] >= '0') && (psNumber->szText[i] <= '9');
		lliLine = lliLine * 10 + (psNumber->szText[i] - '0');
	}
	szName.Init();
	if (bResult && psName)
	{
		psName->Append(&szName);
	}
	bNameWrong = psName && ((pacTokens->NumElements() > 2) || (psName->uiKind != PTK_String));
	acRaw.Kill();
	acExpanded.Kill();
	if (!bResult)
	{
		szName.Kill();
		return Error(&sPos, "Expected a line number after #line.");
	}
	if ((lliLine < 1) || (lliLine > 2147483647))
	{
		szName.Kill();
		return Error(&sPos, "The #line number must be from 1 to 2147483647.");
	}
	if (bNameWrong || (!szName.Empty() && ((szName.Length() < 2) || (szName.GetChar(0) != '"') || (szName.GetChar(szName.Length() - 1) != '"'))))
	{
		szName.Kill();
		return Error(&sPos, "Expected a file name in quotes after the #line number.");
	}

	psSource = GetCurrentFileSource();
	if (psSource)
	{
		InvalidateRecordings();
		psSource->iLineOffset = (int)(lliLine - (pcDirective->Line() + 2));
		if (!szName.Empty())
		{
			psSource->szLineFile.Set(szName.Text());
		}
		mpcOutput->AddLineDirective(mpcCurrentFile, (int)pcDirective->Line() + 1, psSource->iLineOffset, psSource->szLineFile.Text());
	}
	szName.Kill();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ProcessHashUndef(CPPDirective* pcDirective, CPreprocessorTokenParser* pcParser)
{
	bool				bResult;
	CExternalString		cIdentifier;

	pcParser->SkipWhitespace();
	bResult = pcParser->GetIdentifier(&cIdentifier);
	if (bResult)
	{
		mcDefines.Remove(&cIdentifier);
		if (masRecorders.NumElements() > 0)
		{
			NoteDefined(cIdentifier.msz, cIdentifier.miLen);
			NoteEvent(PPHE_Undef, mpcCurrentFile, pcDirective);
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
static bool IsPathPrefix(char* szPath, const char* szBase, size iBaseLength)
{
	size	i;
	char	c1;
	char	c2;

	//Either separator matches; case is ignored only where file names ignore it (CPP_HEADER_NAMES_CASE_SENSITIVE).
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
		if (!CPP_HEADER_NAMES_CASE_SENSITIVE)
		{
			c1 = ToLower(c1);
			c2 = ToLower(c2);
		}
		if (c1 != c2)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A relative include path ("/" separated) with its "." parts and its
// "dir/.." pairs taken out: "Sub/../Engine.h" is "Engine.h".  False if it
// goes above where it starts ("../Engine.h"), when it is left as it was.
//
//////////////////////////////////////////////////////////////////////////
bool CollapseIncludePath(CChars* pszPath)
{
	CArrayChars	aszParts;
	CArrayChars	aszKept;
	CChars*		pszPart;
	size		i;
	bool		bInside;

	aszParts.Init();
	aszParts.Split(pszPath->Text(), '/');
	aszKept.Init();
	bInside = true;
	for (i = 0; (i < aszParts.NumElements()) && bInside; i++)
	{
		pszPart = aszParts.Get(i);
		if (pszPart->Empty() || pszPart->Equals("."))
		{
			continue;
		}
		if (pszPart->Equals(".."))
		{
			if (aszKept.NumElements() == 0)
			{
				bInside = false;
			}
			else
			{
				aszKept.RemoveTail();
			}
			continue;
		}
		aszKept.Add(pszPart->Text());
	}
	if (bInside)
	{
		pszPath->Clear();
		for (i = 0; i < aszKept.NumElements(); i++)
		{
			if (i > 0)
			{
				pszPath->Append('/');
			}
			pszPath->Append(aszKept.GetText(i));
		}
	}
	aszKept.Kill();
	aszParts.Kill();
	return bInside;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::FindBestInclude(CExternalString* pcInclude, bool bSystemFile, CHeaderFile** ppcCFile, CHeaderFiles** ppcHeaderNameMap, bool bNext)
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
	const char*					szBase;
	bool						bCovered;
	bool						bLeaves;
	CFileUtil					cFileUtil;

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
			bCovered = false;
			bLeaves = false;
			for (j = 0; j < mcHeaderNames.NumElements(); j++)
			{
				pcHeaderNameMap = *mcHeaderNames.Get(j);
				szBase = pcHeaderNameMap->GetBaseDirectory();
				iBaseLength = pcHeaderNameMap->GetBaseDirectoryLength();
				if ((szPath.Length() >= iBaseLength) && IsPathPrefix(szPath.Text(), szBase, iBaseLength) && ((szPath.Length() == iBaseLength) || (szPath.GetChar(iBaseLength) == '/') || (szPath.GetChar(iBaseLength) == '\\')))
				{
					bCovered = true;
					szRelative.Init();
					if (szPath.Length() > iBaseLength)
					{
						szRelative.Append(szPath.Text(iBaseLength + 1));
						szRelative.Append('/');
					}
					szRelative.Append(szInclude);

					//"../Engine.h" from Sub/User.cpp is Engine.h in the same include directory.
					if (CollapseIncludePath(&szRelative))
					{
						pcHeaderFile = pcHeaderNameMap->GetFile(szRelative.Text());
					}
					else
					{
						bLeaves = true;
						pcHeaderFile = NULL;
					}
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

			//Going up out of every include directory ("../Engine.h" from Tool/Tool.cpp), or from a file in none of them,
			//it is looked for on the disk from the file's directory, as MSVC looks for it.
			if ((bLeaves || !bCovered) && (mcHeaderNames.NumElements() > 0))
			{
				szRelative.Init(szPath);
				szRelative.Append('/');
				szRelative.Append(szInclude);
				szRelative.Replace('/', FILE_SEPARATOR[0]);
				cFileUtil.CollapsePath(&szRelative);
				pcHeaderFile = (*mcHeaderNames.Get(0))->FindOutside(szRelative.Text());
				szRelative.Kill();
				if (pcHeaderFile)
				{
					*ppcCFile = pcHeaderFile;
					*ppcHeaderNameMap = *mcHeaderNames.Get(0);
					szPath.Kill();
					szInclude.Kill();
					return;
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

	//"./Engine.h" and "Sub/../Engine.h" are Engine.h in an include directory too.
	CollapseIncludePath(&szInclude);
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
void CCPPPreprocessor::LogInclude(CSourceFile* pcFile)
{
	CChars			sz;

	if (mbLogIncludes)
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
char* CCPPPreprocessor::GetFileName(void) const
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
//
// The current file as its errors name it: its full path with
// SetFullPathErrors, otherwise its name.
//
//////////////////////////////////////////////////////////////////////////
char* CCPPPreprocessor::GetErrorFileName(void) const
{
	if (mpcCurrentFile)
	{
		return ErrorFileName(mpcCurrentFile);
	}
	return GetFileName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CCPPPreprocessor::ErrorFileName(CSourceFile* pcFile) const
{
	return mbFullPathErrors ? pcFile->FullName() : pcFile->ShortName();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::LogIncludes(CChars* pszIncludesLog)
{
	if (pszIncludesLog)
	{
		mbLogIncludes = true;
		mpszIncludesLog = pszIncludesLog;
	}
	else
	{
		mbLogIncludes = false;
		mpszIncludesLog = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::LogDumping(bool bDumpLogs)
{
	mbDumpLogs = bDumpLogs;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ProcessHashError(CPPDirective* pcDirective, CPreprocessorTokenParser* pcParser, bool bWarning)
{
	CChars					szMessage;
	CPPToken*				pcToken;
	SPreprocessorPosition	sPos;

	//#error's text is an error (preprocessing fails); #warning's is a warning.
	MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
	pcParser->SkipWhitespace();
	szMessage.Init(bWarning ? "#warning " : "#error ");
	while (pcParser->HasToken())
	{
		pcToken = pcParser->GetToken();
		pcToken->Print(&szMessage);
		pcParser->NextToken();
	}
	szMessage.StripWhitespace(true);
	if (bWarning)
	{
		AddPositioned(&mcWarnings, &sPos, szMessage.Text(), NULL);
		szMessage.Kill();
		return true;
	}
	Error(&sPos, szMessage.Text());
	szMessage.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ProcessHashPragma(CPreprocessorTokenParser* pcParser)
{
	bool			bResult;

	pcParser->SkipWhitespace();
	if (mbKeepPragmaPack)
	{
		pcParser->PushPosition();
		if (pcParser->GetExactIdentifier("pack", true, true))
		{
			pcParser->PassPosition();
			EmitPragmaPack(pcParser);
			return true;
		}
		pcParser->PopPosition();
	}
	bResult = pcParser->GetExactIdentifier("once", true, true);
	if (bResult)
	{
		if (mpcCurrentFile && mpcCurrentFile->IsHeader())
		{
			mcPragmaOnce.Put(mpcCurrentFile, mpcCurrentFile);
			if (masRecorders.NumElements() > 0)
			{
				NoteEvent(PPHE_PragmaOnce, mpcCurrentFile, NULL);
			}
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
static bool IsIdentifierStart(char c)
{
	return IsCPPNameStart(c) || (c == '$');
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
//
// The length of the pp-number starting at sz (a digit, or . and a digit),
// at most uiLength: digits, letters, _, ., ' and a sign after e, E, p or
// P.  [lex.ppnumber]
//
//////////////////////////////////////////////////////////////////////////
static size PPNumberLength(const char* sz, size uiLength)
{
	size	i;
	char	c;

	for (i = 1; i < uiLength; i++)
	{
		c = sz[i];
		if (!IsIdentifierCharacter(c) && (c != '.') && (c != '\'') &&
			!(((c == '+') || (c == '-')) && ((sz[i - 1] == 'e') || (sz[i - 1] == 'E') || (sz[i - 1] == 'p') || (sz[i - 1] == 'P'))))
		{
			break;
		}
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//																		//
// #pragma pack(...) is output as __pragma(pack(...)), which the parser	//
// takes as the packing of the classes defined after it.					//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::EmitPragmaPack(CPreprocessorTokenParser* pcParser)
{
	CChars					szArguments;
	SPreprocessedToken		sToken;
	char*					sz;
	size					i;
	size					uiStart;
	EPreprocessedTokenKind	eKind;
	int						iLine;
	int						iColumn;
	bool					bSpace;

	//Each token is at where the arguments start (the pragma's line), with the spaces between them kept; __pragma starts
	//the line, as the directive did.
	pcParser->SkipWhitespace();
	iLine = pcParser->Line();
	iColumn = pcParser->Column();
	szArguments.Init("__pragma(pack");
	pcParser->AppendRemaining(&szArguments);
	szArguments.Append(")");
	sz = szArguments.Text();
	bSpace = false;
	for (i = 0; sz[i];)
	{
		if ((sz[i] == ' ') || (sz[i] == '\t') || (sz[i] == '\n') || (sz[i] == '\r'))
		{
			bSpace = true;
			i++;
			continue;
		}
		uiStart = i;
		if (IsIdentifierStart(sz[i]))
		{
			eKind = PTK_Identifier;
			while (IsIdentifierCharacter(sz[i]))
			{
				i++;
			}
		}
		else if ((sz[i] >= '0') && (sz[i] <= '9'))
		{
			eKind = PTK_Number;
			i += PPNumberLength(&sz[i], StrLen(&sz[i]));
		}
		else
		{
			eKind = PTK_Punctuator;
			i++;
		}
		sToken.Init(eKind, mpcOutput->AddString(&sz[uiStart], i - uiStart), i - uiStart, mpcCurrentFile, iLine, iColumn);
		sToken.uiFlags |= PTF_OUTPUT_TEXT | (bSpace ? PTF_LEADING_SPACE : 0) | ((uiStart == 0) ? PTF_START_OF_LINE : 0);
		bSpace = false;
		EmitToken(&sToken);
	}
	szArguments.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ProcessHashUnknown(CPPDirective* pcDirective)
{
	CChars					szError;
	SPreprocessorPosition	sPos;

	//Unknown directives are only an error when they are not skipped by a conditional.
	MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
	szError.Init("Unknown directive #");
	pcDirective->GetTokenList()->Print(&szError);
	szError.StripWhitespace(true);
	Error(&sPos, szError.Text());
	szError.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE CCPPPreprocessor::EvaluateEquation(char* szText, CChars* szCalculatorError)
{
	return CPreprocessorExpression::Evaluate(szText, szCalculatorError);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::AddConfigDefines(CConfig* pcConfig)
{
	size		i;
	CChars*		pszText;
	size		iEquals;
	CChars		szName;

	if (pcConfig)
	{
		for (i = 0; i < pcConfig->maszDefines.NumElements(); i++)
		{
			//A define can have a value, as in "NAME=VALUE".  Without one it is 1, as -D NAME and /D NAME are.
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
			mcDefines.Remove(szName.Text());
			if (iEquals == ARRAY_ELEMENT_NOT_FOUND)
			{
				AddDefine(szName.Text(), "1", &mcDefineReplacementMemory);
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
void CCPPPreprocessor::MarkPositionForError(CPreprocessorTokenParser* pcParser, SPreprocessorPosition* psPos)
{
	char* szShortFileName;

	szShortFileName = GetErrorFileName();
	psPos->Init(pcParser, szShortFileName);
}


//////////////////////////////////////////////////////////////////////////
//
// Moves psPos to the parser's next token, or leaves it where it was at the
// end of the line (an error there is reported at the last token seen).
//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::MarkNextPositionForError(CPreprocessorTokenParser* pcParser, SPreprocessorPosition* psPos)
{
	pcParser->SkipWhitespace();
	if (pcParser->HasToken())
	{
		MarkPositionForError(pcParser, psPos);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::MarkPositionForError(size iLine, size iColumn, SPreprocessorPosition* psPos)
{
	char* szShortFileName;

	szShortFileName = GetErrorFileName();
	psPos->Init(iLine, iColumn, szShortFileName);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::TranslationUnitLogging(CTranslationUnit* pcFile)
{
	if (pcFile->LogIncludes())
	{
		LogIncludes(&pcFile->GetLogs()->szIncludesLog);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CPPToken* CCPPPreprocessor::DuplicatePPToken(CPPToken* pcSource)
{
	CPPTokenMemory*		pcTokenMemory;

	pcTokenMemory = GetTokenMemory();
	return ::DuplicatePPToken(pcSource, pcTokenMemory);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::Next(SPreprocessedToken* psToken)
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
				miLookAhead++;
				bResult = SkipPragmaOperator();
				miLookAhead--;
				if (bResult)
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
		if (pcDefine->IsBracketed())
		{
			//A function-like macro name not followed by ( is not an invocation.  Reading what follows it (and its
			//arguments) may read past the end of a header (miLookAhead).
			miLookAhead++;
			if (!PeekOpenParenthesis())
			{
				miLookAhead--;
				acArguments.Kill();
				return true;
			}

			bResult = CollectArguments(pcDefine, psToken, &acArguments);
			miLookAhead--;
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
bool CCPPPreprocessor::PeekOpenParenthesis(void)
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
bool CCPPPreprocessor::CollectArguments(CDefine* pcDefine, SPreprocessedToken* psName, CArrayPreprocessedTokenArrays* pacArguments)
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
		//A call has an argument for ..., if only an empty one after a comma (C++20 lets it be left out).
		//[cpp.replace] 4
		szError.Init("Macro ");
		szError.Append(pcDefine->GetName());
		szError.Append(" needs an argument for its ...: put a comma after the last argument.");
		Error(psName, szError.Text());
		szError.Kill();
		return false;
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
bool CCPPPreprocessor::ExpandMacro(CDefine* pcDefine, SPreprocessedToken* psName, CArrayPreprocessedTokenArrays* pacArguments)
{
	CArrayPreprocessedTokens	acResult;
	CArrayPreprocessedTokens	acFinal;
	SPreprocessedToken*			psToken;
	int							iExpansion;

	iExpansion = mpcOutput->AddExpansion(psName->szText, psName->uiLength, psName->pcFile, psName->iLine, psName->iColumn, psName->iExpansion);

	acResult.Init();
	if (!SubstituteParameters(pcDefine, pacArguments, iExpansion, &acResult))
	{
		acResult.Kill();
		return false;
	}
	acFinal.Init();
	PasteTokens(&acResult, &acFinal);
	acResult.Kill();

	if (mbInCondition && (acFinal.NumElements() == 0) && !pcDefine->IsBracketed())
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
// An argument's tokens appended, the first spaced as uiLeadingSpace	//
// says.																//
//																		//
//////////////////////////////////////////////////////////////////////////
static void AppendArgumentTokens(CArrayPreprocessedTokens* pacArgument, uint32 uiLeadingSpace, CArrayPreprocessedTokens* pacResult)
{
	SPreprocessedToken	sToken;
	size				i;

	for (i = 0; i < pacArgument->NumElements(); i++)
	{
		sToken = *pacArgument->Get(i);
		if (i == 0)
		{
			sToken.uiFlags = (sToken.uiFlags & ~(PTF_LEADING_SPACE | PTF_START_OF_LINE)) | uiLeadingSpace;
		}
		pacResult->Add(&sToken);
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
// The macro's body with its parameters replaced, in pacResult.  False	//
// if expanding an argument failed.  [cpp.subst]						//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::SubstituteParameters(CDefine* pcDefine, CArrayPreprocessedTokenArrays* pacArguments, int iExpansion, CArrayPreprocessedTokens* pacResult)
{
	CArrayPreprocessedTokens*		pacBody;
	CArrayPreprocessedTokenArrays	acExpanded;
	CArrayPreprocessedTokens*		pacArgument;
	SPreprocessedToken*				psBody;
	SPreprocessedToken				sToken;
	size							i;
	size							iNumBody;
	size							iNumParameters;
	int								iParameter;
	bool							bFunctionLike;
	bool							bRaw;

	pacBody = GetBody(pcDefine);
	iNumBody = pacBody->NumElements();
	iNumParameters = pacArguments->NumElements();
	bFunctionLike = pcDefine->IsBracketed();

	//Arguments are macro replaced once, when first needed.
	acExpanded.Init();
	for (i = 0; i < iNumParameters; i++)
	{
		acExpanded.Add()->Init();
	}

	for (i = 0; i < iNumBody; i++)
	{
		psBody = pacBody->Get(i);
		if (bFunctionLike && (psBody->uiFlags & PTF_STRINGIFY) && (i + 1 < iNumBody) && (pacBody->Get(i + 1)->iParameter >= 0))
		{
			//#parameter.  [cpp.stringize]
			Stringify(pacArguments->Get(pacBody->Get(i + 1)->iParameter), &sToken);
			sToken.uiFlags = psBody->uiFlags & PTF_LEADING_SPACE;
			sToken.iExpansion = iExpansion;
			pacResult->Add(&sToken);
			i++;
			continue;
		}

		if ((psBody->uiFlags & PTF_PASTE) && pcDefine->IsVariadic() && (i > 0) && (i + 1 < iNumBody) && pacBody->Get(i - 1)->IsPunctuator(",") && (pacBody->Get(i - 1)->iParameter < 0) && (pacBody->Get(i + 1)->iParameter == (int)iNumParameters - 1))
		{
			//GNU and Microsoft: in ", ## __VA_ARGS__" the comma is removed when there are no variadic arguments.
			pacArgument = pacArguments->Get(iNumParameters - 1);
			if (pacArgument->NumElements() == 0)
			{
				pacResult->Pop();
			}
			else
			{
				AppendArgumentTokens(pacArgument, pacBody->Get(i + 1)->uiFlags & PTF_LEADING_SPACE, pacResult);
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
				pacResult->Add(&sToken);
			}
			else
			{
				AppendArgumentTokens(pacArgument, psBody->uiFlags & PTF_LEADING_SPACE, pacResult);
			}
			continue;
		}

		sToken = *psBody;
		sToken.iExpansion = iExpansion;
		sToken.iParameter = -1;
		pacResult->Add(&sToken);
	}

	for (i = 0; i < acExpanded.NumElements(); i++)
	{
		acExpanded.Get(i)->Kill();
	}
	acExpanded.Kill();
	return !mbError;
}


//////////////////////////////////////////////////////////////////////////
//																		//
// The tokens with each ## done and the placemarkers gone, in pacFinal.	//
// ## is only an operator if it came from the body.  [cpp.concat]		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::PasteTokens(CArrayPreprocessedTokens* pacTokens, CArrayPreprocessedTokens* pacFinal)
{
	CArrayPreprocessedTokens	acPasted;
	SPreprocessedToken*			psToken;
	SPreprocessedToken*			psLeft;
	SPreprocessedToken			sToken;
	size						i;
	CChars						szError;

	acPasted.Init();
	for (i = 0; i < pacTokens->NumElements(); i++)
	{
		psToken = pacTokens->Get(i);
		if (psToken->uiFlags & PTF_PASTE)
		{
			if ((acPasted.NumElements() > 0) && (i + 1 < pacTokens->NumElements()))
			{
				psLeft = acPasted.Tail();
				if (Paste(psLeft, pacTokens->Get(i + 1), &sToken))
				{
					*psLeft = sToken;
				}
				else
				{
					//What ## makes is one preprocessing token.  [cpp.concat] 3
					szError.Init("Pasting \"");
					psLeft->Append(&szError);
					szError.Append("\" and \"");
					pacTokens->Get(i + 1)->Append(&szError);
					szError.Append("\" doesn't make one token.");
					Error(psLeft, szError.Text());
					szError.Kill();
					acPasted.Add(pacTokens->Get(i + 1));
				}
				i++;
			}
			continue;
		}
		acPasted.Add(psToken);
	}

	for (i = 0; i < acPasted.NumElements(); i++)
	{
		psToken = acPasted.Get(i);
		if (psToken->uiKind != PTK_Placemarker)
		{
			sToken = *psToken;
			sToken.uiFlags &= ~(PTF_PASTE | PTF_STRINGIFY | PTF_START_OF_LINE);
			pacFinal->Add(&sToken);
		}
	}
	acPasted.Kill();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::ExpandArgument(CArrayPreprocessedTokens* pacRaw, CArrayPreprocessedTokens* pacExpanded)
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
void CCPPPreprocessor::ExpandTokens(CArrayPreprocessedTokens* pacSource, CArrayPreprocessedTokens* pacDest)
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
void CCPPPreprocessor::Stringify(CArrayPreprocessedTokens* pacArgument, SPreprocessedToken* psResult)
{
	CChars				sz;
	size				i;
	size				j;
	SPreprocessedToken*	psToken;
	char				c;

	//Whitespace between tokens becomes one space.  " and \ in string and character literals are escaped, and a new line
	//in a raw string is \n, so that the result is a string.  [cpp.stringize]
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
				if (c == '\n')
				{
					//A raw string's new line.
					sz.Append("\\n");
				}
				else if (c != '\r')
				{
					if ((c == '"') || (c == '\\'))
					{
						sz.Append('\\');
					}
					sz.Append(c);
				}
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
//
// The length of the identifier at sz, uiLength at most; 0 if there isn't
// one.
//
//////////////////////////////////////////////////////////////////////////
static size IdentifierLength(const char* sz, size uiLength)
{
	size	i;

	if ((uiLength == 0) || !IsIdentifierStart(sz[0]))
	{
		return 0;
	}
	for (i = 1; (i < uiLength) && IsIdentifierCharacter(sz[i]); i++)
	{
	}
	return i;
}


//////////////////////////////////////////////////////////////////////////
//
// The kind of the one preprocessing token sz spells, -1 if it is not
// exactly one token: what pasting two tokens must make.  [cpp.concat]
//
//////////////////////////////////////////////////////////////////////////
static int LexSingleToken(const char* sz, size uiLength)
{
	size	uiPrefix;
	size	uiQuoted;
	char	c;

	if (uiLength == 0)
	{
		return -1;
	}

	uiPrefix = IdentifierLength(sz, uiLength);
	if (uiPrefix == uiLength)
	{
		return PTK_Identifier;
	}

	c = sz[uiPrefix];
	if ((c == '"') || (c == '\''))
	{
		//A string or a character, after its prefix if it has one and before its user-defined suffix if it has one.
		if ((uiPrefix != 0) && !IsLiteralPrefix(sz, uiPrefix, c == '"'))
		{
			return -1;
		}
		uiQuoted = QuotedLiteralLength(sz, uiLength);
		if ((uiQuoted == 0) || ((uiQuoted != uiLength) && (uiQuoted + IdentifierLength(&sz[uiQuoted], uiLength - uiQuoted) != uiLength)))
		{
			return -1;
		}
		return (c == '"') ? PTK_String : PTK_Character;
	}
	if (uiPrefix != 0)
	{
		return -1;
	}

	if (((c >= '0') && (c <= '9')) || ((c == '.') && (uiLength > 1) && (sz[1] >= '0') && (sz[1] <= '9')))
	{
		return (PPNumberLength(sz, uiLength) == uiLength) ? PTK_Number : -1;
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
bool CCPPPreprocessor::Paste(SPreprocessedToken* psLeft, SPreprocessedToken* psRight, SPreprocessedToken* psResult)
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
void CCPPPreprocessor::ExpandSpecial(CDefine* pcDefine, SPreprocessedToken* psToken)
{
	SPPSource*		psSource;
	CChars			sz;
	const char*		szName;
	char*			szFileName;
	time_t			tNow;
	struct tm		sTime;
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
		psSource = GetCurrentFileSource();
		if (iLine < 0)
		{
			iLine = psSource ? psSource->iCurrentLine : 0;
		}
		sz.Append(iLine + 1 + (psSource ? psSource->iLineOffset : 0));
	}
	else if (strcmp(szName, "__COUNTER__") == 0)
	{
		//A header that counts, or tells the time, is preprocessed every time.
		InvalidateRecordings();
		sz.Append(miCounter);
		miCounter++;
	}
	else if ((strcmp(szName, "__FILE__") == 0) && GetCurrentFileSource() && !GetCurrentFileSource()->szLineFile.Empty())
	{
		//The name given by #line, as it was written.
		eKind = PTK_String;
		sz.Append(GetCurrentFileSource()->szLineFile);
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
		InvalidateRecordings();
		eKind = PTK_String;
		tNow = time(NULL);
		//The thread's own copy: units are preprocessed at the same time (CCPP65Builder::SetJobs), and localtime's is shared.
#ifdef _WIN32
		localtime_s(&sTime, &tNow);
#else
		localtime_r(&tNow, &sTime);
#endif
		psTime = &sTime;
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
bool CCPPPreprocessor::SkipPragmaOperator(void)
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
CArrayPreprocessedTokens* CCPPPreprocessor::GetBody(CDefine* pcDefine)
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
CDefine* CCPPPreprocessor::GetDefine(SPreprocessedToken* psToken)
{
	return LookUpDefine(psToken->szText, psToken->uiLength);
}


//////////////////////////////////////////////////////////////////////////
//
// The macro named szName (uiLength characters), NULL if there isn't one.
// A header being kept depends on what it is (NoteLookUp).
//
//////////////////////////////////////////////////////////////////////////
CDefine* CCPPPreprocessor::LookUpDefine(const char* szName, size uiLength)
{
	CExternalString		cName;
	CDefine*			pcDefine;

	cName.Init((char*)szName, (int)uiLength);
	pcDefine = mcDefines.Get(&cName);
	if (masRecorders.NumElements() > 0)
	{
		NoteLookUp(szName, uiLength, pcDefine);
	}
	return pcDefine;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::IsDisabled(int64 lliDefineID)
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
bool CCPPPreprocessor::IsDefined(SPreprocessedToken* psToken)
{
	CExternalString		cName;

	//The special operators, like __has_include, count as defined so that headers can test for them.
	if (LookUpDefine(psToken->szText, psToken->uiLength))
	{
		return true;
	}
	cName.Init((char*)psToken->szText, (int)psToken->uiLength);
	return mcSpecialOperators.Get(&cName) != NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::AddNumber(int64 lliValue, SPreprocessedToken* psPosition, CArrayPreprocessedTokens* pacDest)
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
bool CCPPPreprocessor::Error(SPreprocessedToken* psToken, char* szMessage)
{
	CChars	szDetail;

	//Preprocessor errors have no token index: the token may not be in the output.
	szDetail.Init();
	if (psToken)
	{
		psToken->Append(&szDetail);
	}
	if (psToken && psToken->pcFile && mpcOutput)
	{
		mpcOutput->AddError(&mcErrors, psToken, -1, szMessage, szDetail.Text(), mbFullPathErrors);
	}
	else if (psToken && psToken->pcFile)
	{
		mcErrors.Add(ErrorFileName(psToken->pcFile), psToken->iLine, psToken->iColumn, -1, szMessage, szDetail.Text());
	}
	else
	{
		mcErrors.Add(GetErrorFileName(), -1, -1, -1, szMessage, psToken ? szDetail.Text() : NULL);
	}
	szDetail.Kill();

	mbError = true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::Error(SPreprocessorPosition* psPosition, char* szMessage, char* szDetail)
{
	AddPositioned(&mcErrors, psPosition, szMessage, szDetail);
	mbError = true;
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// An error or warning in the current file at psPosition, where #line says
// that is.
//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::AddPositioned(CCPPErrors* pcErrors, SPreprocessorPosition* psPosition, char* szMessage, char* szDetail)
{
	const char*	szFile;
	int			iLine;

	szFile = psPosition->mszShortName;
	iLine = (int)psPosition->miLine;
	if (mpcOutput && mpcCurrentFile)
	{
		mpcOutput->ReportedPosition(mpcCurrentFile, &szFile, &iLine);
	}
	pcErrors->Add(szFile, iLine, (int)psPosition->miColumn, -1, szMessage, szDetail);
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPPreprocessor::GetErrors(void)
{
	//The errors in all the translation units preprocessed so far.
	return &mcErrors;
}


//////////////////////////////////////////////////////////////////////////
//
// The #warning messages in all the translation units preprocessed so far.
//
//////////////////////////////////////////////////////////////////////////
CCPPErrors* CCPPPreprocessor::GetWarnings(void)
{
	return &mcWarnings;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::EmitToken(SPreprocessedToken* psToken)
{
	const char*	szContents;
	size		iLength;

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
char* CCPPPreprocessor::AddString(const char* szText, size uiLength)
{
	char*	sz;

	//Text that lasts as long as the preprocessor.
	sz = (char*)mcDefineStrings.Add(uiLength + 1);
	memcpy(sz, szText, uiLength);
	sz[uiLength] = '\0';
	return sz;
}


//////////////////////////////////////////////////////////////////////////
//
// Where #define's tokens go: the preprocessor's, not the file's (a header
// is shared by the units preprocessed at the same time, and its defines
// are this unit's).
//
//////////////////////////////////////////////////////////////////////////
CPPTokenMemory* CCPPPreprocessor::GetTokenMemory(void)
{
	return &mcDefineReplacementMemory;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ProcessDirective(CPPDirective* pcDirective)
{
	CPreprocessorTokenParser	cParser;
	bool						bResult;
	bool						bParsing;

	if (pcDirective->IsConditional())
	{
		//A conditional is obeyed if the code before it or after it is compiled: one nested in a skipped group is skipped
		//with it.
		bParsing = mcConditionalStack.IsParsing();
		bResult = ProcessConditional(pcDirective);
		mpcOutput->AddDirectiveLine(mpcCurrentFile, pcDirective->Line(), bParsing || mcConditionalStack.IsParsing());
		NoteSkipping(pcDirective, bParsing);
		return bResult;
	}

	if (!mcConditionalStack.IsParsing())
	{
		mpcOutput->AddDirectiveLine(mpcCurrentFile, pcDirective->Line(), false);
		AddInactiveLine(mpcCurrentFile, pcDirective->Line());
		return true;
	}
	mpcOutput->AddDirectiveLine(mpcCurrentFile, pcDirective->Line(), true);

	bResult = true;
	cParser.Init(pcDirective);
	if (pcDirective->Is(PPD_define))
	{
		bResult = ProcessHashDefine(pcDirective, &cParser);
	}
	else if (pcDirective->Is(PPD_undef))
	{
		bResult = ProcessHashUndef(pcDirective, &cParser);
	}
	else if (pcDirective->Is(PPD_include))
	{
		bResult = ProcessHashInclude(pcDirective, false);
	}
	else if (pcDirective->Is(PPD_include_next))
	{
		bResult = ProcessHashInclude(pcDirective, true);
	}
	else if (pcDirective->Is(PPD_error) || pcDirective->Is(PPD_warning))
	{
		bResult = ProcessHashError(pcDirective, &cParser, pcDirective->Is(PPD_warning));
	}
	else if (pcDirective->Is(PPD_pragma))
	{
		bResult = ProcessHashPragma(&cParser);
	}
	else if (pcDirective->Is(PPD_line))
	{
		bResult = ProcessHashLine(pcDirective, &cParser);
	}
	else if (pcDirective->Is(PPD_unknown))
	{
		bResult = ProcessHashUnknown(pcDirective);
	}
	cParser.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A skipped line.  While a group of the current file is being skipped
// its lines are added together when it ends (NoteSkipping); a region is
// only ever added after the others, as the header cache records them.
//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::AddInactiveLine(CSourceFile* pcFile, int iLine)
{
	if ((mpcSkipFile == pcFile) && (iLine >= miSkipStartLine))
	{
		return;
	}
	mpcOutput->AddInactiveLine(pcFile, iLine);
}


//////////////////////////////////////////////////////////////////////////
//
// A skipped group's lines are inactive, comments and blank lines too, so
// from the line after the directive that starts skipping to the line
// before the one that stops it.  bWasParsing is what the conditional
// stack was before pcDirective.
//
//////////////////////////////////////////////////////////////////////////
void CCPPPreprocessor::NoteSkipping(CPPDirective* pcDirective, bool bWasParsing)
{
	bool	bParsing;

	bParsing = mcConditionalStack.IsParsing();
	if (bWasParsing && !bParsing)
	{
		mpcSkipFile = mpcCurrentFile;
		miSkipStartLine = pcDirective->Line() + 1;
	}
	else if (!bWasParsing && bParsing)
	{
		if ((mpcSkipFile == mpcCurrentFile) && (mpcSkipFile != NULL) && (pcDirective->Line() > miSkipStartLine))
		{
			mpcOutput->AddInactiveLines(mpcSkipFile, miSkipStartLine, pcDirective->Line() - 1);
		}
		mpcSkipFile = NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ProcessConditional(CPPDirective* pcDirective)
{
	CPreprocessorTokenParser	cParser;
	CExternalString				cIdentifier;
	SPreprocessedToken			sToken;
	TRISTATE					tResult;
	bool						bDefined;
	bool						bExtra;
	SPPSource*					psSource;
	size						iFileDepth;
	SPreprocessorPosition		sPos;

	//A conditional is inactive if the group it is in is skipped.  #elif, #else and #endif are in their #if's group.
	if (pcDirective->Is(PPD_if) || pcDirective->Is(PPD_ifdef) || pcDirective->Is(PPD_ifndef))
	{
		if (!mcConditionalStack.IsParsing())
		{
			AddInactiveLine(mpcCurrentFile, pcDirective->Line());
		}
	}
	else if (!mcConditionalStack.IsParentParsing())
	{
		AddInactiveLine(mpcCurrentFile, pcDirective->Line());
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
			return Error(&sPos, "Expected an identifier.");
		}
		cParser.SkipWhitespace();
		if (cParser.HasToken())
		{
			//#ifdef and #ifndef name one macro.  [cpp.cond] 1
			cParser.Kill();
			MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
			return Error(&sPos, (char*)(pcDirective->Is(PPD_ifdef) ? "#ifdef names one macro: there is more after it." : "#ifndef names one macro: there is more after it."));
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
		return Error(&sPos, "#elif, #else or #endif without #if.");
	}
	if ((pcDirective->Is(PPD_elif) || pcDirective->Is(PPD_else)) && mcConditionalStack.IsInElse())
	{
		MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
		return Error(&sPos, "#elif or #else after #else.");
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

	if ((pcDirective->Is(PPD_else) || pcDirective->Is(PPD_endif)) && mcConditionalStack.IsParentParsing())
	{
		//#else and #endif have nothing after them.  [cpp.cond] 1
		cParser.Init(pcDirective);
		cParser.SkipWhitespace();
		bExtra = cParser.HasToken();
		cParser.Kill();
		if (bExtra)
		{
			MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
			return Error(&sPos, (char*)(pcDirective->Is(PPD_else) ? "#else has nothing after it." : "#endif has nothing after it."));
		}
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
//
// The operator an alternative token written as a name stands for (and is
// &&), or NULL for any other name.  [lex.digraph] 2, Table 2
//
//////////////////////////////////////////////////////////////////////////
const char* CCPPPreprocessor::AlternativeOperator(const char* szName, size uiLength)
{
	static const char*	aszAlternatives[][2] =
	{
		{ "and", "&&" }, { "and_eq", "&=" }, { "bitand", "&" }, { "bitor", "|" }, { "compl", "~" }, { "not", "!" },
		{ "not_eq", "!=" }, { "or", "||" }, { "or_eq", "|=" }, { "xor", "^" }, { "xor_eq", "^=" },
	};
	size	i;

	for (i = 0; i < sizeof(aszAlternatives) / sizeof(aszAlternatives[0]); i++)
	{
		if ((strlen(aszAlternatives[i][0]) == uiLength) && (memcmp(aszAlternatives[i][0], szName, uiLength) == 0))
		{
			return aszAlternatives[i][1];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
TRISTATE CCPPPreprocessor::EvaluateCondition(CPPDirective* pcDirective)
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
	const char*					szAlternative;

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
		//and, or, not and the rest are the operators they stand for, not names (which would be 0).  [lex.digraph]
		szAlternative = acFinal.Get(i)->IsIdentifier() ? AlternativeOperator(acFinal.Get(i)->szText, acFinal.Get(i)->uiLength) : NULL;
		if (szAlternative)
		{
			sz.Append(szAlternative);
		}
		else
		{
			acFinal.Get(i)->Append(&sz);
		}
	}

	szCalculatorError.Init();
	tResult = EvaluateEquation(sz.Text(), &szCalculatorError);
	if (tResult == TRIERROR)
	{
		MarkPositionForError(pcDirective->Line(), pcDirective->Column(), &sPos);
		szError.Init("Cannot evaluate \"");
		szError.Append(sz);
		szError.Append("\".  ");
		szError.Append(szCalculatorError);
		Error(&sPos, szError.Text());
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
//
// What __has_cpp_attribute is for the attribute named szName: the year
// and month the standard took it (or last changed it), 0 for one it
// doesn't have.  __name__ is name.  The parser takes all of them and none
// changes the code made, as the standard allows.  [cpp.cond], [dcl.attr]
//
//////////////////////////////////////////////////////////////////////////
struct SCppAttribute
{
	const char*		szName;
	int64			lliVersion;
};


static const SCppAttribute	gasCppAttributes[] =
{
	{ "carries_dependency",	200809 },
	{ "deprecated",			201309 },
	{ "fallthrough",		201603 },
	{ "maybe_unused",		201603 },
	{ "nodiscard",			201603 },
	{ "noreturn",			200809 },
};


static int64 CppAttributeVersion(const char* szName, size uiLength)
{
	size	i;

	if ((uiLength > 4) && (memcmp(szName, "__", 2) == 0) && (memcmp(&szName[uiLength - 2], "__", 2) == 0))
	{
		szName += 2;
		uiLength -= 4;
	}
	for (i = 0; i < sizeof(gasCppAttributes) / sizeof(SCppAttribute); i++)
	{
		if ((strlen(gasCppAttributes[i].szName) == uiLength) && (memcmp(gasCppAttributes[i].szName, szName, uiLength) == 0))
		{
			return gasCppAttributes[i].lliVersion;
		}
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
bool CCPPPreprocessor::ReplaceConditionOperators(CArrayPreprocessedTokens* pacSource, CArrayPreprocessedTokens* pacDest)
{
	size					i;
	size					iNumTokens;
	SPreprocessedToken*		psToken;
	SPreprocessedToken*		psNext;
	SPreprocessedToken*		psName;
	CSpecialOperator*		pcSpecial;
	CExternalString			cName;
	bool					bParenthesis;
	int64					lliValue;
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
		pcSpecial = mcSpecialOperators.Get(&cName);
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

			//Builtins, features and the compilers' own attributes are not known.  A standard attribute is, when its
			//name is all there is in the parentheses.
			lliValue = 0;
			psName = pacSource->SafeGet(i + 2);
			psNext = pacSource->SafeGet(i + 3);
			if ((eType == PPSO_HasCppAttribute) && psName && psName->IsIdentifier() && psNext && psNext->IsPunctuator(")"))
			{
				lliValue = CppAttributeVersion(psName->szText, psName->uiLength);
			}
			i++;
			if (!SkipParenthesised(pacSource, &i))
			{
				return Error(psToken, "Expected ')'.");
			}
			AddNumber(lliValue, psToken, pacDest);
			continue;
		}

		if (psNext && psNext->IsPunctuator("(") && !GetDefine(psToken) && !AlternativeOperator(psToken->szText, psToken->uiLength))
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
bool CCPPPreprocessor::GetIncludeName(CArrayPreprocessedTokens* pacTokens, size* piIndex, CChars* pszName, bool* pbSystem)
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
bool CCPPPreprocessor::ProcessHashInclude(CPPDirective* pcDirective, bool bNext)
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
	bool						bOnce;
	bool						bCache;
	uint64						uiContext;

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
		return Error(&sPos, "Expected \"file\" or <file> after #include.");
	}

	cName.Init(szName.Text(), (int)szName.Length());
	FindBestInclude(&cName, bSystem, &pcIncludeFile, &pcHeaderNameMap, bNext);
	if (!pcIncludeFile)
	{
		szError.Init("Could not include file ");
		szError.Append(bSystem ? '<' : '"');
		szError.Append(szName);
		szError.Append(bSystem ? '>' : '"');
		Error(&sPos, szError.Text());
		szError.Kill();
		szName.Kill();
		return false;
	}
	szName.Kill();

	bOnce = mcPragmaOnce.Get(pcIncludeFile) != NULL;
	if (masRecorders.NumElements() > 0)
	{
		NotePragmaQuery(pcIncludeFile, bOnce);
	}
	if (bOnce)
	{
		miIncludeDepth++;
		LogInclude(pcIncludeFile);
		miIncludeDepth--;
		return true;
	}

	//A header another unit preprocessed with the same macros is replayed, otherwise it is preprocessed and kept.
	bCache = CanUseHeaderCache();
	if (bCache && ReplayCachedHeader(pcIncludeFile, pcHeaderNameMap))
	{
		return true;
	}
	uiContext = bCache ? HeaderContext() : 0;
	bResult = PushFile(pcIncludeFile, pcHeaderNameMap);
	if (bResult)
	{
		LogInclude(pcIncludeFile);
		if (bCache)
		{
			BeginRecording(pcIncludeFile, pcHeaderNameMap, uiContext);
		}
	}
	return bResult;
}

