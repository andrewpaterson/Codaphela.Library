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
#include "HeaderFiles.h"
#include "PreprocessorPosition.h"
#include "HeaderNameMapStack.h"


typedef CArrayTemplate<CHeaderFiles*> CArrayHeaderFiles;
typedef CArrayTemplate<CHeaderFiles> CArrayHeaderNameMap;


class CPreprocessor
{
protected:
	CConditionalStack			mcConditionalStack;
	CSourceFile*				mpcCurrentFile;
	CPPAbstractHolder*			mpcCurrentLine;
	CPreprocessorTokenParser*	mpcCurrentLineParser;
	CASCIITree					mcDirectives;
	CDefineMap					mcDefines;
	CSpecialOperatorMap			mcSpecialOperators;
	CDefineArguments			mcArguments;
	int							miIncludeDepth;
	CArrayHeaderFiles			mcHeaderNames;
	CHeaderNameMapStack			mcHeadersStack;

	CPPTokenList*				mpcProcessedTokens;
	CPPTokenMemory*				mpcTokenMemory;

	int							miBlockReuse;
	BOOL						mbLogBlocks;
	BOOL						mbLogInlucdes;
	BOOL						mbDumpLogs;
	CChars*						mpszBlocksLog;
	CChars*						mpszIncludesLog;
	int							miDefineReuse;
	CChars						mszVaArgs;

	int							miProcessTokensCalledCount;

public:
	static void			Preprocess(char* szSource, CChars* szDest);

public:
	void 				Init(CConfig* pcConfig, CPPTokenMemory* pcTokenMemory, CPPTokenList* pcProcessedTokens);
	void				Init(CConfig* pcConfig, CTranslationUnit* pcFile);
	void 				Kill(void);
	void				InitPlatformSpecific(void);
	void				AddConfigDefines(CConfig* pcConfig);	
	void				LogIncludes(CChars* pszIncludesLog);
	void				LogBlocks(CChars* pszBlocksLog);
	void				LogBlocks(BOOL bLogBlocks);
	void				LogDumping(BOOL bDumpLogs);
	void				AddIncludeDirectories(CArrayHeaderNameMap* pcHeaderNames);
	void				AddIncludeDirectory(CHeaderFiles* pcHeaderNames);
	int					GetBlockReuse(void);

	CDefine*			GetDefine(CExternalString* pcString, BOOL bExact);
	CDefine*			GetDefine(char* szName, BOOL bExact);
	CDefine* 			AddDefine(char* szDefine);
	CDefine*			AddDefine(char* szDefine, char* szReplacement);
	CDefine*			AddSpecialDefine(char* szDefine);

	CSpecialOperator*	GetSpecialOperator(CExternalString* pcString, BOOL bExact);
	CSpecialOperator*	GetSpecialOperator(char* szName, BOOL bExact);
	CSpecialOperator*	AddSpecialOperator(char* szSpecialOperator, EPreprocessorSpecialOperator eType);

	BOOL				PreprocessFile(CSourceFile* pcFile, CSourceFile* pcFromFile);
	BOOL				PreprocessBlockSets(CPPBlockSetArray* pacBlockSets);
	SPPTokenBlockIndex	PreprocessDirectiveTokens(CPPTokenList* pcSourceTokens, int iBlock, int iToken);
	SPPTokenBlockIndex	PreprocessNormalLineTokens(CPPTokenList* pcSourceTokens, int iBlock, int iToken);
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

	BOOL				ProcessIdentifierDirective(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ProcessIdentifierNormalLine(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ProcessDefinedIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasIncludeIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasAttributeIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasCPPAttributeIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasBuiltInIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHashDefineBracketted(CPreprocessorTokenParser* pcParser, CDefine* pcDefine);
	BOOL				ProcessUnknownDirective(CPreprocessorTokenParser* pcParser, CPPTokenList* pcDest);
	BOOL				ProcessIncludeFile(CPreprocessorTokenParser* pcParser, CHeaderFile** ppcCFile, CHeaderFiles** ppcHeaderNameMap);
	BOOL				ProcessDirectiveLine(CPPTokenList* pcTokenHolder, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ProcessNormalLine(CPPTokenList* pcTokenHolder, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ProcessSingleHash(CPPTokenList* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessDoubleHash(CPPTokenList* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser);
	void				FindBestInclude(CExternalString* pcInclude, BOOL bSystemFile, CHeaderFile** ppcCFile, CHeaderFiles** ppcHeaderNameMap);
	BOOL				FindArguments(CPreprocessorTokenParser* pcParser, CArrayPPTokenHolders* pacArguments);
	SPPTokenBlockIndex	Condition(CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	void				AddTokenToArgument(CPPTokenList* pcArgument, CPPToken* pcToken);
	CPPToken*			AddToken(CPPToken* pcToken, CArrayPPTokenPtrs* pacTokenPts);
	BOOL				ExpandDirectiveTokenIfNecessary(CPPToken* pcToken, CPPTokenList* pcDest, CPreprocessorTokenParser* pcParser, int iDepth);
	BOOL				ExpandNormalLineTokenIfNecessary(CPPToken* pcToken, CPPTokenList* pcDest, CPreprocessorTokenParser* pcParser, int iDepth);
	void				ExpandDirectiveReplacement(CPPReplacement* pcReplacement, CPPTokenList* pcDest, int iDepth);
	void				ExpandReplacementNormalLine(CPPReplacement* pcReplacement, CPPTokenList* pcDest, int iDepth);
	CPPToken*			QuoteTokens(CPPTokenList* pcDest, CPPAbstractHolder* pcHolder);
	CPPToken*			ConcaternateTokens(CPPTokenList* pcDest, CPPToken* pcLeft, CPPToken* pcRight);
	BOOL				TokeniseFile(CSourceFile* pcFile);
	void				DeltaDefines(CArrayNamedDefines* pcDelta);
	void				LogBlocks(CPPBlockSetArray* pacBlockSets, SPPTokenBlockIndex sResult);
	void				LogInclude(CSourceFile* pcFile);
	void				TranslationUnitLogging(CTranslationUnit* pcFile);
	CSpecialOperator*	ProcessSpecialOperator(CPreprocessorTokenParser* pcParser);
	void				MarkPositionForError(SPreprocessorPosition* psPos);
	void				KillArguments(SDefineArgument* psArguments);
	char*				GetFileName(void);
	CPPToken*			DuplicatePPToken(CPPToken* pcSource);

	void				AddComma(CPPTokenList* pcDest);
	void				AddZero(CPPTokenList* pcDest);
};


#endif //__PREPROCESSOR_H__

