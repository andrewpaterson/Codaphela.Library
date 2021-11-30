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
#ifndef __PREPROCESSOR_H__
#define __PREPROCESSOR_H__
#include "BaseLib/Chars.h"
#include "BaseLib/MapStringString.h"
#include "BaseLib/MapStringString.h"
#include "BaseLib/TextParser.h"
#include "DefineMap.h"
#include "SpecialOperatorMap.h"
#include "ConditionalStack.h"
#include "ASCIITree.h"
#include "PreprocessorTokeniser.h"
#include "PPDirective.h"
#include "PPHolder.h"
#include "PPText.h"
#include "PPTextWithSource.h"
#include "PPHashes.h"
#include "PPReplacement.h"
#include "PreprocessorTokenParser.h"
#include "DefineArguments.h"
#include "Library.h"
#include "TranslationUnit.h"
#include "PPTokenBlockIndex.h"
#include "HeaderNameMap.h"
#include "PreprocessorPosition.h"
#include "HeaderNameMapStack.h"


class CPreprocessor
{
protected:
	CConditionalStack				mcConditionalStack;
	CCFile*							mpcCurrentFile;
	CPPLine*						mpcCurrentLine;
	CPreprocessorTokenParser*		mpcCurrentLineParser;
	CASCIITree						mcDirectives;
	CDefineMap						mcDefines;
	CSpecialOperatorMap				mcSpecialOperators;
	CDefineArguments				mcArguments;
	int								miIncludeDepth;
	CArrayHeaderNameMapPtr			mcHeaderNames;
	CHeaderNameMapStack				mcHeadersStack;

	CPPTokenHolder*					mpcPost;
	CTranslationUnit*				mpcUnit;
	CPPTokens*						mpcTokens;

	int								miBlockReuse;
	BOOL							mbLogBlocks;
	BOOL							mbLogInlucdes;
	BOOL							mbDumpLogs;
	CChars*							mpszBlocksLog;
	CChars*							mpszIncludesLog;
	int								miDefineReuse;
	CChars							mszVaArgs;

	int								miProcessTokensCalledCount;

public:
	static void			Preprocess(char* szSource, CChars* szDest);

public:
	void 				Init(CConfig* pcConfig, CPPTokens* pcTokens);
	void 				Kill(void);
	void				InitPlatformSpecific(void);
	void				AddConfigDefines(CConfig* pcConfig);	
	void				LogIncludes(CChars* pszIncludesLog);
	void				LogBlocks(CChars* pszBlocksLog);
	void				LogBlocks(BOOL bLogBlocks);
	void				LogDumping(BOOL bDumpLogs);
	void				AddIncludeDirectories(CArrayHeaderNameMap* pcHeaderNames);
	void				AddIncludeDirectory(CHeaderNameMap* pcHeaderNames);
	int					GetBlockReuse(void);

	CDefine*			GetDefine(CExternalString* pcString, BOOL bExact);
	CDefine*			GetDefine(char* szName, BOOL bExact);
	CDefine* 			AddDefine(char* szDefine);
	CDefine*			AddDefine(char* szDefine, char* szReplacement);
	CDefine*			AddSpecialDefine(char* szDefine);

	CSpecialOperator*	GetSpecialOperator(CExternalString* pcString, BOOL bExact);
	CSpecialOperator*	GetSpecialOperator(char* szName, BOOL bExact);
	CSpecialOperator*	AddSpecialOperator(char* szSpecialOperator, EPreprocessorSpecialOperator eType);

	BOOL				PreprocessFile(CCFile* pcFile, CCFile* pcFromFile);
	SPPTokenBlockIndex	PreprocessTokens(CPPTokenHolder* pcDestTokens, CPPTokens* pcTokens, CPPTokenHolder* pcSourceTokens, int iBlock, int iToken);
	BOOL				PreprocessTranslationUnit(CTranslationUnit* pcFile);

	BOOL 				ProcessHashDefine(CPreprocessorTokenParser* pcParser);
	BOOL 				ProcessHashUndef(CPreprocessorTokenParser* pcParser);
	BOOL 				ProcessHashInclude(CPreprocessorTokenParser* pcParser);
	SPPTokenBlockIndex	ProcessHashEndif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashIfndef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashIfdef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashElse(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashIf(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashElif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	BOOL 				ProcessHashError(CPreprocessorTokenParser* pcParser);
	BOOL 				ProcessHashPragma(CPreprocessorTokenParser* pcParser);
	BOOL				ProcessNormalLine(CPreprocessorTokenParser* pcParser);

	TRISTATE			EvaluateEquation(char* szText, CChars* szCalculatorError);

	BOOL				ProcessIdentifierDirective(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ProcessIdentifierNormalLine(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ProcessDefinedIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasIncludeIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasAttributeIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasCPPAttributeIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasBuiltInIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHashDefineBracketted(CPreprocessorTokenParser* pcParser, CDefine* pcDefine);
	BOOL				ProcessUnknownDirective(CPreprocessorTokenParser* pcParser, CPPTokenHolder* pcDest);
	BOOL				ProcessIncludeFile(CPreprocessorTokenParser* pcParser, CHeaderFile** ppcCFile, CHeaderNameMap** ppcHeaderNameMap);
	BOOL				ProcessDirectiveLine(CPPTokenHolder* pcTokenHolder, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ProcessNormalLine(CPPTokenHolder* pcTokenHolder, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ProcessSingleHash(CPPTokenHolder* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessDoubleHash(CPPTokenHolder* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser);
	void				FindBestInclude(CExternalString* pcInclude, BOOL bSystemFile, CHeaderFile** ppcCFile, CHeaderNameMap** ppcHeaderNameMap);
	BOOL				FindArguments(CPreprocessorTokenParser* pcParser, CArrayPPTokenHolders* pacArguments);
	SPPTokenBlockIndex	Condition(CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	void				AddTokenToArgument(CPPTokenHolder* pcArgument, CPPToken* pcToken);
	CPPToken*			AddToken(CPPToken* pcToken, CArrayPPTokenPtrs* pcTokens);
	BOOL				ExpandDirectiveTokenIfNecessary(CPPToken* pcToken, CPPTokenHolder* pcDest, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ExpandNormalLineTokenIfNecessary(CPPToken* pcToken, CPPTokenHolder* pcDest, CPreprocessorTokenParser* pcParser, int iDepth);
	void				ExpandDirectiveReplacement(CPPReplacement* pcReplacement, CPPTokenHolder* pcDest, int iDepth);
	void				ExpandReplacementNormalLine(CPPReplacement* pcReplacement, CPPTokenHolder* pcDest, int iDepth);
	CPPToken*			QuoteTokens(CPPTokenHolder* pcDest, CPPAbstractHolder* pcHolder);
	CPPToken*			ConcaternateTokens(CPPTokenHolder* pcDest, CPPToken* pcLeft, CPPToken* pcRight);
	BOOL				TokeniseFile(CCFile* pcFile);
	void				DeltaDefines(CArrayNamedDefines* pcDelta, CPPTokens* pcTokens);
	void				LogBlocks(CCFile* pcFile, SPPTokenBlockIndex sResult);
	void				LogIncludes(CCFile* pcFile);
	void				TranslationUnitLogging(CTranslationUnit* pcFile);
	CSpecialOperator*	ProcessSpecialOperator(CPreprocessorTokenParser* pcParser);
	void				MarkPositionForError(SPreprocessorPosition* psPos);
	void				KillArguments(SDefineArgument* psArguments);

	void				AddComma(CPPTokenHolder* pcDest);
	void				AddZero(CPPTokenHolder* pcDest);
};


#endif //__PREPROCESSOR_H__

