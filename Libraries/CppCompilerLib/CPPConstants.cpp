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
#include <string.h>
#include "BaseLib/PointerFunctions.h"
#include "CPPUnit.h"
#include "CPP65IRFrontEnd.h"
#include "CPPConstants.h"
#include "CPPUnitData.h"
#include "CPPTypeWriter.h"


#define CPP_CONSTANTS_NESTED	32		//Constants needed while a constant is written, at most.
#define CPP_CONSTANTS_TRAIL		8		//The functions a reason names, at most.


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mpcDiagnostics = pcUnit->GetDiagnostics();
	mcInterpreter.Init(this);
	mapcFrontEnds.Init();
	muiWriting = 0;
	masFunctions.Init();
	masVariables.Init();
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
	for (i = 0; i < masVariables.NumElements(); i++)
	{
		masVariables.Get(i)->szLabel.Kill();
	}
	masVariables.Kill();
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
const char* CCPPConstants::GetReason(void) const
{
	return mszReason.Text();
}


//////////////////////////////////////////////////////////////////////////
//
// No reason yet: the next expression's is its own (CCPPUnit::Constant).
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::ClearReason(void)
{
	if (mszReason.Length() > 0)
	{
		mszReason.Clear();
	}
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
	if (mpcUnit->GetContext()->IsWorking(CPPWK_ConstantValue, pcExpression))
	{
		mszReason.Append("It needs its own value");
		return false;
	}
	pcFrontEnd = BeginWriting();
	if (!pcFrontEnd)
	{
		mszReason.Append("Too many constants need each other");
		return false;
	}
	szName.Init();
	NewName("__constant", &szName);
	mpcUnit->GetContext()->EnterWorking(CPPWK_ConstantValue, pcExpression);
	pcIR = pcFrontEnd->ConstantExpression(szName.Text(), pcExpression, psType);
	mpcUnit->GetContext()->LeaveWorking();
	szName.Kill();
	EndWriting();
	if (!pcIR)
	{
		//The front end says why when it knows (SetReason).
		if (mszReason.Empty())
		{
			mszReason.Append("It uses what has no value until the program runs");
		}
		return false;
	}

	bResult = Run(pcFrontEnd->GetModule(), pcIR, NULL, 0, &sResult);
	pcFrontEnd->GetModule()->RemoveFunction(pcIR);
	if (!bResult)
	{
		return false;
	}
	if (sResult.bPointer && !psType->IsPointer())
	{
		//(long)&g: where g is isn't known until the program is linked.  [expr.const] 2.13
		mszReason.Append("It is an address, which is no number until the program is linked");
		return false;
	}
	*plliValue = mpcUnit->GetFolder()->ConstantOfType(psType, (int64)sResult.ulliBits);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Why the last constant asked for has no value, where the tree's folder
// knows (a signed overflow).
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::SetReason(const char* szReason)
{
	mszReason.Clear();
	mszReason.Append(szReason);
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a constant's IR, within the limits the compile options set.  If it
// has no value, the reason is the interpreter's, and the constexpr
// functions it failed in, innermost first: "It divides by zero, in f(int),
// called from g(int)".
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::Run(CIRModule* pcModule, CIRFunction* pcIR, SIRValue* pasArguments, size uiNumArguments, SIRValue* psResult)
{
	SCPPConstantFunction*	psFunction;
	CArrayChars*			paszTrail;
	CCPPFunction*			pcLast;
	size					i;
	size					uiNamed;
	size					uiRepeats;

	//The depth is of constexpr calls: the function the constant expression is written as is one more.
	mcInterpreter.SetLimits(mpcUnit->GetOptions()->muiConstantSteps, mpcUnit->GetOptions()->muiConstantDepth + 1);
	if (mcInterpreter.Call(pcModule, pcIR, pasArguments, uiNumArguments, psResult))
	{
		return true;
	}
	mszReason.Clear();
	mszReason.Append(mcInterpreter.GetError());
	paszTrail = mcInterpreter.GetTrail();
	pcLast = NULL;
	uiNamed = 0;
	uiRepeats = 0;
	for (i = 0; i < paszTrail->NumElements(); i++)
	{
		psFunction = FindFunction(paszTrail->Get(i)->Text());
		if (!psFunction)
		{
			continue;
		}
		if (psFunction->pcFunction == pcLast)
		{
			uiRepeats++;
			continue;
		}
		AppendRepeats(uiRepeats);
		uiRepeats = 0;
		if (uiNamed == CPP_CONSTANTS_TRAIL)
		{
			mszReason.Append(", ...");
			pcLast = NULL;
			break;
		}
		mszReason.Append((uiNamed == 0) ? ", in " : ", called from ");
		AppendFunctionName(psFunction->pcFunction, &mszReason);
		pcLast = psFunction->pcFunction;
		uiNamed++;
	}
	AppendRepeats(uiRepeats);
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A function that calls itself is named once in a reason's calls, and
// then how often it was called from itself.
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::AppendRepeats(size uiRepeats)
{
	if (uiRepeats == 0)
	{
		return;
	}
	mszReason.Append(", called from itself ");
	if (uiRepeats == 1)
	{
		mszReason.Append("once");
		return;
	}
	mszReason.Append((int)uiRepeats);
	mszReason.Append(" times");
}


//////////////////////////////////////////////////////////////////////////
//
// A function as the program names it: Class::name(parameter types), and a
// constructor as its class.
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::AppendFunctionName(CCPPFunction* pcFunction, CChars* pszDest)
{
	CCPPTypeWriter	cWriter;
	size			i;

	cWriter.Init(mpcUnit);
	if (pcFunction->mpcClass)
	{
		cWriter.AppendType(pcFunction->mpcClass->mpsType, pszDest);
		pszDest->Append("::");
	}
	if ((pcFunction->meKind == CPPFK_Constructor) && pcFunction->mpcClass)
	{
		cWriter.AppendType(pcFunction->mpcClass->mpsType, pszDest);
	}
	else if (pcFunction->IsOperator())
	{
		pszDest->Append("operator");
	}
	else
	{
		pszDest->Append(mpcUnit->GetTokeniser()->GetIdentifier(pcFunction->mlliName));
	}
	pszDest->Append('(');
	for (i = 0; i < pcFunction->mapsParameters.NumElements(); i++)
	{
		if (i > 0)
		{
			pszDest->Append(", ");
		}
		cWriter.AppendType(pcFunction->mapsParameters.GetPtr(i), pszDest);
	}
	pszDest->Append(')');
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
	if (pcFunction->mbImplicit && (pcFunction->meKind == CPPFK_Constructor) && !pcFunction->mpcDefinition)
	{
		//An implicit constructor is constexpr where what it does is: a literal class's default constructor, an
		//inherited constexpr constructor.  The interpreter finds out by running it.  [class.ctor] 5, [class.inhctor] 4
		return true;
	}
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
			mpcUnit->EnterFunction(pcFunction);
			mpcUnit->GetContext()->EnterBindings(pcFunction->mpcBindings);
			if (pcFunction->mpcDefinition)
			{
				//An implicit function's parameters have no names: the front end takes them as they come (Parameters).
				mpcUnit->AddParameters(pcFunction);
			}
			mpcDiagnostics->BeginQuiet();
			bLocals = !pcFunction->mpcDefinition || mpcUnit->AllocateLocals(pcFunction->mpcDefinition->mpcBody);
			mpcDiagnostics->EndQuiet();
			pcIR = bLocals ? pcFrontEnd->ConstantFunction(pcFunction) : NULL;
			mpcUnit->GetContext()->LeaveBindings();
			mpcUnit->LeaveFunction();
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
// A constant's IR names a variable (the front end tells of each as it
// writes the place of one), whose data the interpreter may then ask for.
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::NoteVariable(const char* szLabel, CCPPVariable* pcVariable)
{
	SCPPConstantVariable*	psVariable;

	if (FindVariable(szLabel))
	{
		return;
	}
	psVariable = masVariables.Add();
	psVariable->szLabel.Init(szLabel);
	psVariable->pcVariable = pcVariable;
	psVariable->uiObject = 0;
	psVariable->bAsked = false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
SCPPConstantVariable* CCPPConstants::FindVariable(const char* szLabel)
{
	size	i;

	for (i = 0; i < masVariables.NumElements(); i++)
	{
		if (masVariables.Get(i)->szLabel.Equals(szLabel))
		{
			return masVariables.Get(i);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A variable's initialiser (its declaration's, a variable template
// instance's its template's, or for a static member the one its class gave
// it) and its declaration's specifiers' flags; NULL if it has none.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPConstants::VariableInitialiser(CCPPVariable* pcVariable, uint32* puiFlags)
{
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntax*						pcInitialiser;

	*puiFlags = 0;
	pcInit = mpcUnit->GetInstantiator()->VariableDeclarator(pcVariable);
	if (!pcInit)
	{
		return NULL;
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_SimpleDeclaration) && pcDeclaration->mpcSpecifiers)
	{
		*puiFlags = pcDeclaration->mpcSpecifiers->muiFlags;
	}
	pcInitialiser = pcInit->mpcInitializer;
	return pcInitialiser ? pcInitialiser : pcVariable->mpcInClassInitialiser;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a variable's value can be read in a constant expression: one
// declared constexpr, a global or a local of the function being written,
// or a const integer or enum global with an initialiser (a local one's
// value is what the tree's folder gives).  [expr.const] 2.7
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::IsConstantVariable(CCPPVariable* pcVariable)
{
	SCPP65Type*	psType;
	uint32		uiFlags;

	if (((pcVariable->meStorage != CPPVS_Global) && (pcVariable->meStorage != CPPVS_Local)) || !VariableInitialiser(pcVariable, &uiFlags))
	{
		return false;
	}
	if (uiFlags & CPPDS_CONSTEXPR)
	{
		return true;
	}
	psType = pcVariable->mpsType;
	return (pcVariable->meStorage == CPPVS_Global) && psType->bConst && !psType->bVolatile && (psType->IsInteger() || (psType->eKind == CPP65TK_Bool));
}


//////////////////////////////////////////////////////////////////////////
//
// The object of a variable's value, worked out the first time it is asked
// for by running its initialiser where the object is: its constructor,
// what its list in braces gives each element, or the address a reference
// is bound to.  The object is frozen once it has its value.  0 if the
// symbol isn't such a variable, or its value can't be worked out
// (mszReason says why).
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPPConstants::ConstantObject(const char* szName)
{
	SCPPConstantVariable*	psVariable;
	CCPPVariable*			pcVariable;
	uint32					uiObject;
	size					uiVariable;

	psVariable = FindVariable(szName);
	if (!psVariable || psVariable->bAsked)
	{
		return psVariable ? psVariable->uiObject : 0;
	}
	psVariable->bAsked = true;
	pcVariable = psVariable->pcVariable;
	if (!IsConstantVariable(pcVariable))
	{
		return 0;
	}
	uiVariable = masVariables.GetIndex(psVariable);
	uiObject = Evaluate(pcVariable, szName);
	if (uiObject != 0)
	{
		masVariables.Get(uiVariable)->uiObject = uiObject;
	}
	return uiObject;
}


//////////////////////////////////////////////////////////////////////////
//
// Runs a variable's initialiser where its object (named szName) is, and
// freezes the object: 0, with the object forgotten, if it can't be worked
// out (mszReason says why).
//
//////////////////////////////////////////////////////////////////////////
uint32 CCPPConstants::Evaluate(CCPPVariable* pcVariable, const char* szName)
{
	CCPP65IRFrontEnd*		pcFrontEnd;
	CCPPSyntax*				pcInitialiser;
	CIRFunction*			pcIR;
	CChars					szInitialiser;
	CCPPTemplateArguments*	pcBindings;
	SIRValue				sObject;
	SIRValue				sResult;
	uint32					uiObject;
	uint32					uiFlags;
	bool					bResult;

	pcInitialiser = VariableInitialiser(pcVariable, &uiFlags);
	pcFrontEnd = BeginWriting();
	if (!pcFrontEnd)
	{
		return 0;
	}
	szInitialiser.Init();
	NewName("__initialise", &szInitialiser);
	//A variable template's instance has its own bindings.  A class template instance's static member has the class's:
	//those its definition is being made with (CCPPUnit::DefineInstanceStatics', under its own parameter names, with the
	//class's outside them for the initialiser the class gave it) when it is, else the class's.
	pcBindings = pcVariable->mpcBindings;
	if (pcVariable->meStorage == CPPVS_Local)
	{
		//A local is of the function being written, whose bindings are in use.
		pcBindings = mpcUnit->GetContext()->GetBindings();
	}
	if (!pcBindings && pcVariable->mpcClass && pcVariable->mpcClass->mpcBindings)
	{
		pcBindings = mpcUnit->GetContext()->GetBindings();
		if (!pcBindings || (pcBindings->mpcClass != pcVariable->mpcClass))
		{
			pcBindings = pcVariable->mpcClass->mpcBindings;
		}
	}
	mszReason.Clear();
	mpcUnit->GetContext()->EnterBindings(pcBindings);
	pcIR = pcFrontEnd->ConstantInitialiser(szInitialiser.Text(), pcVariable->mpsType, pcInitialiser);
	mpcUnit->GetContext()->LeaveBindings();
	szInitialiser.Kill();
	EndWriting();
	if (!pcIR)
	{
		//The front end says why when it is what can never be a constant (SetReason); anything else it can't write yet.
		if (mszReason.Empty())
		{
			mszReason.Append("Its initialiser uses what the compiler can't work out yet");
		}
		return 0;
	}

	uiObject = mcInterpreter.NewNamedObject(szName, (size)pcVariable->mpsType->iBytes);
	sObject = mcInterpreter.PointerTo(uiObject, 0, pcFrontEnd->GetModule()->GetPointerSize(0));
	bResult = Run(pcFrontEnd->GetModule(), pcIR, &sObject, 1, &sResult);
	pcFrontEnd->GetModule()->RemoveFunction(pcIR);
	if (!bResult)
	{
		mcInterpreter.Forget(uiObject);
		return 0;
	}
	mcInterpreter.Freeze(uiObject);
	return uiObject;
}


//////////////////////////////////////////////////////////////////////////
//
// A variable declared constexpr must be initialised with a constant
// expression: its value is worked out when it is defined, and an error
// where it can't be.  An initialiser the front end can't write for the
// interpreter yet is let be: its start up routine makes it at run time,
// as before.  [dcl.constexpr] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::CheckVariable(CCPPVariable* pcVariable, CCPPSyntax* pcNode)
{
	CChars		szLabel;
	uint32		uiFlags;
	bool		bResult;

	if (!VariableInitialiser(pcVariable, &uiFlags))
	{
		return mpcDiagnostics->ReportError(pcNode, "A constexpr variable needs an initialiser");
	}
	if (!IsLiteralType(pcVariable->mpsType))
	{
		//[dcl.constexpr] 9, [basic.types] 10
		return mpcDiagnostics->ReportError(pcNode, "A constexpr variable's type is a literal type: a class needs no destructor of its own and a constexpr constructor, or is an aggregate");
	}
	szLabel.Init();
	pcVariable->Refer();
	mpcUnit->GetMangler()->AppendVariableLabel(&szLabel, pcVariable);
	NoteVariable(szLabel.Text(), pcVariable);
	bResult = CheckObject(szLabel.Text(), pcNode);
	szLabel.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// CheckVariable for the variable noted as szLabel.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::CheckObject(const char* szLabel, CCPPSyntax* pcNode)
{
	CChars	szMessage;

	mszReason.Clear();
	if ((ConstantObject(szLabel) != 0) || (strncmp(mszReason.Text(), "Its initialiser uses", 20) == 0))
	{
		return true;
	}
	szMessage.Init("A constexpr variable's initialiser must be a constant: ");
	szMessage.Append(mszReason.Text());
	mpcDiagnostics->ReportError(pcNode, szMessage.Text());
	szMessage.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// CheckVariable for a local of the function being written, whose value is
// kept under a label of its own (LocalLabel) until the function is done
// (ForgetLocal).  A pointer or a reference is to what has static storage:
// not to a variable of the function, another constexpr one either.
// [expr.const] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::CheckLocal(CCPPVariable* pcVariable, CCPPSyntax* pcNode)
{
	CChars		szLabel;
	uint32		uiObject;
	bool		bResult;

	szLabel.Init();
	LocalLabel(pcVariable, &szLabel);
	bResult = CheckObject(szLabel.Text(), pcNode);
	uiObject = bResult ? ConstantObject(szLabel.Text()) : 0;
	szLabel.Kill();
	if ((uiObject != 0) && (pcVariable->mpsType->IsPointer() || pcVariable->mpsType->IsReference()) && PointsToLocal(uiObject))
	{
		return mpcDiagnostics->ReportError(pcNode, "A constexpr variable's initialiser must be a constant: It is the address of a variable of the function, which has none until the program runs");
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the pointer an object is (a pointer's, or a reference's) points
// into a local's object (LocalLabel).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::PointsToLocal(uint32 uiObject)
{
	SIRObject*	psObject;
	uint32		uiTarget;

	psObject = mcInterpreter.GetObject(uiObject);
	uiTarget = (psObject->uiSize > 0) ? IR_SOURCE_OBJECT(psObject->pauiSource[0]) : 0;
	return (uiTarget != 0) && mcInterpreter.GetObject(uiTarget)->szName.StartsWith("__local");
}


//////////////////////////////////////////////////////////////////////////
//
// The label a constexpr local of the function being written is known to
// the interpreter by: one made for it the first time it is asked for.
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::LocalLabel(CCPPVariable* pcVariable, CChars* pszLabel)
{
	size	i;

	for (i = 0; i < masVariables.NumElements(); i++)
	{
		if (masVariables.Get(i)->pcVariable == pcVariable)
		{
			pszLabel->Append(masVariables.Get(i)->szLabel);
			return;
		}
	}
	NewName("__local", pszLabel);
	NoteVariable(pszLabel->Text(), pcVariable);
}


//////////////////////////////////////////////////////////////////////////
//
// A local is freed with its function: what was kept of it goes too.
//
//////////////////////////////////////////////////////////////////////////
void CCPPConstants::ForgetLocal(CCPPVariable* pcVariable)
{
	SCPPConstantVariable*	psVariable;
	size					i;

	for (i = masVariables.NumElements(); i > 0; i--)
	{
		psVariable = masVariables.Get(i - 1);
		if (psVariable->pcVariable == pcVariable)
		{
			if (psVariable->uiObject != 0)
			{
				mcInterpreter.Forget(psVariable->uiObject);
			}
			psVariable->szLabel.Kill();
			masVariables.RemoveAt(i - 1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What [dcl.constexpr] 3 and 4 ask of a constexpr function's definition,
// whether or not it is ever called for a constant: it isn't virtual, it
// returns and takes literal types, its body has no goto, try, static or
// thread_local variable or variable left uninitialised, and a constructor
// initialises every member that has no constructor of its own.  An
// instance of a template that doesn't is only not constexpr ([dcl.constexpr]
// 6), so instances aren't checked.  A class being laid out (a constexpr
// member of it) is taken to be literal.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::CheckDefinition(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	size							i;

	pcDefinition = pcFunction->mpcDefinition;
	if (!pcFunction->mbConstexpr || !pcDefinition || pcFunction->mpcTemplate || (pcFunction->mpcClass && pcFunction->mpcClass->mpcTemplate))
	{
		return true;
	}
	if (pcFunction->mbVirtual)
	{
		return mpcDiagnostics->ReportError(pcDefinition, "A constexpr function can't be virtual");
	}
	if (pcFunction->meKind == CPPFK_Destructor)
	{
		return mpcDiagnostics->ReportError(pcDefinition, "A destructor can't be constexpr");
	}
	if ((pcFunction->meKind == CPPFK_Function) && !IsLiteralType(pcFunction->mpsReturn))
	{
		return mpcDiagnostics->ReportError(pcDefinition, "A constexpr function must return a literal type");
	}
	for (i = 0; i < pcFunction->mapsParameters.NumElements(); i++)
	{
		if (!IsLiteralType(pcFunction->mapsParameters.GetPtr(i)))
		{
			return mpcDiagnostics->ReportError(pcDefinition, "A constexpr function's parameters must be of literal types");
		}
	}
	if ((pcFunction->meKind == CPPFK_Constructor) && !CheckConstructor(pcFunction))
	{
		return false;
	}
	return CheckBody(pcDefinition->mpcBody);
}


//////////////////////////////////////////////////////////////////////////
//
// True if objects of the type can be constants: void, scalars, references,
// arrays of literal types, and a class with no destructor to run whose
// members and bases are literal, that is an aggregate or has a constexpr
// constructor.  [basic.types] 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::IsLiteralType(SCPP65Type* psType)
{
	CCPPClass*		pcClass;
	CCPPFunction*	pcFunction;
	size			i;
	bool			bConstructor;

	if (!psType || psType->IsReference() || !psType->IsClass())
	{
		return !psType || !psType->IsArray() || IsLiteralType(psType->psBase);
	}
	pcClass = psType->pcClass;
	if (!pcClass->IsComplete() && pcClass->mpcSpecifier && !pcClass->IsBeingMade())
	{
		//A class is laid out when it is first needed whole: this is such a need.
		mpcUnit->GetClasses()->CompleteClass(pcClass, NULL);
	}
	if (!pcClass->IsComplete())
	{
		return true;
	}
	if (pcClass->NeedsDestructor())
	{
		return false;
	}
	bConstructor = !pcClass->mbConstructors;
	for (i = 0; !bConstructor && (i < pcClass->mapcMembers.NumElements()); i++)
	{
		pcFunction = pcClass->mapcMembers.GetPtr(i);
		bConstructor = (pcFunction->meKind == CPPFK_Constructor) && pcFunction->mbConstexpr && (pcFunction != pcClass->GetCopier(false, false)) && (pcFunction != pcClass->GetCopier(false, true));
	}
	if (!bConstructor)
	{
		return false;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if (!IsLiteralType(pcClass->macBases.Get(i)->pcClass->mpsType))
		{
			return false;
		}
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		if (!IsLiteralType(pcClass->macFields.Get(i)->psType))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What a constexpr function's body may not have.  A class defined in it
// is its own.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::CheckBody(CCPPSyntax* pcNode)
{
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxInitDeclarator*		pcInit;
	size							i;

	if (!pcNode || (pcNode->GetKind() == CPPS_ClassSpecifier) || (pcNode->GetKind() == CPPS_LambdaExpression))
	{
		return true;
	}
	switch (pcNode->GetKind())
	{
	case CPPS_GotoStatement:
		return mpcDiagnostics->ReportError(pcNode, "A constexpr function can't have goto");
	case CPPS_TryStatement:
		return mpcDiagnostics->ReportError(pcNode, "A constexpr function can't have try");
	case CPPS_SimpleDeclaration:
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcNode;
		if (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & CPPDS_TYPEDEF))
		{
			return true;
		}
		if (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_THREAD_LOCAL)))
		{
			return mpcDiagnostics->ReportError(pcNode, "A constexpr function can't have a static or thread_local variable");
		}
		for (i = 0; i < pcDeclaration->macpDeclarators.NumElements(); i++)
		{
			pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(i);
			if ((pcInit->GetKind() == CPPS_InitDeclarator) && !pcInit->mpcInitializer && pcInit->mpcDeclarator && !pcInit->mpcDeclarator->IsFunction() &&
				NeedsInitialiser(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit))
			{
				return mpcDiagnostics->ReportError(pcInit, "A variable in a constexpr function must be initialised");
			}
		}
		break;
	default:
		break;
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		if (!CheckBody(pcNode->GetChild(i)))
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a variable or member declared so, with no initialiser, is left
// with no value: what isn't an object (whose constructor gives it one) or
// an array of them.  False if its type can't be worked out here.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::NeedsInitialiser(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntax* pcNode)
{
	SCPP65Type*	psType;

	mpcDiagnostics->BeginSilent();
	psType = mpcUnit->GetDeclarators()->GetType(pcSpecifiers, pcDeclarator, pcNode);
	mpcDiagnostics->EndSilent();
	while (psType && psType->IsArray())
	{
		psType = psType->psBase;
	}
	return psType && !psType->IsClass() && !psType->IsReference();
}


//////////////////////////////////////////////////////////////////////////
//
// A constexpr constructor initialises every member that needs a value:
// with a member initialiser or its default one.  One that delegates to
// another of its class's, and a union's, are left be.  [dcl.constexpr] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::CheckConstructor(CCPPFunction* pcFunction)
{
	CCPPSyntaxFunctionDefinition*	pcDefinition;
	CCPPSyntaxClassSpecifier*		pcSpecifier;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxName*					pcName;
	CCPPSyntax*						pcMember;
	CChars							szMessage;
	size							i;
	size							j;

	pcDefinition = pcFunction->mpcDefinition;
	pcSpecifier = pcFunction->mpcClass->mpcSpecifier;
	if (!pcSpecifier || pcFunction->mpcClass->mbUnion || IsInitialisedMember(pcDefinition, pcFunction->mpcClass->mlliName))
	{
		return true;
	}
	for (i = 0; i < pcSpecifier->macpMembers.NumElements(); i++)
	{
		pcMember = pcSpecifier->macpMembers.GetPtr(i);
		if (pcMember->GetKind() != CPPS_SimpleDeclaration)
		{
			continue;
		}
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcMember;
		if (pcDeclaration->mpcSpecifiers && (pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_TYPEDEF | CPPDS_STATIC | CPPDS_FRIEND)))
		{
			continue;
		}
		for (j = 0; j < pcDeclaration->macpDeclarators.NumElements(); j++)
		{
			pcInit = (CCPPSyntaxInitDeclarator*)pcDeclaration->macpDeclarators.GetPtr(j);
			if ((pcInit->GetKind() != CPPS_InitDeclarator) || pcInit->mpcInitializer || !pcInit->mpcDeclarator || pcInit->mpcDeclarator->IsFunction())
			{
				continue;
			}
			pcName = pcInit->mpcDeclarator->GetName();
			if (!pcName || IsInitialisedMember(pcDefinition, mpcUnit->GetComponentName(pcName->GetLast())) || !NeedsInitialiser(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit))
			{
				continue;
			}
			szMessage.Init("A constexpr constructor must initialise every member [");
			mpcUnit->GetCheck()->AppendTokensText(pcName->GetLast()->miName, pcName->GetLast()->miName, &szMessage);
			szMessage.Append("]");
			mpcDiagnostics->ReportError(pcDefinition, szMessage.Text());
			szMessage.Kill();
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the constructor's member initialisers name the member (or the
// class, for one that delegates).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::IsInitialisedMember(CCPPSyntaxFunctionDefinition* pcDefinition, int64 lliName)
{
	CCPPSyntaxMemberInitializer*	pcInitialiser;
	size							i;

	for (i = 0; i < pcDefinition->macpMemberInitializers.NumElements(); i++)
	{
		pcInitialiser = (CCPPSyntaxMemberInitializer*)pcDefinition->macpMemberInitializers.GetPtr(i);
		if (pcInitialiser->mpcName && (mpcUnit->GetComponentName(pcInitialiser->mpcName->GetLast()) == lliName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Writes a constexpr variable's value as its data, if it was worked out
// when it was defined (CheckVariable): its bytes as the interpreter made
// them, and each pointer in them as the label of what it points into and
// the offset.  So it has its value before any of the program runs, as a
// constant initialisation must ([basic.start.static] 2), and needs no
// start up routine.  False, with nothing written, if its value isn't
// known or has a pointer that can't be data; it is then made at run time.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::AppendData(CCPPVariable* pcVariable, CCPPUnitData* pcData)
{
	SCPPConstantVariable*	psVariable;
	CChars					szLabel;

	szLabel.Init();
	pcVariable->Refer();
	mpcUnit->GetMangler()->AppendVariableLabel(&szLabel, pcVariable);
	psVariable = FindVariable(szLabel.Text());
	szLabel.Kill();
	if (!psVariable || (psVariable->uiObject == 0))
	{
		return false;
	}
	return AppendObjectData(psVariable->uiObject, pcData);
}


//////////////////////////////////////////////////////////////////////////
//
// A global that isn't constexpr but whose initialiser is a constant
// expression: an object made by a constexpr constructor from constants, a
// reference bound to an object of static storage, a pointer to one.  It is
// initialised before anything runs, as data ([basic.start.init] 2): true
// if its data is written.  Its value isn't a constant for anything else,
// so the interpreter's object for it is forgotten.  False, with nothing
// written, where the initialiser isn't such a constant: it is then made by
// its start up routine.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::AppendConstantInitialisation(CCPPVariable* pcVariable, CCPPUnitData* pcData)
{
	CChars		szLabel;
	CChars		szName;
	uint32		uiObject;
	uint32		uiFlags;
	bool		bResult;

	if (!VariableInitialiser(pcVariable, &uiFlags))
	{
		return false;
	}
	szLabel.Init();
	pcVariable->Refer();
	mpcUnit->GetMangler()->AppendVariableLabel(&szLabel, pcVariable);
	szName.Init();
	NewName("__static", &szName);
	mpcDiagnostics->BeginQuiet();
	uiObject = Evaluate(pcVariable, szName.Text());
	mpcDiagnostics->EndQuiet();
	szName.Kill();
	szLabel.Kill();
	mszReason.Clear();
	if (uiObject == 0)
	{
		return false;
	}
	bResult = AppendObjectData(uiObject, pcData);
	mcInterpreter.Forget(uiObject);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// An object's bytes as data (AppendData).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::AppendObjectData(uint32 uiObject, CCPPUnitData* pcData)
{
	SIRObject*				psObject;
	CChars					szText;
	size					i;
	int						iPointerBytes;
	bool					bResult;
	int						iPass;

	psObject = mcInterpreter.GetObject(uiObject);
	iPointerBytes = mpcUnit->GetTarget()->GetPointerBytes();

	//Checked first (with no strings added), then written.
	bResult = true;
	szText.Init();
	for (iPass = 0; bResult && (iPass < 2); iPass++)
	{
		szText.Clear();
		for (i = 0; bResult && (i < psObject->uiSize); )
		{
			if (psObject->pauiSource[i] == 0)
			{
				szText.Append("\t\t.byte ");
				szText.Append((int)(psObject->pucWritten[i] ? psObject->pucBytes[i] : 0));
				szText.AppendNewLine();
				i++;
				continue;
			}
			bResult = PointerData(psObject, i, iPointerBytes, (iPass == 1) ? pcData : NULL, &szText);
			i += (size)iPointerBytes;
		}
	}
	if (bResult)
	{
		pcData->GetText()->Append(&szText);
	}
	szText.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A pointer of iPointerBytes at uiOffset of an object's bytes as data:
// the label of the variable, function or string it points into, and the
// offset.  With pcData NULL it is only checked (a string isn't added to
// the unit's).  False if the bytes aren't one whole pointer, or it points
// into what has no label in the program.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPConstants::PointerData(SIRObject* psObject, size uiOffset, int iPointerBytes, CCPPUnitData* pcData, CChars* pszText)
{
	SIRObject*				psTarget;
	SCPPConstantFunction*	psFunction;
	uint32					uiTarget;
	uint64					ulliOffset;
	int						i;
	int						iString;

	if ((uiOffset + (size)iPointerBytes > psObject->uiSize) || ((iPointerBytes != 2) && (iPointerBytes != 4)))
	{
		return false;
	}
	uiTarget = IR_SOURCE_OBJECT(psObject->pauiSource[uiOffset]);
	ulliOffset = 0;
	for (i = 0; i < iPointerBytes; i++)
	{
		if (psObject->pauiSource[uiOffset + i] != IR_SOURCE(uiTarget, i))
		{
			return false;
		}
		ulliOffset |= ((uint64)psObject->pucBytes[uiOffset + i]) << (8 * i);
	}

	psTarget = mcInterpreter.GetObject(uiTarget);
	if (psTarget->bDead || (psTarget->szName.Length() == 0) || ((psTarget->uiKind != IROB_Global) && (psTarget->uiKind != IROB_Function) && (psTarget->uiKind != IROB_Unknown)))
	{
		return false;
	}
	pszText->Append((iPointerBytes == 2) ? "\t\t.word " : "\t\t.long ");
	if (psTarget->szName.StartsWith("__string"))
	{
		//A constant's string: the unit's own string, with the same bytes (AddString adds the zero at its end).
		if ((psTarget->uiSize == 0) || (psTarget->pucBytes[psTarget->uiSize - 1] != 0))
		{
			return false;
		}
		iString = pcData ? pcData->AddString((char*)psTarget->pucBytes, psTarget->uiSize - 1) : 0;
		pszText->Append("Str");
		pszText->Append(iString);
	}
	else
	{
		if (pcData && (psTarget->uiKind != IROB_Global))
		{
			//A function's address in data: it must be written.  (One the interpreter never ran is of no kind to it.)
			psFunction = FindFunction(psTarget->szName.Text());
			if (psFunction)
			{
				mpcUnit->RequireFunction(psFunction->pcFunction);
			}
		}
		pszText->Append(&psTarget->szName);
	}
	if (ulliOffset != 0)
	{
		pszText->Append('+');
		pszText->Append((int64)ulliOffset);
	}
	pszText->AppendNewLine();
	return true;
}
