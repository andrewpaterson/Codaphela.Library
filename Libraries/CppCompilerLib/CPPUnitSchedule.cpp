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
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/FileUtil.h"
#include "Project.h"
#include "CPPUnitCompiler.h"
#include "CPPUnitSchedule.h"


//////////////////////////////////////////////////////////////////////////
//
// The project pcProject's units compiled for pcBuild, into pcProgram's
// objects; their errors and warnings added to pszErrors and pszWarnings,
// their counts and times to pcStatistics.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::Init(CCPPBuild* pcBuild, CCPPProgramLinker* pcProgram, CProject* pcProject, CChars* pszErrors, CChars* pszWarnings, CCPPCompileStatistics* pcStatistics)
{
	mpcBuild = pcBuild;
	mpcProgram = pcProgram;
	mpcProject = pcProject;
	mpszErrors = pszErrors;
	mpszWarnings = pszWarnings;
	mpcStatistics = pcStatistics;
	mcImports.Init(IR_IMPORT_INSTRUCTIONS);
	mcImports.SetResume(ResumeUnit, this);
	mpcUnitJobs = NULL;
	mcMemorySpillStore.Init();
	mpcSpillStore = &mcMemorySpillStore;
	mcHeaderIndex.Init();
	muiLazySkipped = 0;
	muiLazyLoaded = 0;
	muiLazyRead = 0;
	mpcJobs = NULL;
	mcCompilingMutex.Init();
	mcCompiledCondition.Init();
	muiCompiling = 0;
	muiUnitsFinished = 0;
	muiCompiledAgain = 0;
	mapcWorkspaces.Init();
	mbUnitReports = false;
	masUnitReports.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::Kill(void)
{
	if (mpcSpillStore == &mcIndexTreeSpillStore)
	{
		mcIndexTreeSpillStore.Kill();
	}
	KillWorkspaces();
	mapcWorkspaces.Kill();
	KillUnitReports(&masUnitReports);
	mcCompiledCondition.Kill();
	mcCompilingMutex.Kill();
	mcMemorySpillStore.Kill();
	mcHeaderIndex.Kill();
	mcImports.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The project's units are compiled by pcJobs: several at once by ThreadLib's
// CWorkerThreadJobs (the builder doesn't own them).  What is built is the
// same as without (though NumLazySkipped and NumLazyLoaded depend on how
// the units overlap).  NULL compiles them one at a time, as do jobs of one
// thread and a spill directory (SetSpillDirectory).  Set it before
// compiling.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::SetJobs(CJobs* pcJobs)
{
	mpcJobs = pcJobs;
}


//////////////////////////////////////////////////////////////////////////
//
// Each unit is reported as soon as it has finished compiling, with its
// errors and warnings (TakeUnitReports), so they can be shown while the
// others compile.  Its errors and warnings are then only in its report,
// not in GetErrors and GetWarnings, which keep what isn't a unit's (the
// link's, say).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::SetUnitReports(bool bUnitReports)
{
	mbUnitReports = bUnitReports;
}


//////////////////////////////////////////////////////////////////////////
//
// The units reported since the last time, in the order they finished,
// added to pasDest (KillUnitReports frees them).  Safe while Compile runs
// on another thread.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::TakeUnitReports(CArrayCPP65UnitReports* pasDest)
{
	size	i;

	mcCompilingMutex.Lock();
	for (i = 0; i < masUnitReports.NumElements(); i++)
	{
		pasDest->Add(masUnitReports.Get(i));
	}
	masUnitReports.ReInit();
	mcCompilingMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::KillUnitReports(CArrayCPP65UnitReports* pasReports)
{
	size				i;
	SCPP65UnitReport*	psReport;

	for (i = 0; i < pasReports->NumElements(); i++)
	{
		psReport = pasReports->Get(i);
		psReport->szWarnings.Kill();
		psReport->szErrors.Kill();
		psReport->szName.Kill();
	}
	pasReports->Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// psUnit has finished compiling: with SetUnitReports it is reported, its
// errors and warnings (only the first time it is compiled, as FinishUnit
// adds them) given to the report rather than kept for FinishUnit.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::ReportUnit(SCPP65CompiledUnit* psUnit, bool bFirst)
{
	SCPP65UnitReport	sReport;

	if (!mbUnitReports)
	{
		return;
	}
	sReport.szName.Init(psUnit->pcFile->ShortName());
	sReport.szErrors.Init(psUnit->szErrors);
	sReport.szWarnings.Init();
	if (bFirst)
	{
		sReport.szWarnings.Append(psUnit->szWarnings);
	}
	sReport.bAgain = !bFirst;
	psUnit->szErrors.ReInit();
	psUnit->szWarnings.ReInit();

	mcCompilingMutex.Lock();
	masUnitReports.Add(&sReport);
	mcCompilingMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnitSchedule::NumLazySkipped(void) const
{
	return muiLazySkipped;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnitSchedule::NumLazyLoaded(void) const
{
	return muiLazyLoaded;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the lazy declarations loaded were read as parsed before (in
// an earlier unit), not parsed again.
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnitSchedule::NumLazyRead(void) const
{
	return muiLazyRead;
}


//////////////////////////////////////////////////////////////////////////
//
// How many of the W65C816's units were compiled again because a function
// they call was exported after they were compiled (CompileUnits).
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnitSchedule::NumCompiledAgain(void) const
{
	return muiCompiledAgain;
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit, inlining before its turn, missed a
// function another unit exported meanwhile, and was generated again in its
// turn (CIRImports::Missed).
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnitSchedule::NumImportsMissed(void) const
{
	return mcImports.NumMissed();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit found before its turn that it had missed
// a function another unit exported while it inlined, and was generated
// again then (CIRImports::Arrive).
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnitSchedule::NumImportsStale(void) const
{
	return mcImports.NumStale();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit waited for its turn without a thread (CIRImports::
// Arrive).
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnitSchedule::NumImportsParked(void) const
{
	return mcImports.NumParked();
}


//////////////////////////////////////////////////////////////////////////
//
// For testing: every unit is generated twice, as if it missed an
// imported function the first time.  The code is the same.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::SetForceImportsMissed(bool bForce)
{
	mcImports.SetForceMisses(bForce);
}


//////////////////////////////////////////////////////////////////////////
//
// Each unit's functions' code waits to be assembled in an evicting index
// tree in szDirectory (made, and removed by Kill), with at most uiCacheSize
// bytes of it in memory, instead of all in memory.  The code is the same.
// NULL for memory.  Set it before compiling.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnitSchedule::SetSpillDirectory(const char* szDirectory, lsize uiCacheSize)
{
	if (mpcSpillStore == &mcIndexTreeSpillStore)
	{
		mcIndexTreeSpillStore.Kill();
	}
	mpcSpillStore = &mcMemorySpillStore;
	if (StrEmpty(szDirectory))
	{
		return true;
	}
	if (!mcIndexTreeSpillStore.Init(szDirectory, uiCacheSize))
	{
		mpszErrors->Append("Can't make the spill directory ");
		mpszErrors->Append(szDirectory);
		mpszErrors->AppendNewLine();
		return false;
	}
	mpcSpillStore = &mcIndexTreeSpillStore;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles the project's units, each into an object (of its library when it
// is linked as one).  Each unit is preprocessed just before it is compiled,
// and its tokens and source freed after, so only the units being compiled
// are kept (headers stay tokenised for the units after).
//
// The units are preprocessed and compiled by the jobs (SetJobs), several at
// once.  What they share is used as if they were compiled one after
// another: a header is tokenised by the first to include it (CCPPPreprocessor::
// TokeniseFile); their objects are added in order, here; the W65C816's
// small functions are found and exported in each unit's turn (mcImports);
// the header declarations they parse can't change what any of them is
// compiled to (mcHeaderIndex); and their errors and warnings are added in
// order at the end.  A unit that called one of those functions
// before it was exported is compiled again after them all, in another turn.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnitSchedule::CompileUnits(size uiUnits)
{
	CTranslationUnit*			pcFile;
	SProcessIter				sIter;
	CArrayCPP65CompiledUnits	apsUnits;
	SCPP65CompiledUnit*			psUnit;
	CSerialJobs					cSerialJobs;
	CJobs*						pcJobs;
	size						uiMaxCompiling;
	size						uiTurn;
	size						ui;
	bool						bResult;
	CFileUtil					cFileUtil;

	if (uiUnits == 0)
	{
		return true;
	}

	mpcProject->CreatePost();
	if (mpcBuild->HasListingDirectory())
	{
		cFileUtil.TouchDir(mpcBuild->GetListingDirectory());
	}
	pcJobs = UnitJobs(&cSerialJobs);
	mpcUnitJobs = pcJobs;
	uiMaxCompiling = (size)pcJobs->NumThreads() * 2;
	mcImports.ResetTurns();
	bResult = true;
	uiTurn = 0;
	apsUnits.Init();
	for (pcFile = mpcProject->Start(&sIter, (char*)mpcBuild->GetTarget()->GetConfigurationName()); pcFile; pcFile = mpcProject->Iterate(&sIter))
	{
		psUnit = NewMalloc<SCPP65CompiledUnit>();
		StartUnit(psUnit, pcFile, uiTurn, mpcSpillStore != &mcIndexTreeSpillStore);
		uiTurn++;
		apsUnits.Add(psUnit);
		psUnit->pcObject = mpcProgram->AddUnitObject(psUnit->pcFile);
		RunUnit(pcJobs, CompileUnitJob, psUnit, uiMaxCompiling);
	}
	pcJobs->Wait();
	for (ui = 0; ui < apsUnits.NumElements(); ui++)
	{
		psUnit = *apsUnits.Get(ui);
		FinishUnit(psUnit, true);
		bResult &= psUnit->bResult;
	}

	//The units that call functions exported after they were compiled, compiled again in order.  A unit decides in its
	//turn if it calls only functions not yet exported (a unit before it may export one).
	if (bResult && mpcBuild->GetTarget()->InlinesAcrossUnits())
	{
		for (ui = 0; ui < apsUnits.NumElements(); ui++)
		{
			psUnit = *apsUnits.Get(ui);
			if (psUnit->aszMissing.NumElements() == 0)
			{
				psUnit->eAgain = CPP65CA_No;
			}
			else
			{
				psUnit->eAgain = CalleeExported(psUnit) ? CPP65CA_Yes : CPP65CA_IfExported;
			}
		}
		for (ui = 0; ui < apsUnits.NumElements(); ui++)
		{
			psUnit = *apsUnits.Get(ui);
			if (psUnit->eAgain != CPP65CA_No)
			{
				psUnit->uiTurn = uiTurn;
				uiTurn++;
				psUnit->bResult = true;
				RunUnit(pcJobs, CompileUnitAgainJob, psUnit, uiMaxCompiling);
			}
		}
		pcJobs->Wait();
		for (ui = 0; ui < apsUnits.NumElements(); ui++)
		{
			psUnit = *apsUnits.Get(ui);
			if (psUnit->eAgain == CPP65CA_Yes)
			{
				muiCompiledAgain++;
				FinishUnit(psUnit, false);
				bResult &= psUnit->bResult;
			}
		}
	}

	for (ui = 0; ui < apsUnits.NumElements(); ui++)
	{
		psUnit = *apsUnits.Get(ui);

		//What it tests, and that its object is C++'s, for CheckTested.
		mpcProgram->AddCompiled(psUnit->pcObject, &psUnit->aszTested);
		mpcBuild->AddLayouts(&psUnit->aszLayouts);
		psUnit->aszLayouts.Kill();
		psUnit->aszTested.Kill();
		psUnit->aszMissing.Kill();
		psUnit->szWarnings.Kill();
		psUnit->szErrors.Kill();
		free(psUnit);
	}
	KillWorkspaces();
	mpcUnitJobs = NULL;
	bResult &= mpcBuild->WriteLayouts(mpszErrors);

	if (apsUnits.NumElements() != uiUnits)
	{
		mpszErrors->Append("Only ")->Append((int)apsUnits.NumElements())->Append(" of the ")->Append((int)uiUnits)->Append(" translation units were compiled.\n");
		bResult = false;
	}
	apsUnits.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What compiles the units: mpcJobs, or pcSerialJobs to compile them one at a
// time (without SetJobs, with an index tree to spill to, or while the IR
// front end is being debugged).
//
//////////////////////////////////////////////////////////////////////////
CJobs* CCPPUnitSchedule::UnitJobs(CSerialJobs* pcSerialJobs)
{
	if (!mpcJobs || (mpcJobs->NumThreads() <= 1))
	{
		return pcSerialJobs;
	}
	if ((mpcSpillStore == &mcIndexTreeSpillStore) || mpcBuild->GetOptions()->mbPrintIR || (mpcBuild->GetOptions()->muiFunctionLimit != CPP_NO_FUNCTION_LIMIT))
	{
		return pcSerialJobs;
	}

	//Built before the units, which only look them up.
	mpcBuild->GetTarget()->PrepareForThreads();
	return mpcJobs;
}


//////////////////////////////////////////////////////////////////////////
//
// psUnit compiles pcFile in its turn uiTurn, with its own spill store or
// the builder's index tree.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::StartUnit(SCPP65CompiledUnit* psUnit, CTranslationUnit* pcFile, size uiTurn, bool bOwnSpillStore)
{
	psUnit->pcSchedule = this;
	psUnit->pcFile = pcFile;
	psUnit->pcObject = NULL;
	psUnit->aszMissing.Init();
	psUnit->aszTested.Init();
	psUnit->aszLayouts.Init();
	psUnit->eAgain = CPP65CA_No;
	psUnit->uiTurn = uiTurn;
	psUnit->bResult = true;
	psUnit->szErrors.Init();
	psUnit->szWarnings.Init();
	psUnit->pcSpillStore = bOwnSpillStore ? &psUnit->cSpillStore : mpcSpillStore;
	psUnit->uiLazySkipped = 0;
	psUnit->uiLazyLoaded = 0;
	psUnit->uiLazyRead = 0;
	psUnit->cStatistics.Init();
	psUnit->cStatistics.SetTimes(mpcStatistics->IsTimed());
	psUnit->pcCompiler = NULL;
	psUnit->eStage = CPP65US_Generating;
}


//////////////////////////////////////////////////////////////////////////
//
// Gives psUnit to the jobs to compile (fJob), once fewer than
// uiMaxCompiling units are compiling (each keeps its file's tokens).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::RunUnit(CJobs* pcJobs, JobFunction fJob, SCPP65CompiledUnit* psUnit, size uiMaxCompiling)
{
	mcCompilingMutex.Lock();
	while (muiCompiling >= uiMaxCompiling)
	{
		mcCompiledCondition.Wait(&mcCompilingMutex);
	}
	muiCompiling++;
	mcCompilingMutex.Unlock();

	pcJobs->Run(fJob, psUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function psUnit calls, that wasn't found to inline, has been
// exported since.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnitSchedule::CalleeExported(SCPP65CompiledUnit* psUnit)
{
	size	ui;

	for (ui = 0; ui < psUnit->aszMissing.NumElements(); ui++)
	{
		if (mcImports.Find(psUnit->aszMissing.GetText(ui)) != NULL)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Adds what compiling the unit gave to the builder's: its errors, and its
// warnings if bWarnings (the first time it is compiled).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::FinishUnit(SCPP65CompiledUnit* psUnit, bool bWarnings)
{
	mpszErrors->Append(psUnit->szErrors);
	if (bWarnings)
	{
		mpszWarnings->Append(psUnit->szWarnings);
	}
	psUnit->szErrors.ReInit();
	psUnit->szWarnings.ReInit();
	muiLazySkipped += psUnit->uiLazySkipped;
	muiLazyLoaded += psUnit->uiLazyLoaded;
	muiLazyRead += psUnit->uiLazyRead;
	psUnit->uiLazySkipped = 0;
	psUnit->uiLazyLoaded = 0;
	psUnit->uiLazyRead = 0;
	mpcStatistics->Add(&psUnit->cStatistics);
	psUnit->cStatistics.Clear();
}


//////////////////////////////////////////////////////////////////////////
//
// A job: compiles the unit (CompileUnit).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::CompileUnitJob(void* pvUnit)
{
	SCPP65CompiledUnit*	psUnit;

	psUnit = (SCPP65CompiledUnit*)pvUnit;
	psUnit->pcSchedule->CompileUnit(psUnit, true);
}


//////////////////////////////////////////////////////////////////////////
//
// A job: compiles the unit again (CompileUnitAgain).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::CompileUnitAgainJob(void* pvUnit)
{
	SCPP65CompiledUnit*	psUnit;

	psUnit = (SCPP65CompiledUnit*)pvUnit;
	psUnit->pcSchedule->CompileUnitAgain(psUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// Compiles psUnit into its object again, if it is to be (eAgain).  If it
// calls only functions that weren't exported after the first pass, it
// decides in its turn, when the units before it have exported theirs
// (DecideAgain; till then it waits without a thread, CIRImports::Arrive).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::CompileUnitAgain(SCPP65CompiledUnit* psUnit)
{
	if (psUnit->eAgain == CPP65CA_IfExported)
	{
		psUnit->eStage = CPP65US_Deciding;
		if (mcImports.Arrive(psUnit->uiTurn, psUnit, NULL) == IRA_Turn)
		{
			DecideAgain(psUnit);
		}
		return;
	}
	CompileUnit(psUnit, false);
}


//////////////////////////////////////////////////////////////////////////
//
// In psUnit's turn: it is compiled again (in that turn, which it keeps) if
// a unit before it has exported a function it calls, otherwise its object
// is as it was (eAgain is No).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::DecideAgain(SCPP65CompiledUnit* psUnit)
{
	if (!CalleeExported(psUnit))
	{
		mcImports.EndTurn(psUnit->uiTurn);
		psUnit->eAgain = CPP65CA_No;
		UnitFinished();
		return;
	}
	psUnit->eAgain = CPP65CA_Yes;
	CompileUnit(psUnit, false);
}


//////////////////////////////////////////////////////////////////////////
//
// A workspace for compiling a unit (CCPP65UnitWorkspace): one another
// unit has finished with, or a new one.  Units compiled at the same time
// have their own.
//
//////////////////////////////////////////////////////////////////////////
CCPP65UnitWorkspace* CCPPUnitSchedule::TakeWorkspace(void)
{
	CCPP65UnitWorkspace*	pcWorkspace;

	mcCompilingMutex.Lock();
	pcWorkspace = mapcWorkspaces.IsNotEmpty() ? mapcWorkspaces.Pop() : NULL;
	mcCompilingMutex.Unlock();

	if (!pcWorkspace)
	{
		pcWorkspace = NewMalloc<CCPP65UnitWorkspace>();
		pcWorkspace->Init();
	}
	pcWorkspace->Begin();
	return pcWorkspace;
}


//////////////////////////////////////////////////////////////////////////
//
// The unit compiled with pcWorkspace has finished with it.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::GiveBackWorkspace(CCPP65UnitWorkspace* pcWorkspace)
{
	mcCompilingMutex.Lock();
	mapcWorkspaces.Push(pcWorkspace);
	mcCompilingMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// The workspaces' memory is freed (no unit is compiling).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::KillWorkspaces(void)
{
	size					i;
	CCPP65UnitWorkspace*	pcWorkspace;

	for (i = 0; i < mapcWorkspaces.NumElements(); i++)
	{
		pcWorkspace = *mapcWorkspaces.Get(i);
		pcWorkspace->Kill();
		free(pcWorkspace);
	}
	mapcWorkspaces.ReInit();
}


//////////////////////////////////////////////////////////////////////////
//
// A unit given to the jobs has finished (muiCompiling).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::UnitFinished(void)
{
	mcCompilingMutex.Lock();
	muiCompiling--;
	muiUnitsFinished++;
	mcCompiledCondition.WakeAll();
	mcCompilingMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit has finished compiling, for showing progress from
// another thread while Compile runs.  A unit compiled again counts
// twice, so it can pass NumUnitsToCompile.
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnitSchedule::NumUnitsFinished(void)
{
	size	uiFinished;

	mcCompilingMutex.Lock();
	uiFinished = muiUnitsFinished;
	mcCompilingMutex.Unlock();
	return uiFinished;
}


//////////////////////////////////////////////////////////////////////////
//
// CIRImports' resume function: a unit that waited for its turn is given to
// the jobs, to go on in its turn or (bStale) to be generated again.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::ResumeUnit(void* pvSchedule, void* pvUnit, bool bStale)
{
	CCPPUnitSchedule*	pcSchedule;

	pcSchedule = (CCPPUnitSchedule*)pvSchedule;
	pcSchedule->mpcUnitJobs->Run(bStale ? RestartUnitJob : ResumeUnitJob, pvUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// A job: the unit goes on in its turn (Resume).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::ResumeUnitJob(void* pvUnit)
{
	SCPP65CompiledUnit*	psUnit;

	psUnit = (SCPP65CompiledUnit*)pvUnit;
	psUnit->pcSchedule->Resume(psUnit);
}


//////////////////////////////////////////////////////////////////////////
//
// A job: the unit is generated again before its turn (CCPPUnitCompiler::
// Restart).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::RestartUnitJob(void* pvUnit)
{
	SCPP65CompiledUnit*	psUnit;

	psUnit = (SCPP65CompiledUnit*)pvUnit;
	psUnit->pcCompiler->Restart();
}


//////////////////////////////////////////////////////////////////////////
//
// In psUnit's turn, after it waited for it: it goes on where it was
// (eStage): deciding whether it is compiled again, or generating.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::Resume(SCPP65CompiledUnit* psUnit)
{
	if (psUnit->eStage == CPP65US_Deciding)
	{
		DecideAgain(psUnit);
	}
	else
	{
		psUnit->pcCompiler->Continue();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Preprocesses and compiles psUnit's file into its object (a
// CCPPUnitCompiler), then frees the file's tokens and source.  Its errors and warnings go to the unit's (the
// preprocessor's only if bReport; compiled again, its object is made
// again), the functions it calls that weren't found for inlining to its
// aszMissing.  The unit is counted as compiling (muiCompiling) until it has
// finished.  On the W65C816 it may wait for its turn with the imports
// without holding the thread (StartGenerating): the rest is done later.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::CompileUnit(SCPP65CompiledUnit* psUnit, bool bReport)
{
	CCPPUnitCompiler*	pcCompiler;

	if (!bReport)
	{
		psUnit->aszMissing.ReInit();
		psUnit->pcObject->Kill();
		psUnit->pcObject->Init(psUnit->pcFile->ShortName());
	}

	pcCompiler = NewMalloc<CCPPUnitCompiler>();
	psUnit->pcCompiler = pcCompiler;
	pcCompiler->Init(this, psUnit, bReport);
	pcCompiler->Compile();
}


//////////////////////////////////////////////////////////////////////////
//
// pcCompiler has finished compiling its unit: it is freed, and the unit
// counted as finished (UnitFinished).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitSchedule::Compiled(CCPPUnitCompiler* pcCompiler)
{
	SCPP65CompiledUnit*	psUnit;

	psUnit = pcCompiler->GetUnit();
	free(pcCompiler);
	psUnit->pcCompiler = NULL;
	UnitFinished();
}
