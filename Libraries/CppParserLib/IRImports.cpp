/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "IRPrinter.h"
#include "IRParser.h"
#include "IRInliner.h"
#include "IRImports.h"


//////////////////////////////////////////////////////////////////////////
//
// Functions of up to uiMaxInstructions instructions are kept (the most the
// inliner takes, CIRInliner's loop limit).
//
//////////////////////////////////////////////////////////////////////////
void CIRImports::Init(size uiMaxInstructions)
{
	mcModule.Init();
	mcModule.AddSpace("default", 4);
	muiMaxInstructions = uiMaxInstructions;
	mcMutex.Init();
	mcTurnChanged.Init();
	muiTurn = 0;
	mcModuleLock.Init();
	muiMissed = 0;
	muiStale = 0;
	mbForceMisses = false;
	muiForcedTurn = SIZE_MAX;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRImports::Kill(void)
{
	mcModuleLock.Kill();
	mcTurnChanged.Kill();
	mcMutex.Kill();
	mcModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The first unit compiled next has turn 0.
//
//////////////////////////////////////////////////////////////////////////
void CIRImports::ResetTurns(void)
{
	mcMutex.Lock();
	muiTurn = 0;
	muiForcedTurn = SIZE_MAX;
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// Waits for turn uiTurn: the units before it have ended theirs.  Compiled one after another, it is uiTurn's turn
// already.
//
//////////////////////////////////////////////////////////////////////////
void CIRImports::BeginTurn(size uiTurn)
{
	mcMutex.Lock();
	while (muiTurn < uiTurn)
	{
		mcTurnChanged.Wait(&mcMutex);
	}
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// Ends turn uiTurn (waiting for it first): the next unit's begins.  A unit ends its turn when it has exported its
// functions, and again when it has finished (it may have stopped before it took its turn); the second time does
// nothing.
//
//////////////////////////////////////////////////////////////////////////
void CIRImports::EndTurn(size uiTurn)
{
	mcMutex.Lock();
	while (muiTurn < uiTurn)
	{
		mcTurnChanged.Wait(&mcMutex);
	}
	if (muiTurn == uiTurn)
	{
		muiTurn++;
		mcTurnChanged.WakeAll();
	}
	mcMutex.Unlock();
}


//////////////////////////////////////////////////////////////////////////
//
// Functions are found (and read) before the unit's turn, while the unit
// whose turn it is may export: between BeginRead and EndRead they don't
// change.  In its turn a unit needn't (only it exports).
//
//////////////////////////////////////////////////////////////////////////
void CIRImports::BeginRead(void)
{
	mcModuleLock.LockShared();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRImports::EndRead(void)
{
	mcModuleLock.UnlockShared();
}


//////////////////////////////////////////////////////////////////////////
//
// In a unit's turn: whether a function named in paszNotFound, which the
// unit looked for before its turn and didn't find, has been exported since
// (the unit must do its work again, in its turn).  With SetForceMisses each
// turn misses the first time it asks.
//
//////////////////////////////////////////////////////////////////////////
bool CIRImports::Missed(CArrayChars* paszNotFound)
{
	size	ui;
	bool	bMissed;

	bMissed = false;
	if (mbForceMisses && (muiForcedTurn != muiTurn))
	{
		muiForcedTurn = muiTurn;
		bMissed = true;
	}
	for (ui = 0; (ui < paszNotFound->NumElements()) && !bMissed; ui++)
	{
		bMissed = Find(paszNotFound->GetText(ui)) != NULL;
	}
	if (bMissed)
	{
		muiMissed++;
	}
	return bMissed;
}


//////////////////////////////////////////////////////////////////////////
//
// Waits for turn uiTurn (as BeginTurn), unless a function named in
// paszNotFound, which the unit looked for and didn't find, is exported
// before then: true if one was (the turn hasn't begun), false when the
// turn has (Missed tells it then).  Each turn that ends before is looked
// at.
//
//////////////////////////////////////////////////////////////////////////
bool CIRImports::BeginTurnUnlessStale(CArrayChars* paszNotFound, size uiTurn)
{
	size	ui;
	bool	bStale;
	size	uiSeen;

	mcMutex.Lock();
	for (;;)
	{
		if (muiTurn >= uiTurn)
		{
			mcMutex.Unlock();
			return false;
		}
		uiSeen = muiTurn;
		mcMutex.Unlock();

		bStale = false;
		BeginRead();
		for (ui = 0; (ui < paszNotFound->NumElements()) && !bStale; ui++)
		{
			bStale = Find(paszNotFound->GetText(ui)) != NULL;
		}
		EndRead();

		mcMutex.Lock();
		if (bStale)
		{
			muiStale++;
			mcMutex.Unlock();
			return true;
		}
		while (muiTurn == uiSeen)
		{
			mcTurnChanged.Wait(&mcMutex);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many units found they had missed a function before their turn
// (BeginTurnUnlessStale).
//
//////////////////////////////////////////////////////////////////////////
size CIRImports::NumStale(void)
{
	return muiStale;
}


//////////////////////////////////////////////////////////////////////////
//
// How many times a unit's turn found it had missed a function (Missed).
//
//////////////////////////////////////////////////////////////////////////
size CIRImports::NumMissed(void)
{
	return muiMissed;
}


//////////////////////////////////////////////////////////////////////////
//
// For testing: every unit's turn takes it as having missed a function once
// (the unit does its work again in its turn).
//
//////////////////////////////////////////////////////////////////////////
void CIRImports::SetForceMisses(bool bForceMisses)
{
	mbForceMisses = bForceMisses;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether sValue is a symbol local to pcModule's unit (a static global or
// function, a static local, a string's label): another unit can't name it.
// The front end marks those it doesn't define as IR (CIRModule::SetLocal).
//
//////////////////////////////////////////////////////////////////////////
bool CIRImports::IsLocalSymbol(CIRModule* pcModule, SIRReference sValue)
{
	CIRGlobal*		pcGlobal;
	CIRFunction*	pcFunction;

	if (!sValue.IsSymbol())
	{
		return false;
	}
	if (pcModule->IsLocal(sValue.Index()))
	{
		return true;
	}
	pcGlobal = pcModule->FindGlobal(sValue.Index());
	pcFunction = pcModule->FindFunction(sValue.Index());
	return (pcGlobal && (pcGlobal->muiAttributes & IRA_Local)) || (pcFunction && (pcFunction->muiAttributes & IRA_Local));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction's body can be copied to another unit: it has one, is
// small, has external linkage, calls no named function and names no symbol
// local to pcModule's unit.
//
//////////////////////////////////////////////////////////////////////////
bool CIRImports::IsExportable(CIRModule* pcModule, CIRFunction* pcFunction)
{
	CIRBlock*		pcBlock;
	SIRInstruction*	psInstruction;
	SIRPhi*			psPhi;
	size			ui;
	size			uiIndex;
	size			uiArgument;

	if ((pcFunction->NumBlocks() == 0) || (pcFunction->muiAttributes & (IRA_Local | IRA_Weak | IRA_Variadic)) ||
		(CIRInliner::NumInstructions(pcFunction) > muiMaxInstructions))
	{
		return false;
	}
	for (ui = 0; ui < pcFunction->NumBlocks(); ui++)
	{
		pcBlock = pcFunction->GetBlock(ui);
		for (uiIndex = 0; uiIndex < pcBlock->masInstructions.NumElements(); uiIndex++)
		{
			psInstruction = pcBlock->masInstructions.Get(uiIndex);
			if (((psInstruction->uiOpcode == IRO_Call) && psInstruction->asArguments[0].IsSymbol()) ||
				IsLocalSymbol(pcModule, psInstruction->asArguments[0]) || IsLocalSymbol(pcModule, psInstruction->asArguments[1]))
			{
				return false;
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				if (IsLocalSymbol(pcModule, psPhi->asArguments.Get(uiArgument)->sValue))
				{
					return false;
				}
			}
		}
		if (IsLocalSymbol(pcModule, pcBlock->msJump.sArgument))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Copies pcFunction (of pcModule) in, through its text, if it can be
// inlined in other units and isn't in already.  True if it was copied.
// Only in the unit's turn (units reading before theirs wait for it).
//
//////////////////////////////////////////////////////////////////////////
bool CIRImports::Export(CIRModule* pcModule, CIRFunction* pcFunction)
{
	CChars		szText;
	CIRPrinter	cPrinter;
	CIRParser	cParser;
	CCPPErrors	cErrors;
	bool		bResult;
	size		ui;
	int			iSymbol;

	if (!IsExportable(pcModule, pcFunction) || Find(pcModule->GetSymbolName(pcFunction->muiSymbol)))
	{
		return false;
	}
	szText.Init();
	cPrinter.Init(pcModule, &szText);
	cPrinter.PrintFunction(pcFunction);
	cPrinter.Kill();
	mcModuleLock.Lock();
	cErrors.Init();
	cParser.Init(&mcModule, &cErrors, "imports");
	bResult = cParser.Parse(szText.Text());
	cParser.Kill();
	cErrors.Kill();
	szText.Kill();

	//The function's restrict symbols stay restrict.
	for (ui = 0; ui < pcModule->maszSymbols.NumElements(); ui++)
	{
		if (pcModule->IsRestrict((uint32)ui))
		{
			iSymbol = mcModule.FindSymbol(pcModule->GetSymbolName((uint32)ui));
			if (iSymbol >= 0)
			{
				mcModule.SetRestrict((uint32)iSymbol);
			}
		}
	}
	mcModuleLock.Unlock();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The function named szName, NULL when no unit exported it.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CIRImports::Find(const char* szName)
{
	int				iSymbol;
	CIRFunction*	pcFunction;

	iSymbol = mcModule.FindSymbol(szName);
	if (iSymbol < 0)
	{
		return NULL;
	}
	pcFunction = mcModule.FindFunction((uint32)iSymbol);
	return (pcFunction && (pcFunction->NumBlocks() != 0)) ? pcFunction : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CIRImports::NumFunctions(void)
{
	return mcModule.NumFunctions();
}


//////////////////////////////////////////////////////////////////////////
//
// The module the functions were copied into (their symbols' names are
// pcModule's: CIRInliner maps them to the caller's).
//
//////////////////////////////////////////////////////////////////////////
CIRModule* CIRImports::GetModule(void)
{
	return &mcModule;
}
