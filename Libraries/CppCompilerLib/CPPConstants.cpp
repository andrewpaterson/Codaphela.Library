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
#include "BaseLib/PointerFunctions.h"
#include "CPPUnit.h"
#include "CPP65IRFrontEnd.h"
#include "CPPConstants.h"


#define CPP_CONSTANTS_NESTED	32		//Constants needed while a constant is written, at most.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mcInterpreter.Init(this);
	mapcFrontEnds.Init();
	muiWriting = 0;
	masFunctions.Init();
	muiNames = 0;
	mszReason.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::Kill(void)
{
	size	i;

	mszReason.Kill();
	for (i = 0; i < masFunctions.NumElements(); i++)
	{
		masFunctions.Get(i)->szLabel.Kill();
	}
	masFunctions.Kill();
	for (i = 0; i < mapcFrontEnds.NumElements(); i++)
	{
		mapcFrontEnds.GetPtr(i)->Kill();
		free(mapcFrontEnds.GetPtr(i));
	}
	mapcFrontEnds.Kill();
	mcInterpreter.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Why the last expression Value was asked for had none.
//
//////////////////////////////////////////////////////////////////////////
char* CCPPConstants::GetReason(void)
{
	return mszReason.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// A name nothing else in the unit has, for a constant's function or data:
// the interpreter knows things by their names, whichever module they are in.
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::NewName(const char* szPrefix, CChars* pszName)
{
	pszName->Append(szPrefix);
	pszName->Append((int)muiNames);
	muiNames++;
}


//////////////////////////////////////////////////////////////////////////
//
// A front end to write a function with: the first that isn't in the
// middle of one.  NULL when too many are.
//
//////////////////////////////////////////////////////////////////////////
CCPP65IRFrontEnd* CCPPConstants::BeginWriting(void)
{
	CCPP65IRFrontEnd*	pcFrontEnd;

	if (muiWriting >= CPP_CONSTANTS_NESTED)
	{
		return NULL;
	}
	if (muiWriting == mapcFrontEnds.NumElements())
	{
		pcFrontEnd = NewMalloc<CCPP65IRFrontEnd>();
		pcFrontEnd->Init(mpcUnit, true);
		mapcFrontEnds.Add(pcFrontEnd);
	}
	muiWriting++;
	return mapcFrontEnds.GetPtr(muiWriting - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::EndWriting(void)
{
	muiWriting--;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of an expression of psType (an integer's) that the tree's
// folder couldn't give, if it has one: the expression is written as a
// function that returns it, and that is run.  GetReason says why not.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::Value(CCPPSyntax* pcExpression, SCPP65Type* psType, int64* plliValue)
{
	CCPP65IRFrontEnd*	pcFrontEnd;
	CIRFunction*		pcIR;
	SIRValue			sResult;
	CChars				szName;
	bool				bResult;

	mszReason.Clear();
	pcFrontEnd = BeginWriting();
	if (!pcFrontEnd)
	{
		mszReason.Append("Too many constants need each other");
		return false;
	}
	szName.Init();
	NewName("__constant", &szName);
	pcIR = pcFrontEnd->ConstantExpression(szName.Text(), pcExpression, psType);
	szName.Kill();
	EndWriting();
	if (!pcIR)
	{
		mszReason.Append("It uses what has no value until the program runs");
		return false;
	}

	bResult = mcInterpreter.Call(pcFrontEnd->GetModule(), pcIR, NULL, 0, &sResult);
	pcFrontEnd->GetModule()->RemoveFunction(pcIR);
	if (!bResult)
	{
		mszReason.Append(mcInterpreter.GetError());
		return false;
	}
	*plliValue = mpcUnit->ConstantOfType(psType, (int64)sResult.ulliBits);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A constant's IR names a function (the front end tells of each as it
// writes a call of it), which the interpreter may then ask for.
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::NoteFunction(const char* szLabel, CCPPFunction* pcFunction)
{
	SCPPConstantFunction*	psFunction;

	if (FindFunction(szLabel))
	{
		return;
	}
	psFunction = masFunctions.Add();
	psFunction->szLabel.Init(szLabel);
	psFunction->pcFunction = pcFunction;
	psFunction->pcIR = NULL;
	psFunction->pcModule = NULL;
	psFunction->bAsked = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPPConstantFunction* CCPPConstants::FindFunction(const char* szLabel)
{
	size	i;

	for (i = 0; i < masFunctions.NumElements(); i++)
	{
		if (masFunctions.Get(i)->szLabel.Equals(szLabel))
		{
			return masFunctions.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the function was defined constexpr: only such a function's value
// may be worked out when the program is compiled.  [expr.const] 2.2
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::IsConstexpr(CCPPFunction* pcFunction)
{
	return pcFunction->mpcDefinition && pcFunction->mpcDefinition->mpcSpecifiers && (pcFunction->mpcDefinition->mpcSpecifiers->muiFlags & CPPDS_CONSTEXPR);
}


//////////////////////////////////////////////////////////////////////////
//
// The IR of a function the interpreter has met a call of, if it is a
// constexpr function with a body: written the first time, with its
// parameters and locals made as they are to generate it, in a function
// scope of its own, as another function may be being written.
//
//////////////////////////////////////////////////////////////////////////
CIRFunction* CCPPConstants::ConstantFunction(const char* szName, CIRModule** ppcModule)
{
	SCPPConstantFunction*	psFunction;
	CCPP65IRFrontEnd*		pcFrontEnd;
	CCPPFunction*			pcFunction;
	CIRFunction*			pcIR;
	SCPPFunctionScope		sScope;
	size					uiFunction;
	bool					bLocals;

	psFunction = FindFunction(szName);
	if (!psFunction)
	{
		return NULL;
	}
	if (!psFunction->bAsked)
	{
		psFunction->bAsked = true;
		pcFunction = psFunction->pcFunction;
		uiFunction = masFunctions.GetIndex(psFunction);
		pcFrontEnd = IsConstexpr(pcFunction) ? BeginWriting() : NULL;
		if (pcFrontEnd)
		{
			mpcUnit->EnterFunctionScope(&sScope, pcFunction);
			mpcUnit->GetContext()->EnterBindings(pcFunction->mpcBindings);
			mpcUnit->AddParameters(pcFunction);
			mpcUnit->BeginQuiet();
			bLocals = mpcUnit->AllocateLocals(pcFunction->mpcDefinition->mpcBody);
			mpcUnit->EndQuiet();
			pcIR = bLocals ? pcFrontEnd->ConstantFunction(pcFunction) : NULL;
			mpcUnit->GetContext()->LeaveBindings();
			mpcUnit->LeaveFunctionScope(&sScope);
			EndWriting();

			//Writing it may have noted more functions.
			psFunction = masFunctions.Get(uiFunction);
			psFunction->pcIR = pcIR;
			psFunction->pcModule = pcFrontEnd->GetModule();
		}
	}
	*ppcModule = psFunction->pcModule;
	return psFunction->pcIR;
}


//////////////////////////////////////////////////////////////////////////
//
// The data of a variable that is a constant (ConstexprPlan.txt stage 5).
//
//////////////////////////////////////////////////////////////////////////
CIRGlobal* CCPPConstants::ConstantGlobal(const char* szName, CIRModule** ppcModule)
{
	return NULL;
}
