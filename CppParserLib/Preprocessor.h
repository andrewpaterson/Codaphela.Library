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
#include "PPTokenReplacementsHolder.h"
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
#include "PreprocessorStack.h"


typedef CArrayTemplate<CHeaderFiles*> CArrayHeaderFiles;
typedef CArrayTemplate<CHeaderFiles> CArrayHeaderNameMap;


class CPreprocessor
{
protected:
	CConditionalStack		mcConditionalStack;
	CSourceFile*			mpcCurrentFile;
	CASCIITree				mcDirectives;
	CDefineMap				mcDefines;
	CPPTokenMemory			mcDefineReplacementMemory;
	CMemoryStackExtended	mcDefineStrings;
	CSpecialOperatorMap		mcSpecialOperators;
	CDefineArguments		mcArguments;
	CArrayHeaderFiles		mcHeaderNames;
	CHeaderNameMapStack		mcHeadersStack;

	CChars*					mpszBlocksLog;
	CChars*					mpszIncludesLog;
	int						miDefineReuse;
	int						miIncludeDepth;
	int						miBlockReuse;

	CPreprocessorStack		mcStack;

	bool					mbLogBlocks;
	bool					mbLogInlucdes;
	bool					mbDumpLogs;
	CChars					mszVaArgs;

public:
	static bool			Preprocess(char* szSource, CChars* szDest, bool bShowFileAndLine = false);

public:
	void 				Init(CConfig* pcConfig);
	void 				Kill(void);
	void				InitPlatformSpecific(void);
	void				AddConfigDefines(CConfig* pcConfig);	
	void				LogIncludes(CChars* pszIncludesLog);
	void				LogBlocks(CChars* pszBlocksLog);
	void				LogBlocks(bool bLogBlocks);
	void				LogDumping(bool bDumpLogs);
	void				AddIncludeDirectories(CArrayHeaderNameMap* pcHeaderNames);
	void				AddIncludeDirectory(CHeaderFiles* pcHeaderNames);
	int					GetBlockReuse(void);

	CDefine*			GetDefine(CExternalString* pcString, bool bExact);
	CDefine*			GetDefine(char* szName, bool bExact);
	CDefine* 			AddDefine(char* szDefine);
	CDefine*			AddDefine(char* szDefine, char* szReplacement, CPPTokenMemory* pcTokenMemory);
	CDefine*			AddSpecialDefine(char* szDefine);

	CSpecialOperator*	GetSpecialOperator(CExternalString* pcString, bool bExact);
	CSpecialOperator*	GetSpecialOperator(char* szName, bool bExact);
	CSpecialOperator*	AddSpecialOperator(char* szSpecialOperator, EPreprocessorSpecialOperator eType);

	bool				PreprocessFile(CSourceFile* pcFile, CSourceFile* pcFromFile);
	bool				PreprocessBlockSets(CPPBlockSetArray* pacBlockSets);
	SPPTokenBlockIndex	PreprocessDirectiveTokens(CPPTokenList* pcSourceTokens, int iBlock, int iToken);
	SPPTokenBlockIndex	PreprocessNormalLineTokens(CPPTokenList* pcSourceTokens, int iBlock, int iToken);
	bool				PreprocessTranslationUnit(CTranslationUnit* pcFile);

	bool 				ProcessHashDefine(CPreprocessorTokenParser* pcParser);
	bool 				ProcessHashUndef(CPreprocessorTokenParser* pcParser);
	bool 				ProcessHashInclude(CPreprocessorTokenParser* pcParser);
	SPPTokenBlockIndex	ProcessHashEndif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashIfndef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashIfdef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashElse(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashIf(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	SPPTokenBlockIndex 	ProcessHashElif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	bool 				ProcessHashError(CPreprocessorTokenParser* pcParser);
	bool 				ProcessHashPragma(CPreprocessorTokenParser* pcParser);
	bool				ProcessNormalLine(CPreprocessorTokenParser* pcParser);

	TRISTATE			EvaluateEquation(char* szText, CChars* szCalculatorError);

	bool				ProcessIdentifierDirective(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, int iDepth);
	bool				ProcessIdentifierNormalLine(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, int iDepth);
	bool				ProcessDefinedIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	bool				ProcessHasIncludeIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	bool				ProcessHasAttributeIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	bool				ProcessHasCPPAttributeIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	bool				ProcessHasBuiltInIdentifier(CPPTokenList* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	bool				ProcessHashDefineBracketted(CPreprocessorTokenParser* pcParser, CDefine* pcDefine);
	bool				ProcessUnknownDirective(CPreprocessorTokenParser* pcParser, CPPTokenList* pcDest);
	bool				ProcessIncludeFile(CPreprocessorTokenParser* pcParser, CHeaderFile** ppcCFile, CHeaderFiles** ppcHeaderNameMap);
	bool				ProcessDirectiveLine(CPPTokenList* pcTokenHolder, CPreprocessorTokenParser* pcParser, int iDepth);
	bool				ProcessNormalLine(CPPTokenList* pcTokenHolder, CPreprocessorTokenParser* pcParser, int iDepth);
	bool				ProcessSingleHash(CPPTokenList* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser);
	bool				ProcessDoubleHash(CPPTokenList* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser);
	void				FindBestInclude(CExternalString* pcInclude, bool bSystemFile, CHeaderFile** ppcCFile, CHeaderFiles** ppcHeaderNameMap);
	bool				FindArguments(CPreprocessorTokenParser* pcParser, CArrayPPTokenLists* pacArguments);
	SPPTokenBlockIndex	Condition(CPPConditional* pcCond, SPPTokenBlockIndex iLine);
	void				AddTokenToArgument(CPPTokenList* pcArgument, CPPToken* pcToken);
	CPPToken*			AddToken(CPPToken* pcToken, CArrayPPTokenPtrs* pacTokenPts);
	bool				ExpandDirectiveTokenIfNecessary(CPPToken* pcToken, CPPTokenList* pcDest, CPreprocessorTokenParser* pcParser, int iDepth);
	bool				ExpandNormalLineTokenIfNecessary(CPPToken* pcToken, CPPTokenList* pcDest, CPreprocessorTokenParser* pcParser, int iDepth);
	void				ExpandDirectiveReplacement(CPPReplacement* pcReplacement, CPPTokenList* pcDest, int iDepth);
	void				ExpandReplacementNormalLine(CPPReplacement* pcReplacement, CPPTokenList* pcDest, int iDepth);
	CPPToken*			QuoteTokens(CPPTokenList* pcDest, CPPTokenListHolder* pcHolder);
	CPPToken*			ConcaternateTokens(CPPTokenList* pcDest, CPPToken* pcLeft, CPPToken* pcRight);
	bool				TokeniseFile(CSourceFile* pcFile);
	void				DeltaDefines(CArrayNamedDefines* pcDelta);
	void				LogBlocks(CPPBlockSetArray* pacBlockSets, SPPTokenBlockIndex sResult);
	void				LogInclude(CSourceFile* pcFile);
	void				TranslationUnitLogging(CTranslationUnit* pcFile);
	CSpecialOperator*	ProcessSpecialOperator(CPreprocessorTokenParser* pcParser);

	void				KillArguments(SDefineArgument* psArguments);
	char*				GetFileName(void);
	CPPToken*			DuplicatePPToken(CPPToken* pcSource);

	void				MarkPositionForError(CPreprocessorTokenParser* pcParser, SPreprocessorPosition* psPos);
	void				MarkPositionForError(int iLine, int iColumn, SPreprocessorPosition* psPos);

	void				StackPush(CPPTokenList* pcTokenList, CPPTokenMemory* pcTokenMemory);
	void				StackPop(void);

	void				AddComma(CPPTokenList* pcDest);
	void				AddZero(CPPTokenList* pcDest);
};


#endif //__PREPROCESSOR_H__

