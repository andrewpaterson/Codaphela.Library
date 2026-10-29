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
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CIRImports::Kill(void)
{
	mcModule.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcFunction's body can be copied to another unit: it has one, is
// small, has external linkage and refers to no symbol.
//
//////////////////////////////////////////////////////////////////////////
bool CIRImports::IsExportable(CIRFunction* pcFunction)
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
			if (psInstruction->asArguments[0].IsSymbol() || psInstruction->asArguments[1].IsSymbol())
			{
				return false;
			}
		}
		for (uiIndex = 0; uiIndex < pcBlock->masPhis.NumElements(); uiIndex++)
		{
			psPhi = pcBlock->masPhis.Get(uiIndex);
			for (uiArgument = 0; uiArgument < psPhi->asArguments.NumElements(); uiArgument++)
			{
				if (psPhi->asArguments.Get(uiArgument)->sValue.IsSymbol())
				{
					return false;
				}
			}
		}
		if (pcBlock->msJump.sArgument.IsSymbol())
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
//
//////////////////////////////////////////////////////////////////////////
bool CIRImports::Export(CIRModule* pcModule, CIRFunction* pcFunction)
{
	CChars		szText;
	CIRPrinter	cPrinter;
	CIRParser	cParser;
	CCPPErrors	cErrors;
	bool		bResult;

	if (!IsExportable(pcFunction) || Find(pcModule->GetSymbolName(pcFunction->muiSymbol)))
	{
		return false;
	}
	szText.Init();
	cPrinter.Init(pcModule, &szText);
	cPrinter.PrintFunction(pcFunction);
	cPrinter.Kill();
	cErrors.Init();
	cParser.Init(&mcModule, &cErrors, "imports");
	bResult = cParser.Parse(szText.Text());
	cParser.Kill();
	cErrors.Kill();
	szText.Kill();
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
