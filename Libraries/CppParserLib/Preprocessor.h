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
#include "BaseLib/MapPtrPtr.h"
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
#include "PPHeaderCache.h"
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
	uint64						uiDirectory;			//The file's directory, hashed.
	uint64						uiDirectories;			//The directories of the file and the files including it, hashed (a quoted #include looks in them).
	bool						bRecorded;				//The file's output is being kept (CPreprocessor's BeginRecording).

	//PPST_Macro.
	int64						lliDefineID;			//The macro is not expanded again while this source is read.

	void	Init(EPPSourceType eType);
	void	Kill(void);
};


typedef CArrayTemplate<SPPSource> CArrayPPSources;


//A header whose output is being kept for the header cache (CPPHeaderCache), as it is preprocessed.  Its output (in
//the unit's) is in segments: runs of its own, and headers it included that are kept on their own.
struct SPPRecorder
{
	SPPCachedHeader*	psHeader;			//Being made.
	CNameSet			cSeen;				//Names looked up or defined: only a name's first look up depends on the includer.
	CArrayPPHeaderFiles	apcPragmaSeen;		//Likewise #pragma once files.
	size				uiStartToken;		//Where its output starts in the unit's.
	size				uiStartExpansion;
	size				uiStartInactive;
	size				uiStartDirective;
	size				uiSegmentToken;		//Where its current run of its own output starts in the unit's.
	size				uiSegmentExpansion;
	size				uiSegmentInactive;
	size				uiSegmentDirective;
	size				uiSegmentEvent;		//In psHeader's asEvents.
	int32				iTailEndLine;		//The end of the inactive region before the run (it mustn't grow), -1 for none.
	size				uiErrors;			//Errors and warnings before it (any made while it is read and it isn't kept).
	size				uiWarnings;
	bool				bValid;				//False if it can't be kept.
};


typedef CArrayTemplate<SPPRecorder> CArrayPPRecorders;


class CPreprocessor
{
protected:
	CConditionalStack				mcConditionalStack;
	CSourceFile*					mpcCurrentFile;
	CSourceFile*					mpcSkipFile;			//The file whose group is being skipped, NULL while parsing.
	int								miSkipStartLine;		//The first line after the directive that started skipping.
	CDefineMap						mcDefines;
	CPPTokenMemory					mcDefineReplacementMemory;
	CMemoryStackExtended			mcDefineStrings;
	CSpecialOperatorMap				mcSpecialOperators;
	CArrayHeaderFiles				mcHeaderNames;
	CMapPtrPtr						mcPragmaOnce;			//The headers with #pragma once read so far.
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
	CPPHeaderCache*					mpcHeaderCache;			//Headers' output kept for all units (SetHeaderCache), NULL for none.
	CArrayPPRecorders				masRecorders;			//The headers being kept, innermost last.
	uint64							muiIncludeDirectories;	//The include directories, hashed (0 until a header is included).
	int								miLookAhead;			//Reading past a token to see what follows (a macro's arguments).
	bool							mbReplaying;			//A kept header is being replayed.
	size							muiHeaderHits;
	size							muiHeaderMisses;
	size							muiReplayedTokens;

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
	void				SetHeaderCache(CPPHeaderCache* pcHeaderCache);

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
	void				NoteSkipping(CPPDirective* pcDirective, bool bWasParsing);
	void				AddInactiveLine(CSourceFile* pcFile, int iLine);
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
	bool 				ProcessHashDefine(CPPDirective* pcDirective, CPreprocessorTokenParser* pcParser);
	bool				ProcessHashDefineBracketted(CPreprocessorTokenParser* pcParser, CDefine* pcDefine);
	bool 				ProcessHashUndef(CPPDirective* pcDirective, CPreprocessorTokenParser* pcParser);
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
	void				MarkNextPositionForError(CPreprocessorTokenParser* pcParser, SPreprocessorPosition* psPos);
	CDefine*			LookUpDefine(const char* szName, size uiLength);

	//The header cache.  PreprocessorHeaderCache.cpp
	bool				CanUseHeaderCache(void);
	uint64				HeaderContext(void);
	uint64				DirectoriesHash(CSourceFile* pcFile, uint64 uiIncluding);
	bool				ReplayCachedHeader(CHeaderFile* pcFile, CHeaderFiles* pcHeaderNameMap);
	bool				CachedHeaderMatches(SPPCachedHeader* psHeader);
	void				ReplayHeader(SPPCachedHeader* psHeader);
	void				ReplayEvent(SPPHeaderEvent* psEvent);
	void				BeginRecording(CSourceFile* pcFile, CHeaderFiles* pcHeaderNameMap, uint64 uiContext);
	void				EndRecording(void);
	void				StartSegment(SPPRecorder* psRecorder);
	void				EndSegment(SPPRecorder* psRecorder, size uiToken, size uiExpansion, size uiInactive, size uiDirective);
	void				AddChildSegment(SPPRecorder* psRecorder, SPPCachedHeader* psChild, size uiToken, size uiExpansion, size uiInactive, size uiDirective);
	void				KillRecorder(SPPRecorder* psRecorder, bool bKeep);
	void				InvalidateRecordings(void);
	SPPRecorder*		Recording(void);
	void				NoteLookUp(const char* szName, size uiLength, CDefine* pcDefine);
	void				NoteDefined(const char* szName, size uiLength);
	void				NotePragmaQuery(CSourceFile* pcFile, bool bOnce);
	void				NoteEvent(EPPHeaderEvent eType, CSourceFile* pcFile, CPPDirective* pcDirective);
	static void			DefineIdentity(CDefine* pcDefine, SPPDefineIdentity* psIdentity);
};


#endif //__PREPROCESSOR_H__

