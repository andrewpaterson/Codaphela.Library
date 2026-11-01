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
#include "DefineMap.h"
#include "SpecialOperatorMap.h"
#include "ConditionalStack.h"
#include "PreprocessorTokeniser.h"
#include "PPDirective.h"
#include "PPText.h"
#include "PPTextWithSource.h"
#include "PPHashes.h"
#include "PPReplacement.h"
#include "PreprocessorTokenParser.h"
#include "Library.h"
#include "TranslationUnit.h"
#include "HeaderFiles.h"
#include "PreprocessorPosition.h"
#include "HeaderNameMapStack.h"
#include "PreprocessedTokens.h"
#include "CPPErrors.h"
#include "BaseLib/ArrayTemplatePtr.h"


typedef CArrayTemplatePtr<CHeaderFiles> CArrayHeaderFiles;
typedef CArrayTemplate<CHeaderFiles> CArrayHeaderNameMap;
typedef CArrayTemplate<CArrayPreprocessedTokens> CArrayPreprocessedTokenArrays;
typedef CArrayTemplate<int64> CArrayDefineIDs;


enum EPPSourceType
{
	PPST_File,
	PPST_Macro,
	PPST_Argument,
};


//Somewhere the preprocessor reads tokens from.  Sources are stacked: a file includes a file, a macro expansion is read
//before the rest of the line that invoked it.  [cpp.rescan]
struct SPPSource
{
	EPPSourceType				eType;
	CArrayPreprocessedTokens	acTokens;				//The current line of a file, or the tokens of a macro or argument.
	size						iToken;

	//PPST_File.
	CSourceFile*				pcFile;
	CSourceFile*				pcIncludingFile;
	size						iBlockSet;
	size						iLine;					//Index of the next line or directive in the block set.
	size						iConditionalDepth;		//Conditionals open when the file was entered.
	bool						bHeaderDirectory;		//A directory was pushed onto the header stack for it.
	int							iCurrentLine;			//Source line of the tokens in acTokens.

	//PPST_Macro.
	int64						lliDefineID;			//The macro is not expanded again while this source is read.

	void	Init(EPPSourceType eType);
	void	Kill(void);
};


typedef CArrayTemplate<SPPSource> CArrayPPSources;


class CPreprocessor
{
protected:
	CConditionalStack				mcConditionalStack;
	CSourceFile*					mpcCurrentFile;
	CDefineMap						mcDefines;
	CPPTokenMemory					mcDefineReplacementMemory;
	CMemoryStackExtended			mcDefineStrings;
	CSpecialOperatorMap				mcSpecialOperators;
	CArrayHeaderFiles				mcHeaderNames;
	CHeaderNameMapStack				mcHeadersStack;

	CChars*							mpszIncludesLog;
	size							miIncludeDepth;
	bool							mbLogIncludes;
	bool							mbDumpLogs;
	CChars							mszVaArgs;

	CArrayPPSources					macSources;
	CArrayPreprocessedTokens		macPushedBack;			//Tokens read ahead while looking for a '('.
	CArrayDefineIDs					maiDisabledDefines;		//Macros being expanded.
	CPreprocessedTokens*			mpcOutput;
	uint8							muiPendingFlags;		//Spacing of a macro that expanded to nothing, given to the next token.
	bool							mbInCondition;			//Expanding the expression of a #if.
	bool							mbDefinedOperand;		//The next identifier is the operand of defined.
	bool							mbError;
	bool							mbKeepPragmaPack;		//#pragma pack(...) is output as __pragma(pack(...)) (CConfig).
	int								miCounter;
	CCPPErrors						mcErrors;
	CCPPErrors						mcWarnings;				//#warning's.

public:
	static bool			Preprocess(char* szSource, CChars* pszDest);
	CCPPErrors*			GetErrors(void);
	CCPPErrors*			GetWarnings(void);

public:
	void 				Init(CConfig* pcConfig);
	void				EmitPragmaPack(CPreprocessorTokenParser* pcParser);
	void 				Kill(void);
	void				InitPlatformSpecific(void);
	void				AddConfigDefines(CConfig* pcConfig);	
	void				LogIncludes(CChars* pszIncludesLog);
	void				LogDumping(bool bDumpLogs);
	void				AddIncludeDirectories(CArrayHeaderNameMap* pcHeaderNames);
	void				AddIncludeDirectory(CHeaderFiles* pcHeaderNames);

	CDefine*			GetDefine(CExternalString* pcString, bool bExact);
	CDefine*			GetDefine(char* szName, bool bExact);
	CDefine* 			AddDefine(char* szDefine);
	CDefine*			AddDefine(char* szDefine, char* szReplacement, CPPTokenMemory* pcTokenMemory);
	CDefine*			AddSpecialDefine(char* szDefine);
	CDefine*			AddNumberDefine(char* szDefine, int64 lliValue);

	CSpecialOperator*	GetSpecialOperator(CExternalString* pcString, bool bExact);
	CSpecialOperator*	GetSpecialOperator(char* szName, bool bExact);
	CSpecialOperator*	AddSpecialOperator(char* szSpecialOperator, EPreprocessorSpecialOperator eType);

	bool				PreprocessTranslationUnit(CTranslationUnit* pcFile);

protected:
	//Sources.
	bool				PushFile(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap);
	void				PushTokens(EPPSourceType eType, CArrayPreprocessedTokens* pacTokens, int64 lliDefineID);
	bool				PopSource(void);
	bool				NextRaw(SPreprocessedToken* psToken);
	bool				NextFileToken(SPPSource* psSource, SPreprocessedToken* psToken, bool* pbChanged);
	void				PushBack(SPreprocessedToken* psToken);
	bool				TokeniseFile(CSourceFile* pcFile);

	//Macro expansion.
	bool				Next(SPreprocessedToken* psToken);
	bool				PeekOpenParenthesis(void);
	bool				CollectArguments(CDefine* pcDefine, SPreprocessedToken* psName, CArrayPreprocessedTokenArrays* pacArguments);
	bool				ExpandMacro(CDefine* pcDefine, SPreprocessedToken* psName, CArrayPreprocessedTokenArrays* pacArguments);
	void				ExpandArgument(CArrayPreprocessedTokens* pacRaw, CArrayPreprocessedTokens* pacExpanded);
	void				ExpandTokens(CArrayPreprocessedTokens* pacSource, CArrayPreprocessedTokens* pacDest);
	void				Stringify(CArrayPreprocessedTokens* pacArgument, SPreprocessedToken* psResult);
	bool				Paste(SPreprocessedToken* psLeft, SPreprocessedToken* psRight, SPreprocessedToken* psResult);
	void				ExpandSpecial(CDefine* pcDefine, SPreprocessedToken* psToken);
	bool				SkipPragmaOperator(void);
	CArrayPreprocessedTokens*	GetBody(CDefine* pcDefine);
	CDefine*			GetDefine(SPreprocessedToken* psToken);
	bool				IsDisabled(int64 lliDefineID);
	bool				IsDefined(SPreprocessedToken* psToken);
	SPPSource*			GetCurrentFileSource(void);

	//Converting tokenised lines.
	void				ConvertTokens(CPPTokenList* pcTokens, CSourceFile* pcFile, CDefine* pcDefine, bool bStartOfLine, CArrayPreprocessedTokens* pacDest);
	CPPToken*			ConvertDecorators(CPPTokenList* pcTokens, size* piIndex, CSourceFile* pcFile, uint8 uiFlags, CArrayPreprocessedTokens* pacDest);

	//Directives.
	bool				ProcessDirective(CPPDirective* pcDirective);
	bool				ProcessConditional(CPPDirective* pcDirective);
	bool 				ProcessHashDefine(CPreprocessorTokenParser* pcParser);
	bool				ProcessHashDefineBracketted(CPreprocessorTokenParser* pcParser, CDefine* pcDefine);
	bool 				ProcessHashUndef(CPreprocessorTokenParser* pcParser);
	bool 				ProcessHashInclude(CPPDirective* pcDirective, bool bNext);
	bool 				ProcessHashError(CPPDirective* pcDirective, CPreprocessorTokenParser* pcParser, bool bWarning);
	bool 				ProcessHashPragma(CPreprocessorTokenParser* pcParser);
	bool 				ProcessHashUnknown(CPPDirective* pcDirective);
	TRISTATE			EvaluateCondition(CPPDirective* pcDirective);
	bool				ReplaceConditionOperators(CArrayPreprocessedTokens* pacSource, CArrayPreprocessedTokens* pacDest);
	bool				GetIncludeName(CArrayPreprocessedTokens* pacTokens, size* piIndex, CChars* pszName, bool* pbSystem);
	void				FindBestInclude(CExternalString* pcInclude, bool bSystemFile, CHeaderFile** ppcCFile, CHeaderFiles** ppcHeaderNameMap, bool bNext);
	TRISTATE			EvaluateEquation(char* szText, CChars* szCalculatorError);

	void				LogInclude(CSourceFile* pcFile);
	void				TranslationUnitLogging(CTranslationUnit* pcFile);
	char*				GetFileName(void);
	CPPToken*			DuplicatePPToken(CPPToken* pcSource);
	CPPTokenMemory*		GetTokenMemory(void);
	void				EmitToken(SPreprocessedToken* psToken);
	char*				AddString(const char* szText, size uiLength);
	void				AddNumber(int64 lliValue, SPreprocessedToken* psPosition, CArrayPreprocessedTokens* pacDest);
	bool				Error(SPreprocessedToken* psToken, char* szMessage);
	bool				Error(SPreprocessorPosition* psPosition, char* szMessage, char* szDetail = NULL);

	void				MarkPositionForError(CPreprocessorTokenParser* pcParser, SPreprocessorPosition* psPos);
	void				MarkPositionForError(size iLine, size iColumn, SPreprocessorPosition* psPos);
};


#endif //__PREPROCESSOR_H__

