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
#include <stdlib.h>
#include <string.h>
#include "BaseLib/ConstructorCall.h"
#include "BaseLib/PointerFunctions.h"
#include "CTIdentifier.h"
#include "CTReservedWord.h"
#include "CTString.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CTInt.h"
#include "CTShort.h"
#include "CTLongLong.h"
#include "CTChar.h"
#include "CTBoolean.h"
#include "CTUserLiteral.h"
#include "CPPUnit.h"
#include "SourceFile.h"
#include "CPP65IRFrontEnd.h"
#include "CPPConstants.h"
#include "CPPSemanticPass.h"
#include "CPPTypeWriter.h"
#include "LiteralHelper.h"
#include "CPPExceptions.h"


//The children of an expression CCPPExceptions::MayThrow asks about.
class CCPPThrowFinder : public CCPPSyntaxChildVisitor
{
public:
	CCPPExceptions*	mpcExceptions;
	bool		mbThrows;

	void	Init(CCPPExceptions* pcExceptions) { mpcExceptions = pcExceptions; mbThrows = false; }
	void	Visit(CCPPSyntax* pcChild) override { mbThrows = mbThrows || mpcExceptions->MayThrow(pcChild); }
};


//////////////////////////////////////////////////////////////////////////
//
// A declaration's declarator: a variable's, a parameter's or a field's.
//
//////////////////////////////////////////////////////////////////////////
static CCPPSyntaxDeclarator* DeclaratorOfDeclaration(CCPPSyntax* pcDeclaration)
{
	if (!pcDeclaration)
	{
		return NULL;
	}
	switch (pcDeclaration->GetKind())
	{
	case CPPS_Declarator:
		return (CCPPSyntaxDeclarator*)pcDeclaration;
	case CPPS_InitDeclarator:
		return ((CCPPSyntaxInitDeclarator*)pcDeclaration)->mpcDeclarator;
	case CPPS_Parameter:
		return ((CCPPSyntaxParameter*)pcDeclaration)->mpcDeclarator;
	default:
		return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptions::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptions::Kill(void)
{
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What throw's operand makes can be thrown: not an object of an incomplete
// or abstract class, nor a pointer to an incomplete class.  [except.throw] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::CheckThrown(CCPPSyntaxThrowExpression* pcThrow)
{
	SCPP65Type*	psType;

	psType = mpcUnit->GetTypeChecker()->TypeOf(pcThrow->mpcOperand);
	if (!psType)
	{
		return false;
	}
	psType = psType->WithoutReference()->Unqualified();
	if (psType->IsVoid())
	{
		return mpcUnit->mcDiagnostics.Error(pcThrow, "A throw's operand can't be void");
	}
	if (psType->IsClass())
	{
		if (!mpcUnit->CompleteClass(psType->pcClass, pcThrow))
		{
			return false;
		}
		if (psType->pcClass->IsAbstract())
		{
			return mpcUnit->mcDiagnostics.Error(pcThrow, "An object of an abstract class can't be thrown");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The body's labels and gotos, for CheckTryJumps; and each case label is
// checked here: it can't be in a try block or a handler that is inside its
// switch.  A lambda's body is its own function's.  [except] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::CollectTryJumps(CCPPSyntax* pcNode, CCPPSyntax* pcBody, CArraySyntax* papcLabels, CArraySyntax* papcGotos)
{
	CCPPSyntax*	pcSwitch;
	size		i;
	bool		bResult;

	if (!pcNode || (pcNode->GetKind() == CPPS_LambdaExpression))
	{
		return true;
	}
	bResult = true;
	if (pcNode->GetKind() == CPPS_LabelStatement)
	{
		papcLabels->Add(pcNode);
	}
	else if (pcNode->GetKind() == CPPS_GotoStatement)
	{
		papcGotos->Add(pcNode);
	}
	else if (pcNode->GetKind() == CPPS_CaseStatement)
	{
		for (pcSwitch = pcNode->mpcParent; pcSwitch && (pcSwitch->GetKind() != CPPS_SwitchStatement) && (pcSwitch != pcBody); pcSwitch = pcSwitch->mpcParent)
		{
		}
		if (pcSwitch && (pcSwitch->GetKind() == CPPS_SwitchStatement) && mpcUnit->TryRegionOf(pcNode, pcSwitch))
		{
			bResult = mpcUnit->mcDiagnostics.Error(pcNode, "A switch can't jump into a try block or a handler");
		}
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		bResult &= CollectTryJumps(pcNode->GetChild(i), pcBody, papcLabels, papcGotos);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// No goto or switch of the function's body jumps into a try block or a
// handler: a goto whose label is in one must be in it too.  [except] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::CheckTryJumps(CCPPSyntax* pcBody)
{
	CArraySyntax	apcLabels;
	CArraySyntax	apcGotos;
	CCPPSyntax*		pcGoto;
	CCPPSyntax*		pcLabel;
	CCPPSyntax*		pcRegion;
	CCPPSyntax*		pcParent;
	CCPPToken*		pcGotoName;
	CCPPToken*		pcLabelName;
	size			i;
	size			j;
	bool			bResult;

	if (!mpcUnit->mpcOptions->mbExceptions || !pcBody)
	{
		return true;
	}
	apcLabels.Init();
	apcGotos.Init();
	bResult = CollectTryJumps(pcBody, pcBody, &apcLabels, &apcGotos);
	for (i = 0; i < apcGotos.NumElements(); i++)
	{
		pcGoto = apcGotos.GetPtr(i);
		pcGotoName = mpcUnit->mpcTokeniser->Get(((CCPPSyntaxGotoStatement*)pcGoto)->miLabel);
		for (j = 0; j < apcLabels.NumElements(); j++)
		{
			pcLabel = apcLabels.GetPtr(j);
			pcLabelName = mpcUnit->mpcTokeniser->Get(((CCPPSyntaxLabelStatement*)pcLabel)->miName);
			if (!pcGotoName || !pcLabelName || !pcGotoName->IsIdentifier() || !pcLabelName->IsIdentifier() ||
				(((CCTIdentifier*)pcGotoName)->miIdentifier != ((CCTIdentifier*)pcLabelName)->miIdentifier))
			{
				continue;
			}
			pcRegion = mpcUnit->TryRegionOf(pcLabel, pcBody);
			for (pcParent = pcGoto; pcRegion && pcParent && (pcParent != pcRegion); pcParent = pcParent->mpcParent)
			{
			}
			if (pcRegion && !pcParent)
			{
				bResult = mpcUnit->mcDiagnostics.Error(pcGoto, "A goto can't jump into a try block or a handler");
			}
		}
	}
	apcGotos.Kill();
	apcLabels.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The exception specification a function's declarator is written with,
// NULL for none.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxExceptionSpecification* CCPPExceptions::WrittenSpecification(CCPPSyntaxFunctionSuffix* pcSuffix)
{
	if (!pcSuffix || !pcSuffix->mpcExceptionSpecification || (pcSuffix->mpcExceptionSpecification->GetKind() != CPPS_ExceptionSpecification))
	{
		return NULL;
	}
	return (CCPPSyntaxExceptionSpecification*)pcSuffix->mpcExceptionSpecification;
}


//////////////////////////////////////////////////////////////////////////
//
// What an exception specification that is written allows: noexcept,
// noexcept(true) and throw() nothing, noexcept(false) and Microsoft's
// throw(...) anything, throw(A, B) those types.  [except.spec] 2 and 3
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptions::WrittenExceptions(CCPPSyntaxExceptionSpecification* pcSpecification, CCPPExceptionSpec* pcDest)
{
	SCPP65Type*	psType;
	int64		lliValue;
	size		i;

	pcDest->SetAnything();
	if (pcSpecification->mbNoexcept)
	{
		if (!pcSpecification->mpcCondition || (mpcUnit->mcFolder.Constant(pcSpecification->mpcCondition, &lliValue) && (lliValue != 0)))
		{
			pcDest->SetNothing();
		}
		return;
	}
	if (pcSpecification->mbAnything)
	{
		return;
	}
	pcDest->SetNothing();
	pcDest->mbDynamic = true;
	for (i = 0; i < pcSpecification->macpTypes.NumElements(); i++)
	{
		psType = mpcUnit->mcDeclarators.GetTypeIdType((CCPPSyntaxTypeId*)pcSpecification->macpTypes.GetPtr(i));
		if (psType)
		{
			pcDest->AddType(mpcUnit->GetFunctionDeclarer()->ParameterType(psType)->Unqualified());
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// What the exception specification a function is declared with allows.
// noexcept(e)'s e may name the function's parameters, as the return type
// written after them may: noexcept(noexcept(t.f())).
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptions::DeclaredExceptions(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator, CCPPSyntaxExceptionSpecification* pcSpecification, CCPPExceptionSpec* pcDest)
{
	if (!pcSpecification->mpcCondition)
	{
		WrittenExceptions(pcSpecification, pcDest);
		return;
	}
	mpcUnit->EnterFunction(pcFunction);
	mpcUnit->AddParameters(pcFunction, pcDeclarator);
	WrittenExceptions(pcSpecification, pcDest);
	mpcUnit->LeaveFunction();
}


//////////////////////////////////////////////////////////////////////////
//
// A new function's exception specification, from its declarator.  One
// declared with none allows anything, but: an operator delete allows
// nothing ([except.spec] 15), and a destructor's is worked out from its
// class when it is asked for (ExceptionsOf).
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptions::SetDeclaredExceptions(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxExceptionSpecification*	pcSpecification;

	pcSpecification = WrittenSpecification(pcDeclarator->GetFunctionSuffix());
	if (pcSpecification)
	{
		DeclaredExceptions(pcFunction, pcDeclarator, pcSpecification, &pcFunction->mcExceptions);
		pcFunction->mbExceptionsWritten = true;
	}
	else if (CPPIsDeleteName(pcFunction->mlliName))
	{
		pcFunction->mcExceptions.SetNothing();
		pcFunction->mbExceptionsWritten = true;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A function declared again: its exception specification must be
// compatible with the one it has ([except.spec] 3).  A declaration
// without one has what the function would have with none written, which
// for a destructor is its class's parts' ([class.dtor] 3).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::CheckRedeclaredExceptions(CCPPFunction* pcFunction, CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntaxExceptionSpecification*	pcSpecification;
	CCPPExceptionSpec					cDeclared;
	bool								bCompatible;

	if (pcDeclarator == pcFunction->mpcDeclarator)
	{
		return true;
	}
	pcSpecification = WrittenSpecification(pcDeclarator->GetFunctionSuffix());
	cDeclared.Init();
	if (pcSpecification)
	{
		DeclaredExceptions(pcFunction, pcDeclarator, pcSpecification, &cDeclared);
	}
	else if (CPPIsDeleteName(pcFunction->mlliName))
	{
		cDeclared.SetNothing();
	}
	else if ((pcFunction->meKind == CPPFK_Destructor) || pcFunction->mbImplicit)
	{
		if (!pcFunction->mbExceptionsWritten || !pcFunction->mpcClass || !pcFunction->mpcClass->IsComplete())
		{
			cDeclared.Kill();
			return true;
		}
		cDeclared.SetNothing();
		ImplicitExceptions(pcFunction, &cDeclared);
	}
	bCompatible = cDeclared.IsCompatible(ExceptionsOf(pcFunction));
	cDeclared.Kill();
	if (!bCompatible)
	{
		//Declarations are made quietly (one that fails is only an error if it is used); this one is wrong where it is.
		mpcUnit->mcDiagnostics.BeginLoud();
		mpcUnit->mcDiagnostics.Error(pcDeclarator, "The function is declared again with a different exception specification");
		mpcUnit->mcDiagnostics.EndLoud();
	}
	return bCompatible;
}


//////////////////////////////////////////////////////////////////////////
//
// What a function may throw.  One declared with a specification has that.
// A destructor declared without one, and a function the compiler writes,
// allow what the functions they call allow ([except.spec] 14): it is
// worked out here, once the class is complete.
//
//////////////////////////////////////////////////////////////////////////
CCPPExceptionSpec* CCPPExceptions::ExceptionsOf(CCPPFunction* pcFunction)
{
	if (pcFunction->mbExceptionsWritten || pcFunction->mbExceptionsKnown)
	{
		return &pcFunction->mcExceptions;
	}
	pcFunction->mbExceptionsKnown = true;
	if (!pcFunction->mpcClass || ((pcFunction->meKind != CPPFK_Destructor) && !pcFunction->mbImplicit))
	{
		return &pcFunction->mcExceptions;
	}
	pcFunction->mcExceptions.SetNothing();
	ImplicitExceptions(pcFunction, &pcFunction->mcExceptions);
	pcFunction->mbExceptionsKnown = pcFunction->mpcClass->IsComplete();
	return &pcFunction->mcExceptions;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::IsNoexcept(CCPPFunction* pcFunction)
{
	return ExceptionsOf(pcFunction)->IsNonThrowing();
}


//////////////////////////////////////////////////////////////////////////
//
// Adds to pcDest what a function with no specification written allows
// because of what it does to its class's bases and fields: a destructor
// destroys them, a default constructor makes them, a copy or move
// constructor or operator= copies or moves them, and an inherited
// constructor calls the base's and makes the rest.  [except.spec] 14
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptions::ImplicitExceptions(CCPPFunction* pcFunction, CCPPExceptionSpec* pcDest)
{
	CCPPClass*			pcClass;
	SCPP65Type*			psParameter;
	ECPPSpecialMember	eMember;
	bool				bCopier;
	bool				bMove;

	pcClass = pcFunction->mpcClass;
	psParameter = (pcFunction->mapsParameters.NumElements() > 0) ? pcFunction->mapsParameters.GetPtr(0) : NULL;
	bCopier = psParameter && psParameter->IsReference() && psParameter->psBase->IsClass() && (psParameter->psBase->pcClass == pcClass);
	bMove = bCopier && psParameter->bRvalue;
	if (pcFunction->meKind == CPPFK_Destructor)
	{
		eMember = CPPSM_Destroy;
	}
	else if (pcFunction->meKind == CPPFK_Constructor)
	{
		eMember = bCopier ? (bMove ? CPPSM_Move : CPPSM_Copy) : CPPSM_DefaultConstruct;
		if (pcFunction->mpcInherited)
		{
			pcDest->Add(ExceptionsOf(pcFunction->mpcInherited));
		}
	}
	else if (bCopier && (pcFunction->mlliName == CPP_OPERATOR_NAME(CT_Assignment)))
	{
		eMember = bMove ? CPPSM_MoveAssign : CPPSM_Assign;
	}
	else if (pcFunction->mbDeleting)
	{
		//A deleting destructor lets out what the destructor it calls does.
		pcDest->Add(ExceptionsOf(pcClass->mpcDestructor));
		return;
	}
	else
	{
		//A closure's conversion to a pointer to a function, which does nothing that throws.
		return;
	}
	PartsExceptions(pcClass, eMember, pcDest);
}


//////////////////////////////////////////////////////////////////////////
//
// Adds to pcDest what doing eMember to each base and field of the class
// allows, and for making an object what its fields' default member
// initialisers do.
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptions::PartsExceptions(CCPPClass* pcClass, ECPPSpecialMember eMember, CCPPExceptionSpec* pcDest)
{
	SCPPField*	psField;
	SCPP65Type*	psType;
	size		i;

	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		ClassExceptions(pcClass->macBases.Get(i)->pcClass, eMember, pcDest);
	}
	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		ClassExceptions(pcClass->macVirtualBases.Get(i)->pcClass, eMember, pcDest);
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		psField = pcClass->macFields.Get(i);
		psType = psField->psType;
		while (psType->IsArray())
		{
			psType = psType->psBase;
		}
		if (psType->IsClass() && (psType->pcClass != pcClass))
		{
			ClassExceptions(psType->pcClass, eMember, pcDest);
		}
		if ((eMember == CPPSM_DefaultConstruct) && psField->pcInitialiser && MayThrow(psField->pcInitialiser))
		{
			pcDest->SetAnything();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Adds to pcDest what doing eMember to an object of the class allows: what
// the function the class has for it allows, and when it has none (the
// compiler copies, makes or destroys the object part by part where it is
// needed) what its parts' do.
//
//////////////////////////////////////////////////////////////////////////
void CCPPExceptions::ClassExceptions(CCPPClass* pcClass, ECPPSpecialMember eMember, CCPPExceptionSpec* pcDest)
{
	CCPPFunction*	pcFunction;
	CCPPFunction*	pcMember;
	size			i;

	pcFunction = NULL;
	switch (eMember)
	{
	case CPPSM_Destroy:
		if (pcClass->mpcDestructor)
		{
			pcDest->Add(ExceptionsOf(pcClass->mpcDestructor));
		}
		return;
	case CPPSM_DefaultConstruct:
		for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
		{
			pcMember = pcClass->mapcMembers.GetPtr(i);
			if ((pcMember->meKind == CPPFK_Constructor) && !pcMember->mbDeleted && !pcMember->mpcInherited && (pcMember->RequiredParameters() == 0))
			{
				pcFunction = pcMember;
				break;
			}
		}
		break;
	case CPPSM_Copy:
	case CPPSM_Move:
		pcFunction = pcClass->GetPartCopier(false, eMember == CPPSM_Move);
		break;
	default:
		pcFunction = pcClass->GetPartCopier(true, eMember == CPPSM_MoveAssign);
		break;
	}
	if (pcFunction)
	{
		pcDest->Add(ExceptionsOf(pcFunction));
	}
	else
	{
		PartsExceptions(pcClass, eMember, pcDest);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether doing eMember to an object of the class may throw.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::ClassMayThrow(CCPPClass* pcClass, ECPPSpecialMember eMember)
{
	CCPPExceptionSpec	cExceptions;
	bool				bThrows;

	cExceptions.Init();
	cExceptions.SetNothing();
	ClassExceptions(pcClass, eMember, &cExceptions);
	bThrows = !cExceptions.IsNonThrowing();
	cExceptions.Kill();
	return bThrows;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcTighter allows only what pcLooser allows: what an overriding
// function's must do against the function's it overrides, and a function's
// against the pointer's it is assigned to.  A class is allowed where a
// base of it is.  [except.spec] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::AllowsOnlyWhat(CCPPExceptionSpec* pcTighter, CCPPExceptionSpec* pcLooser)
{
	SCPP65Type*	psType;
	SCPP65Type*	psAllowed;
	size		i;
	size		j;
	int			iOffset;
	bool		bAllowed;

	if (pcLooser->IsAnything() || pcTighter->IsNonThrowing())
	{
		return true;
	}
	if (pcTighter->IsAnything())
	{
		return false;
	}
	for (i = 0; i < pcTighter->mapsTypes.NumElements(); i++)
	{
		psType = pcTighter->mapsTypes.GetPtr(i);
		bAllowed = pcLooser->HasType(psType);
		for (j = 0; !bAllowed && (j < pcLooser->mapsTypes.NumElements()); j++)
		{
			psAllowed = pcLooser->mapsTypes.GetPtr(j);
			if (psType->IsPointer() && psAllowed->IsPointer() && !psType->IsMemberPointer() && !psAllowed->IsMemberPointer())
			{
				psType = psType->psBase->Unqualified();
				psAllowed = psAllowed->psBase->Unqualified();
			}
			bAllowed = psType->IsClass() && psAllowed->IsClass() && mpcUnit->IsDerived(psType->pcClass, psAllowed->pcClass, &iOffset);
			psType = pcTighter->mapsTypes.GetPtr(i);
		}
		if (!bAllowed)
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// What a call through pcCallee may throw, when pcCallee names a pointer
// or a reference to a function (p, *p, object.p, object->p): what its
// declarator says, anything if it says nothing.  False if pcCallee isn't
// such a name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::PointerExceptions(CCPPSyntax* pcCallee, CCPPExceptionSpec* pcDest)
{
	CCPPSyntaxExceptionSpecification*	pcSpecification;
	CCPPSyntaxDeclarator*				pcDeclarator;
	CCPPSymbol*							pcSymbol;

	pcCallee = mpcUnit->SkipParentheses(pcCallee);
	while (pcCallee && (pcCallee->GetKind() == CPPS_UnaryExpression) && (((CCPPSyntaxUnaryExpression*)pcCallee)->meOperator == CT_Multiply))
	{
		pcCallee = mpcUnit->SkipParentheses(((CCPPSyntaxUnaryExpression*)pcCallee)->mpcOperand);
	}
	pcSymbol = NULL;
	if (pcCallee && (pcCallee->GetKind() == CPPS_IdExpression))
	{
		pcSymbol = ((CCPPSyntaxIdExpression*)pcCallee)->mpcName->mpcSymbol;
	}
	else if (pcCallee && (pcCallee->GetKind() == CPPS_MemberExpression) && ((CCPPSyntaxMemberExpression*)pcCallee)->mpcName)
	{
		pcSymbol = ((CCPPSyntaxMemberExpression*)pcCallee)->mpcName->mpcSymbol;
	}
	pcDeclarator = pcSymbol ? DeclaratorOfDeclaration(pcSymbol->mpcDeclaration) : NULL;
	if (!pcDeclarator)
	{
		return false;
	}
	pcDest->SetAnything();
	pcSpecification = WrittenSpecification(mpcUnit->PointedFunctionSuffix(pcDeclarator));
	if (pcSpecification)
	{
		WrittenExceptions(pcSpecification, pcDest);
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// void (*p)() noexcept = f;  A pointer to a function declared with an
// exception specification is given only a function, or a pointer, that
// allows no more than it does.  [except.spec] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::CheckPointerExceptions(CCPPSyntaxInitDeclarator* pcInit)
{
	CCPPSyntaxExceptionSpecification*	pcSpecification;
	CCPPSyntaxSimpleDeclaration*		pcDeclaration;
	CCPPSyntax*							pcValue;
	CArraySyntax*						papcElements;
	CCPPExceptionSpec					cPointer;
	SCPP65Type*							psType;
	bool								bAllowed;

	pcSpecification = WrittenSpecification(mpcUnit->PointedFunctionSuffix(pcInit->mpcDeclarator));
	pcValue = pcInit->mpcInitializer;
	if (!pcSpecification || !pcValue || !pcInit->mpcParent || (pcInit->mpcParent->GetKind() != CPPS_SimpleDeclaration))
	{
		return true;
	}
	if ((pcValue->GetKind() == CPPS_ParenthesisedInitializer) || (pcValue->GetKind() == CPPS_InitializerList))
	{
		papcElements = mpcUnit->ListElements(pcValue);
		if (!papcElements || (papcElements->NumElements() != 1))
		{
			return true;
		}
		pcValue = papcElements->GetPtr(0);
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;

	cPointer.Init();
	WrittenExceptions(pcSpecification, &cPointer);
	mpcUnit->mcDiagnostics.BeginSilent();
	psType = mpcUnit->mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	bAllowed = !psType || ValueAllowsOnly(pcValue, psType, &cPointer);
	mpcUnit->mcDiagnostics.EndSilent();
	cPointer.Kill();
	if (!bAllowed)
	{
		return mpcUnit->mcDiagnostics.Error(pcInit, "The pointer is declared to allow fewer exceptions than the function it is given");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// p = f;  As CheckPointerExceptions, for a pointer to a function that is
// assigned to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::CheckAssignedExceptions(CCPPSyntaxBinaryExpression* pcAssignment)
{
	CCPPExceptionSpec	cPointer;
	SCPP65Type*			psType;
	bool				bAllowed;

	cPointer.Init();
	bAllowed = true;
	mpcUnit->mcDiagnostics.BeginSilent();
	if (PointerExceptions(pcAssignment->mpcLeft, &cPointer) && !cPointer.IsAnything())
	{
		psType = mpcUnit->GetTypeChecker()->TypeOf(pcAssignment->mpcLeft);
		bAllowed = !psType || ValueAllowsOnly(pcAssignment->mpcRight, psType, &cPointer);
	}
	mpcUnit->mcDiagnostics.EndSilent();
	cPointer.Kill();
	if (!bAllowed)
	{
		return mpcUnit->mcDiagnostics.Error(pcAssignment, "The pointer is declared to allow fewer exceptions than the function it is given");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a value given to a pointer (or a reference) to a function of
// the type psPointer allows only the exceptions pcPointer does: the
// function it names, or the pointer it names.  True for any other value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::ValueAllowsOnly(CCPPSyntax* pcValue, SCPP65Type* psPointer, CCPPExceptionSpec* pcPointer)
{
	CCPPSyntaxIdExpression*	pcName;
	CCPPFunction*			pcFunction;
	CCPPExceptionSpec		cValue;
	bool					bOverloaded;
	bool					bAllowed;

	if (psPointer->IsReference())
	{
		psPointer = mpcUnit->mcTypes.Pointer(psPointer->psBase);
	}
	pcName = mpcUnit->FunctionNameFor(pcValue, psPointer);
	if (pcName)
	{
		pcFunction = mpcUnit->mcBinder.FindAddressedFunction(pcName->mpcName, pcName->mpcName->mpcSymbol->mlliName, psPointer->psBase, pcName, &bOverloaded);
		return !pcFunction || AllowsOnlyWhat(ExceptionsOf(pcFunction), pcPointer);
	}
	cValue.Init();
	bAllowed = !PointerExceptions(pcValue, &cValue) || AllowsOnlyWhat(&cValue, pcPointer);
	cValue.Kill();
	return bAllowed;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether making an object of the class from a list of arguments (NULL
// for none), and destroying it when it is a temporary, may throw: the
// constructor chosen, or with one argument that is an object of the class
// its copy, and the destructor.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::ConstructionMayThrow(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode, bool bTemporary)
{
	CCPPBoundConstruction	cBound;
	CArraySyntax			apcNone;
	SCPP65Type*				psArgument;
	bool					bThrows;

	if (bTemporary && ClassMayThrow(pcClass, CPPSM_Destroy))
	{
		return true;
	}
	if (papcArguments && (papcArguments->NumElements() == 1))
	{
		psArgument = mpcUnit->GetTypeChecker()->TypeOf(papcArguments->GetPtr(0));
		psArgument = psArgument ? psArgument->WithoutReference() : NULL;
		if (psArgument && psArgument->IsClass() && (psArgument->pcClass == pcClass))
		{
			return ClassMayThrow(pcClass, CPPSM_Copy);
		}
	}
	apcNone.Init();
	bThrows = true;
	if (mpcUnit->mcBinder.BindConstruction(pcClass, papcArguments ? papcArguments : &apcNone, pcNode, &cBound, false))
	{
		bThrows = (cBound.meHow == CPPBO_Failed) || ((cBound.meHow == CPPBO_Constructor) && (!cBound.mpcConstructor || !IsNoexcept(cBound.mpcConstructor)));
	}
	apcNone.Kill();
	return bThrows;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether giving a parameter its argument may throw: an object of a class
// made from the argument by a constructor (and destroyed afterwards, a
// temporary), an object copied to be passed by value, or an object
// converted by its conversion function.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::ArgumentMayThrow(CCPPSyntax* pcArgument, SCPP65Type* psParameter)
{
	CCPPBoundConversion	cConversion;
	CArraySyntax		apcArgument;
	SCPP65Type*			psTo;
	SCPP65Type*			psFrom;
	int					iOffset;
	bool				bThrows;

	psTo = (psParameter->IsReference() ? psParameter->psBase : psParameter)->Unqualified();
	psFrom = mpcUnit->GetTypeChecker()->TypeOf(pcArgument);
	if (!psFrom)
	{
		return false;
	}
	psFrom = (psFrom->IsReference() ? psFrom->psBase : psFrom)->Unqualified();
	if (psTo->IsClass())
	{
		if (psFrom->IsClass() && ((psFrom->pcClass == psTo->pcClass) || mpcUnit->IsDerived(psFrom->pcClass, psTo->pcClass, &iOffset)))
		{
			//The object itself for a reference; a copy of it, which the function destroys, by value.
			return !psParameter->IsReference() && (ClassMayThrow(psTo->pcClass, CPPSM_Copy) || ClassMayThrow(psTo->pcClass, CPPSM_Destroy));
		}
		apcArgument.Init();
		apcArgument.Add(pcArgument);
		bThrows = ConstructionMayThrow(psTo->pcClass, &apcArgument, pcArgument, true);
		apcArgument.Kill();
		return bThrows;
	}
	if (psFrom->IsClass())
	{
		mpcUnit->mcBinder.BindConversion(pcArgument, psParameter, false, &cConversion);
		return (cConversion.meHow == CPPBV_Function) && (!cConversion.mpcFunction || !IsNoexcept(cConversion.mpcFunction));
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call may throw: the function it calls allows an exception, or
// it is through a pointer that does, or giving a parameter its argument
// may.  A destructor's call on an object whose class has none asks the
// class's parts.  C++14 says a call that is a constant expression doesn't
// throw whatever it is declared ([expr.unary.noexcept] 3); neither g++
// -std=c++14 nor the Microsoft compiler has that, C++17 took it out, and
// nor has this.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::CallMayThrow(CCPPSyntaxCallExpression* pcCall)
{
	CCPPBoundCall		cBound;
	CCPPExceptionSpec	cPointer;
	size				i;
	bool				bThrows;

	if (!mpcUnit->mcBinder.Bind(pcCall, &cBound, false))
	{
		return true;
	}
	switch (cBound.meHow)
	{
	case CPPBC_Destructor:
		if (cBound.mpcFunction)
		{
			return !IsNoexcept(cBound.mpcFunction);
		}
		return cBound.mpcClass && ClassMayThrow(cBound.mpcClass, CPPSM_Destroy);
	case CPPBC_Pointer:
		cPointer.Init();
		bThrows = !PointerExceptions(cBound.mpcCallee, &cPointer) || !cPointer.IsNonThrowing();
		cPointer.Kill();
		return bThrows;
	case CPPBC_Builtin:
	case CPPBC_InlineMemcpy:
		return false;
	case CPPBC_Member:
	case CPPBC_Function:
		break;
	default:
		return true;
	}
	if (!cBound.mpcFunction)
	{
		return true;
	}
	bThrows = !IsNoexcept(cBound.mpcFunction);
	for (i = 0; !bThrows && cBound.mpapcArguments && (i < cBound.mpapcArguments->NumElements()) && (i < cBound.mpcFunction->mapsParameters.NumElements()); i++)
	{
		bThrows = ArgumentMayThrow(cBound.mpapcArguments->GetPtr(i), cBound.mpcFunction->mapsParameters.GetPtr(i));
	}
	return bThrows;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether new may throw: its allocation function (the program's operator
// new unless the class has one; new (address) T allocates with nothing),
// and the constructor of what it makes.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::NewMayThrow(CCPPSyntaxNewExpression* pcNew)
{
	CCPPFunction*	pcAllocation;
	SCPP65Type*		psType;
	CArraySyntax	apcArguments;
	bool			bFailed;

	psType = mpcUnit->NewElementType(pcNew);
	if (!psType)
	{
		return true;
	}
	bFailed = false;
	apcArguments.Init();
	mpcUnit->NewArguments(pcNew, &apcArguments);
	pcAllocation = mpcUnit->mcBinder.FindAllocation(pcNew, psType, &apcArguments, &bFailed);
	apcArguments.Kill();
	if (bFailed || (pcAllocation ? !IsNoexcept(pcAllocation) : !pcNew->mbPlacement))
	{
		return true;
	}
	while (psType->IsArray())
	{
		psType = psType->psBase;
	}
	return psType->IsClass() && ConstructionMayThrow(psType->pcClass, pcNew->mpcInitializer ? mpcUnit->ListElements(pcNew->mpcInitializer) : NULL, pcNew, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether delete may throw: the destructor of what is deleted, and the
// class's operator delete if it has one (the program's doesn't).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::DeleteMayThrow(CCPPSyntaxDeleteExpression* pcDelete)
{
	CCPPFunction*	pcDeallocation;
	SCPP65Type*		psType;

	psType = mpcUnit->GetTypeChecker()->TypeOf(pcDelete->mpcOperand);
	psType = psType ? psType->WithoutReference() : NULL;
	psType = (psType && psType->IsPointer()) ? psType->psBase->Unqualified() : NULL;
	if (!psType || !psType->IsClass())
	{
		return false;
	}
	pcDeallocation = mpcUnit->mcBinder.FindDeallocation(psType->pcClass, pcDelete->mbArray);
	return ClassMayThrow(psType->pcClass, CPPSM_Destroy) || (pcDeallocation && !IsNoexcept(pcDeallocation));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression may throw ([expr.unary.noexcept] 3): it, or an
// operand of it that is worked out, calls a function that may (CallMayThrow),
// makes or converts an object with one that may, is a throw, a dynamic_cast
// to a reference or a typeid of an object of a polymorphic class, or is a
// new or a delete that may.  A lambda's body isn't its expression's, and the
// operands of sizeof, decltype and noexcept aren't worked out.  Not asked:
// the copies a lambda makes of what it captures by value.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPExceptions::MayThrow(CCPPSyntax* pcExpression)
{
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxBinaryExpression*		pcBinary;
	CCPPSyntaxConstructExpression*	pcConstruct;
	CCPPSyntaxNamedCastExpression*	pcCast;
	CCPPSyntaxTypeidExpression*		pcTypeid;
	CCPPBoundCall					cBound;
	CCPPThrowFinder					cFinder;
	SCPP65Type*						psType;
	bool							bThrows;

	bThrows = false;
	mpcUnit->mcDiagnostics.BeginSilent();
	switch (pcExpression->GetKind())
	{
	case CPPS_ThrowExpression:
		bThrows = true;
		break;
	case CPPS_SizeofExpression:
	case CPPS_TypeTraitExpression:
	case CPPS_NoexceptExpression:
	case CPPS_LambdaExpression:
		mpcUnit->mcDiagnostics.EndSilent();
		return false;
	case CPPS_CallExpression:
		bThrows = CallMayThrow((CCPPSyntaxCallExpression*)pcExpression);
		break;
	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		bThrows = mpcUnit->GetBinder()->IsOperatorOperand(pcUnary->mpcOperand, pcUnary->meOperator) && mpcUnit->mcBinder.BindOperator(pcUnary->meOperator, pcUnary->mpcOperand, NULL, pcUnary, &cBound, false) && cBound.mpcFunction && !IsNoexcept(cBound.mpcFunction);
		break;
	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		bThrows = (mpcUnit->GetBinder()->IsOperatorOperand(pcBinary->mpcLeft, pcBinary->meOperator) || mpcUnit->GetBinder()->IsOperatorOperand(pcBinary->mpcRight, pcBinary->meOperator)) && mpcUnit->mcBinder.BindOperator(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary, &cBound, false) && cBound.mpcFunction && !IsNoexcept(cBound.mpcFunction);
		break;
	case CPPS_NewExpression:
		bThrows = NewMayThrow((CCPPSyntaxNewExpression*)pcExpression);
		break;
	case CPPS_DeleteExpression:
		bThrows = DeleteMayThrow((CCPPSyntaxDeleteExpression*)pcExpression);
		break;
	case CPPS_ConstructExpression:
		//T(arguments) and T{arguments}: a temporary of a class.
		pcConstruct = (CCPPSyntaxConstructExpression*)pcExpression;
		psType = mpcUnit->mcDeclarators.GetTypeIdType(pcConstruct->mpcType);
		bThrows = psType && psType->IsClass() && ConstructionMayThrow(psType->pcClass, pcConstruct->mpcInitializer ? mpcUnit->ListElements(pcConstruct->mpcInitializer) : NULL, pcConstruct, true);
		break;
	case CPPS_NamedCastExpression:
		//dynamic_cast<T&> throws std::bad_cast when the object isn't a T.  [expr.dynamic.cast] 9
		pcCast = (CCPPSyntaxNamedCastExpression*)pcExpression;
		psType = (pcCast->meCast == CT_dynamic_cast) ? mpcUnit->mcDeclarators.GetTypeIdType(pcCast->mpcType) : NULL;
		bThrows = psType && psType->IsReference();
		break;
	case CPPS_TypeidExpression:
		//typeid of an object of a polymorphic class throws std::bad_typeid for *null.  [expr.typeid] 2
		pcTypeid = (CCPPSyntaxTypeidExpression*)pcExpression;
		psType = pcTypeid->mpcOperand ? mpcUnit->GetTypeChecker()->TypeOf(pcTypeid->mpcOperand) : NULL;
		psType = psType ? psType->WithoutReference() : NULL;
		bThrows = psType && psType->IsClass() && psType->pcClass->IsPolymorphic();
		break;
	default:
		break;
	}
	mpcUnit->mcDiagnostics.EndSilent();
	if (bThrows)
	{
		return true;
	}

	cFinder.Init(this);
	pcExpression->VisitFields(&cFinder);
	return cFinder.mbThrows;
}

