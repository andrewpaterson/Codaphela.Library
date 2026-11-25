/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#ifndef __C_P_P_UNIT_SCHEDULE_H__
#define __C_P_P_UNIT_SCHEDULE_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayChars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/Jobs.h"
#include "BaseLib/Mutex.h"
#include "BaseLib/ConditionVariable.h"
#include "Assembler65Lib/Asm65Object.h"
#include "IRImports.h"
#include "CPPHeaderIndex.h"
#include "CPP65UnitWorkspace.h"
#include "CPPCompileStatistics.h"
#include "CPPBuild.h"
#include "CPPProgramLinker.h"


class CProject;
class CTranslationUnit;


//How many times a unit is generated again before its turn, having missed a function exported while it inlined
//(after that it only waits for its turn).
#define CPP65_STALE_ATTEMPTS	3


class CCPPUnitSchedule;
class CCPPUnitCompiler;


//Whether a unit is compiled again (after the first pass, on the W65C816).
enum ECPP65CompileAgain
{
	CPP65CA_No,
	CPP65CA_Yes,			//A function it calls has been exported.
	CPP65CA_IfExported,		//If a unit compiled again before it exports a function it calls.
};


//Where a unit waiting for its turn without a thread (CIRImports::Arrive) is: generated up to its turn (it goes on in
//it, CCPPUnitCompiler::Continue), or deciding whether it is compiled again (CCPPUnitSchedule::CompileUnitAgain).
enum ECPP65UnitStage
{
	CPP65US_Generating,
	CPP65US_Deciding,
};


//A translation unit compiled by CompileUnits: its object, and the functions it calls that no unit had exported
//when it was compiled (it is compiled again if one is exported later).  What compiling it gives (its errors,
//warnings and counts) is kept with it until it is added to the builder's, in the units' order.
struct SCPP65CompiledUnit
{
	CCPPUnitSchedule*	pcSchedule;
	CTranslationUnit*	pcFile;
	CAsm65Object*		pcObject;
	CArrayChars			aszMissing;
	CArrayChars			aszTested;		//The functions it calls by name and tests for an exception (CCPPUnit::GetTestedCallees).
	CArrayChars			aszLayouts;		//The classes it laid out (CCPPUnit::GetLayouts), with SetLayoutsFile.
	ECPP65CompileAgain	eAgain;
	size				uiTurn;			//Its turn with the imports (CIRImports::BeginTurn).
	bool				bResult;
	CChars				szErrors;
	CChars				szWarnings;
	CSpillStore*		pcSpillStore;	//The builder's index tree (SetSpillDirectory), or cSpillStore.
	CMemorySpillStore	cSpillStore;
	size				uiLazySkipped;
	size				uiLazyLoaded;
	size				uiLazyRead;
	CCPPCompileStatistics	cStatistics;	//What compiling it was counted and timed in, until it is added to the builder's (FinishUnit).
	CCPPUnitCompiler*	pcCompiler;		//While it is being compiled.
	ECPP65UnitStage		eStage;			//While it waits for its turn.
};


typedef CArrayTemplatePtr<SCPP65CompiledUnit>	CArrayCPP65CompiledUnits;


//A unit that has finished compiling, with the errors and warnings it had, as soon as it has (SetUnitReports).
struct SCPP65UnitReport
{
	CChars	szName;		//The unit's file name, without its directory.
	CChars	szErrors;
	CChars	szWarnings;
	bool	bAgain;		//A unit compiled a second time; it was reported the first time too.
};


typedef CArrayTemplate<SCPP65UnitReport>	CArrayCPP65UnitReports;


//Compiles a project's units (CCPP65Builder has one): each into its object (pcProgram's, CCPPProgramLinker::
//AddUnitObject), preprocessed just before it is compiled and its tokens and source freed after, several at once on
//the jobs (SetJobs).  What they share is used as if they were compiled one after another: a header is tokenised by the
//first to include it; their objects are added in order; the W65C816's small functions are found and exported in each
//unit's turn (the imports, CIRImports), a unit waiting for its turn without a thread; the header declarations they
//parse can't change what any of them is compiled to (the header index); and their errors and warnings are added in
//order at the end.  A unit that called one of those functions before it was exported is compiled again after them
//all, in another turn.  Each unit is compiled by a CCPPUnitCompiler, with a workspace from those the units share.
class CCPPUnitSchedule
{
friend class CCPPUnitCompiler;
protected:
	CCPPBuild*						mpcBuild;				//What the units are built for and how.
	CCPPProgramLinker*				mpcProgram;				//Where their objects go.
	CProject*						mpcProject;				//The units.
	CChars*							mpszErrors;				//Their errors and warnings, added in order...
	CChars*							mpszWarnings;
	CCPPCompileStatistics*			mpcStatistics;			//...and their counts and times.
	CIRImports						mcImports;				//The units' small functions, for inlining in the others.
	CSpillStore*					mpcSpillStore;			//Where the units' functions' code waits to be assembled (SetSpillDirectory).
	CMemorySpillStore				mcMemorySpillStore;
	CIndexTreeSpillStore			mcIndexTreeSpillStore;
	CCPPHeaderIndex					mcHeaderIndex;			//The header declarations the units have parsed.
	size							muiLazySkipped;			//The units' header declarations left unparsed (CCPPBuild::SetLazyHeaders)...
	size							muiLazyLoaded;			//...and loaded when used...
	size							muiLazyRead;			//...of them read as parsed before, not parsed again.
	CJobs*							mpcJobs;				//What compiles the units (SetJobs), or NULL for one at a time.
	CJobs*							mpcUnitJobs;			//What CompileUnits is compiling the units with (units waiting for their turns are resumed on it).
	CMutex							mcCompilingMutex;
	CConditionVariable				mcCompiledCondition;
	size							muiCompiling;			//Units given to mpcJobs that haven't finished.
	size							muiUnitsFinished;		//Units that have finished compiling (NumUnitsFinished).
	CArrayTemplatePtr<CCPP65UnitWorkspace>	mapcWorkspaces;	//Those no unit is being compiled with (TakeWorkspace).
	size							muiCompiledAgain;		//Units compiled a second time.
	bool							mbUnitReports;			//Each unit is reported as it finishes (SetUnitReports).
	CArrayCPP65UnitReports			masUnitReports;			//Those not taken yet (TakeUnitReports), under mcCompilingMutex.

public:
	void					Init(CCPPBuild* pcBuild, CCPPProgramLinker* pcProgram, CProject* pcProject, CChars* pszErrors, CChars* pszWarnings, CCPPCompileStatistics* pcStatistics);
	void					Kill(void);

	void					SetJobs(CJobs* pcJobs);
	void					SetUnitReports(bool bUnitReports);
	void					TakeUnitReports(CArrayCPP65UnitReports* pasDest);
	static void				KillUnitReports(CArrayCPP65UnitReports* pasReports);
	bool					SetSpillDirectory(const char* szDirectory, lsize uiCacheSize);
	void					SetForceImportsMissed(bool bForce);
	size					NumLazySkipped(void) const;
	size					NumLazyLoaded(void) const;
	size					NumLazyRead(void) const;
	size					NumCompiledAgain(void) const;
	size					NumImportsMissed(void) const;
	size					NumImportsStale(void) const;
	size					NumImportsParked(void) const;
	size					NumUnitsFinished(void);

	bool					CompileUnits(size uiUnits);

protected:
	CJobs*					UnitJobs(CSerialJobs* pcSerialJobs);
	void					StartUnit(SCPP65CompiledUnit* psUnit, CTranslationUnit* pcFile, size uiTurn, bool bOwnSpillStore);
	void					RunUnit(CJobs* pcJobs, JobFunction fJob, SCPP65CompiledUnit* psUnit, size uiMaxCompiling);
	void					FinishUnit(SCPP65CompiledUnit* psUnit, bool bWarnings);
	bool					CalleeExported(SCPP65CompiledUnit* psUnit);
	void					CompileUnit(SCPP65CompiledUnit* psUnit, bool bReport);
	void					CompileUnitAgain(SCPP65CompiledUnit* psUnit);
	void					DecideAgain(SCPP65CompiledUnit* psUnit);
	void					Compiled(CCPPUnitCompiler* pcCompiler);
	void					Resume(SCPP65CompiledUnit* psUnit);
	static void				ResumeUnit(void* pvSchedule, void* pvUnit, bool bStale);
	static void				ResumeUnitJob(void* pvUnit);
	static void				RestartUnitJob(void* pvUnit);
	void					UnitFinished(void);
	void					ReportUnit(SCPP65CompiledUnit* psUnit, bool bFirst);
	CCPP65UnitWorkspace*	TakeWorkspace(void);
	void					GiveBackWorkspace(CCPP65UnitWorkspace* pcWorkspace);
	void					KillWorkspaces(void);
	static void				CompileUnitJob(void* pvUnit);
	static void				CompileUnitAgainJob(void* pvUnit);
};


#endif // __C_P_P_UNIT_SCHEDULE_H__
