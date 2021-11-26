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
#include "CBlockToken.h"
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
	CMemoryStackExtended*			mpcStack;

	int								miBlockReuse;
	BOOL							mbLogBlocks;
	BOOL							mbLogInlucdes;
	BOOL							mbDumpLogs;
	CChars*							mpszBlocksLog;
	CChars*							mpszIncludesLog;
	int								miDefineReuse;
	CChars							mszVaArgs;

	CPPTextWithSource				mcPPComma;
	CPPToken*						mpcPPComma;

public:
	void 				Init(CConfig* pcConfig, CMemoryStackExtended* pcStack);
	void 				Kill(void);
	void				InitPlatformSpecific(void);
	void				AddConfigDefines(CConfig* pcConfig);	
	void				LogIncludes(CChars* pszIncludesLog);
	void				LogBlocks(CChars* pszBlocksLog);
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

	BOOL				PreprocessBlockSets(CCFile* pcFile, CCFile* pcFromFile);
	SCTokenBlock		PreprocessTokens(CPPTokenHolder* pcDestTokens, CMemoryStackExtended* pcStack, CPPTokenHolder* pcSourceTokens, int iBlock, int iToken);
	BOOL				PreprocessTranslationUnit(CTranslationUnit* pcFile);

	BOOL 				ProcessHashDefine(CPreprocessorTokenParser* pcParser);
	void 				ProcessHashUndef(CPreprocessorTokenParser* pcParser);
	BOOL 				ProcessHashInclude(CPreprocessorTokenParser* pcParser);
	SCTokenBlock		ProcessHashEndif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine);
	SCTokenBlock 		ProcessHashIfndef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine);
	SCTokenBlock 		ProcessHashIfdef(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine);
	SCTokenBlock 		ProcessHashElse(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine);
	SCTokenBlock 		ProcessHashIf(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine);
	SCTokenBlock 		ProcessHashElif(CPreprocessorTokenParser* pcParser, CPPConditional* pcCond, SCTokenBlock iLine);
	void 				ProcessHashError(CPreprocessorTokenParser* pcParser);
	BOOL 				ProcessHashPragma(CPreprocessorTokenParser* pcParser);
	BOOL				ProcessNormalLine(CPreprocessorTokenParser* pcParser);

	TRISTATE			EvaluateEquation(char* szText, CChars* szCalculatorError);

	BOOL				ProcessIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser, BOOL bAllowConditional, int iDepth);
	BOOL				ProcessDefinedIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasIncludeIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasAttributeIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasCPPAttributeIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHasBuiltInIdentifier(CPPTokenHolder* pcDest, CPPText* pcText, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessHashDefineBracketted(CPreprocessorTokenParser* pcParser, CDefine* pcDefine);
	BOOL				ProcessIncludeFile(CPreprocessorTokenParser* pcParser, CHeaderFile** ppcCFile, CHeaderNameMap** ppcHeaderNameMap);
	BOOL				ProcessLine(CPPTokenHolder* pcTokenHolder, CPreprocessorTokenParser* pcParser, BOOL bAllowDefined, int iDepth);
	BOOL				ProcessSingleHash(CPPTokenHolder* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser);
	BOOL				ProcessDoubleHash(CPPTokenHolder* pcDest, CPPHashes* pcHash, CPreprocessorTokenParser* pcParser);
	void				FindBestInclude(CExternalString* pcInclude, BOOL bSystemFile, CHeaderFile** ppcCFile, CHeaderNameMap** ppcHeaderNameMap);
	BOOL				FindArguments(CPreprocessorTokenParser* pcParser, CArrayPPTokenHolders* pacArguments);
	SCTokenBlock		Condition(CPPConditional* pcCond, SCTokenBlock iLine);
	void				AddTokenToArgument(CPPTokenHolder* pcArgument, CPPToken* pcToken);
	CPPToken*			AddToken(CPPToken* pcToken, CArrayPPTokens* pcTokens);
	void				ExpandDefined(CPPAbstractHolder* pcHolder, CDefine* pcDefine, BOOL bAllowDefined, int iDepth);
	BOOL				ExpandTokenIfNecessary(CPPToken* pcToken, CPPTokenHolder* pcDest, CPreprocessorTokenParser* pcParser, BOOL bAllowDefined, int iDepth);
	void				ExpandReplacement(CPPReplacement* pcReplacement, CPPTokenHolder* pcDest, BOOL bAllowDefined, int iDepth);
	CPPToken*			QuoteTokens(CPPTokenHolder* pcDest, CPPAbstractHolder* pcHolder);
	CPPToken*			ConcaternateTokens(CPPTokenHolder* pcDest, CPPToken* pcLeft, CPPToken* pcRight);
	BOOL				TokeniseFile(CCFile* pcFile);
	void				DeltaDefines(CArrayNamedDefines* pcDelta, CMemoryStackExtended* pcStack);
	void				LogBlocks(CCFile* pcFile, SCTokenBlock sResult);
	void				LogIncludes(CCFile* pcFile);
	void				TranslationUnitLogging(CTranslationUnit* pcFile);
	CSpecialOperator*	ProcessSpecialOperator(CPreprocessorTokenParser* pcParser);
	void				MarkPositionForError(SPreprocessorPosition* psPos);
	void				KillArguments(SDefineArgument* psArguments);

	static void			Preprocess(char* szSource, CChars* szDest);
};


#endif //__PREPROCESSOR_H__

