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
#include "Project.h"
#include "CPPUnitCompiler.h"


//////////////////////////////////////////////////////////////////////////
//
// psUnit compiled by pcSchedule, the first time (bFirst) or again.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitCompiler::Init(CCPPUnitSchedule* pcSchedule, SCPP65CompiledUnit* psUnit, bool bFirst)
{
	mpcSchedule = pcSchedule;
	mpcBuild = pcSchedule->mpcBuild;
	mpcProgram = pcSchedule->mpcProgram;
	mpcProject = pcSchedule->mpcProject;
	mpsUnit = psUnit;
	muiAttempts = 0;
	mbFirst = bFirst;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPP65CompiledUnit* CCPPUnitCompiler::GetUnit(void)
{
	return mpsUnit;
}


//////////////////////////////////////////////////////////////////////////
//
// Preprocesses and parses the unit's file, then generates it
// (StartGenerating).  Its errors and warnings go to the unit's (the
// preprocessor's only the first time).  On the W65C816 it may wait for
// its turn with the imports without holding the thread: the rest is done
// later (Continue, or Restart if it was stale).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitCompiler::Compile(void)
{
	char*	szConfiguration;
	bool	bResult;

	mpcWorkspace = mpcSchedule->TakeWorkspace();
	mpsUnit->pcFile->GetTokens()->SetMallocators(mpcWorkspace->GetPreprocessedTokensMallocators());

	szConfiguration = (char*)mpcBuild->GetTarget()->GetConfigurationName();
	mpsUnit->cStatistics.Begin(CPPCP_Preprocess);
	if (mbFirst)
	{
		bResult = mpcProject->ProcessUnit(mpsUnit->pcFile, szConfiguration, &mpsUnit->szErrors, &mpsUnit->szWarnings);
	}
	else
	{
		//Preprocessed again: its errors and warnings were reported the first time.
		mpcProject->ProcessUnit(mpsUnit->pcFile, szConfiguration);
		bResult = true;
	}
	mpsUnit->cStatistics.End(CPPCP_Preprocess);

	if (mpsUnit->pcSpillStore == &mpsUnit->cSpillStore)
	{
		mpsUnit->cSpillStore.Init();
	}

	mpsUnit->cStatistics.Begin(CPPCP_Tokenise);
	mcTokeniser.Init(mpcWorkspace->GetCPPTokensMallocator());
	bResult &= mcTokeniser.Tokenise(mpsUnit->pcFile->GetTokens());
	mpsUnit->cStatistics.End(CPPCP_Tokenise);
	mpsUnit->szErrors.Append(mcTokeniser.GetErrors()->Text());
	mcTree.Init(&mcTokeniser, mpcWorkspace->GetSyntaxTreeMallocators());
	mcParser.Init(&mcTokeniser, &mcTree);
	if (mpcBuild->HasLazyHeaders())
	{
		mcParser.SetHeaderIndex(&mpcSchedule->mcHeaderIndex);
	}
	mpsUnit->cStatistics.Begin(CPPCP_Parse);
	bResult &= mcParser.Parse();
	mpsUnit->cStatistics.End(CPPCP_Parse);
	mpsUnit->szErrors.Append(mcTree.GetErrors()->Text());
	mbResult = bResult;

	StartGenerating();
}


//////////////////////////////////////////////////////////////////////////
//
// A generator for the unit's syntax tree, into its assembly (which it
// initialises).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitCompiler::InitGenerator(void)
{
	mcAssembly.Init(mpsUnit->pcSpillStore, (uint32)mpsUnit->uiTurn);
	mcGenerator.Init(&mcTree, mpcBuild->GetTarget()->GetTarget());
	mcGenerator.SetStatistics(&mpsUnit->cStatistics);
	mcGenerator.SetOptions(mpcBuild->LibraryOptions(mpsUnit->pcFile->GetLibrary()));
	mcGenerator.SetSpillStore(mpsUnit->pcSpillStore, (uint32)mpsUnit->uiTurn);
	if (mpcBuild->GetTarget()->InlinesAcrossUnits())
	{
		mcGenerator.SetImports(&mpcSchedule->mcImports, mpsUnit->uiTurn);
		mcGenerator.SetFrameProfile(mpcBuild->GetFrameProfile());
	}
	mcGenerator.SetListing(mpcBuild->HasListingDirectory());
	mcGenerator.SetLayouts(mpcBuild->HasLayouts());
	mcGenerator.SetLineTable(mpcBuild->HasLineTable(), mpcBuild->GetCompileTime());
	mcGenerator.SetNoFloats(mpcBuild->HasNoFloats() && (mpcProgram->GetLinkLibrary(mpsUnit->pcFile->GetLibrary()) == NULL));
	mcGenerator.SetCompiledBefore(!mbFirst);
}


//////////////////////////////////////////////////////////////////////////
//
// Generates the unit up to its turn with the imports (CCPPUnit::
// GenerateBeforeTurn), then arrives at its turn: in it, generation goes on
// (Continue).  Before it the unit waits without a thread, and is
// resumed in its turn, or as soon as a function it looked for and didn't
// find is exported (it is generated again, up to CPP65_STALE_ATTEMPTS
// times).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitCompiler::StartGenerating(void)
{
	EIRArrival	eArrival;

	InitGenerator();
	mbGenerated = mcGenerator.GenerateBeforeTurn();
	if (!mpcBuild->GetTarget()->InlinesAcrossUnits())
	{
		Continue();
		return;
	}

	mpsUnit->eStage = CPP65US_Generating;
	eArrival = mpcSchedule->mcImports.Arrive(mpsUnit->uiTurn, mpsUnit, (muiAttempts < CPP65_STALE_ATTEMPTS) ? mcGenerator.GetNotImported() : NULL);
	if (eArrival == IRA_Turn)
	{
		Continue();
	}
	else if (eArrival == IRA_Stale)
	{
		Restart();
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The unit missed a function exported while it was generated: it is
// generated again (killing its generator takes its code out of the spill
// store).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitCompiler::Restart(void)
{
	mcGenerator.Kill();
	mcAssembly.Kill();
	muiAttempts++;
	StartGenerating();
}


//////////////////////////////////////////////////////////////////////////
//
// In the unit's turn (or with no turns): the rest of its generation
// (CCPPUnit::GenerateAfterTurn), then Finish.  If it missed
// a function exported since it inlined it is generated again, in its turn,
// which it keeps (then it finds everything it would have).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitCompiler::Continue(void)
{
	bool	bGenerated;

	bGenerated = mcGenerator.GenerateAfterTurn(&mcAssembly);
	if (mcGenerator.MissedImports())
	{
		mcGenerator.Kill();
		mcAssembly.Kill();
		InitGenerator();
		mbGenerated = mcGenerator.GenerateBeforeTurn();
		Continue();
		return;
	}
	Finish(bGenerated && mbGenerated);
}


//////////////////////////////////////////////////////////////////////////
//
// The unit is generated (bGenerated if that went well): its turn ends, its
// listing is written, it is assembled into its object, and what compiling
// it needed is freed.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitCompiler::Finish(bool bGenerated)
{
	bool	bResult;

	bResult = mbResult && bGenerated;
	mcGenerator.GetMissingCallees(&mpsUnit->aszMissing);
	mcGenerator.GetTestedCallees(&mpsUnit->aszTested);
	mcGenerator.GetLayouts(&mpsUnit->aszLayouts);
	mpsUnit->szErrors.Append(mcGenerator.GetErrors()->Text());
	mpsUnit->szWarnings.Append(mcGenerator.GetWarnings()->Text());
	mcGenerator.Kill();
	if (mpcBuild->GetTarget()->InlinesAcrossUnits())
	{
		//Its turn is over even if it didn't get to it (an error before the unit was finished).
		mpcSchedule->mcImports.EndTurn(mpsUnit->uiTurn);
	}
	if (mpcBuild->HasListingDirectory())
	{
		bResult &= mpcBuild->WriteListing(mpsUnit->pcFile->ShortName(), &mcAssembly, &mpsUnit->szErrors);
	}
	mpsUnit->uiLazySkipped += mcParser.NumLazySkipped();
	mpsUnit->uiLazyLoaded += mcParser.NumLazyLoaded();
	mpsUnit->uiLazyRead += mcParser.NumLazyRead();
	mcParser.Kill();

	//The syntax tree and tokens aren't needed to assemble.
	mcTree.Kill();
	mcTokeniser.Kill();

	mpsUnit->cStatistics.Begin(CPPCP_Assemble);
	bResult &= mpcProgram->Assemble(mpsUnit->pcObject, &mcAssembly, NULL, &mpsUnit->szErrors);
	mpsUnit->cStatistics.End(CPPCP_Assemble);
	mcAssembly.Kill();
	if (mpsUnit->pcSpillStore == &mpsUnit->cSpillStore)
	{
		mpsUnit->cSpillStore.Kill();
	}
	mpsUnit->bResult &= bResult;

	//The file's tokens and source are freed (the tokens' memory kept by the workspace for the next unit).
	mpsUnit->pcFile->GetTokens()->SetMallocators(NULL);
	mpsUnit->pcFile->Unload();
	mpcSchedule->GiveBackWorkspace(mpcWorkspace);
	mpcSchedule->ReportUnit(mpsUnit, mbFirst);
	//The last thing it does: the schedule frees it.
	mpcSchedule->Compiled(this);
}
