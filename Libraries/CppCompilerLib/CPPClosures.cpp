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
#include "CPPSyntaxCopier.h"
#include "CPPUnit.h"
#include "CPPSyntaxes.h"
#include "CPPClosures.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClosures::Init(CCPPUnit* pcUnit)
{
	mpcUnit = pcUnit;
	mapsLambdas.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClosures::Kill(void)
{
	FreeLambdas();
	mpcUnit = NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The closure of a lambda in the function being written, made the first
// time it is asked for: an unnamed class, its fields what the lambda
// captures, its operator() the lambda's body (or one for each call's
// arguments, for a generic lambda), and for a lambda that captures
// nothing a conversion to a pointer to a function.  NULL after an error.
// [expr.prim.lambda]
//
//////////////////////////////////////////////////////////////////////////
SCPPLambda* CCPPClosures::GetLambda(CCPPSyntaxLambdaExpression* pcSyntax)
{
	SCPPLambda*					psLambda;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	size						i;

	for (i = 0; i < mapsLambdas.NumElements(); i++)
	{
		psLambda = mapsLambdas.GetPtr(i);
		if ((psLambda->pcSyntax == pcSyntax) && (psLambda->pcEnclosing == mpcUnit->mcContext.GetFunction()) && (psLambda->pcBindings == mpcUnit->mcContext.GetBindings()))
		{
			if (psLambda->bFailed)
			{
				mpcUnit->mcDiagnostics.Report(psLambda->pcErrors);
				return NULL;
			}
			return psLambda;
		}
	}

	psLambda = NewMalloc<SCPPLambda>();
	psLambda->pcSyntax = pcSyntax;
	psLambda->pcEnclosing = mpcUnit->mcContext.GetFunction();
	psLambda->pcBindings = mpcUnit->mcContext.GetBindings();
	psLambda->asCaptures.Init();
	psLambda->bThis = false;
	psLambda->cFunctionPacks.Init();
	psLambda->cFunctionPacks.AddAll(mpcUnit->GetFunctionPacks());
	psLambda->bMutable = pcSyntax->IsMutable();
	psLambda->bFailed = false;
	psLambda->pcErrors = NULL;
	psLambda->pcCall = NULL;
	psLambda->szName.Init();
	psLambda->pcClass = mpcUnit->NewClass(NULL, -1);
	psLambda->pcClass->mpsLambda = psLambda;
	NameLambda(psLambda);
	psLambda->pcClass->mpcBindings = psLambda->pcBindings;
	mapsLambdas.Add(psLambda);

	psLambda->bGeneric = false;
	pcSuffix = pcSyntax->mpcDefinition->mpcDeclarator->GetFunctionSuffix();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psLambda->bGeneric |= (pcParameter->GetKind() == CPPS_Parameter) && mpcUnit->mcDeclarators.IsAuto(pcParameter->mpcSpecifiers);
	}

	//What is said making the closure is the closure's: it is said now, and again where the closure is asked for again.
	mpcUnit->mcDiagnostics.BeginKeeping();
	psLambda->bFailed = !LambdaCaptures(psLambda) || !LayoutClosure(psLambda) || (!psLambda->bGeneric && !LambdaCall(psLambda));
	if (psLambda->bFailed && !mpcUnit->mcDiagnostics.HasKept())
	{
		//Nothing said why.
		mpcUnit->mcDiagnostics.ReportError(pcSyntax, "The lambda's closure can't be made");
	}
	psLambda->pcErrors = mpcUnit->mcDiagnostics.EndKeeping();
	if (psLambda->bFailed)
	{
		mpcUnit->mcDiagnostics.Report(psLambda->pcErrors);
		return NULL;
	}
	return psLambda;
}


//////////////////////////////////////////////////////////////////////////
//
// A lambda in a function every unit may write is named for it: its label,
// __L and the lambda's number among those in the function's definition
// (written the same in every unit), so its closure is one type in every
// unit, its functions COMDAT with the same labels.  [basic.def.odr] 6
//
//////////////////////////////////////////////////////////////////////////
void CCPPClosures::NameLambda(SCPPLambda* psLambda)
{
	size	uiNumber;

	uiNumber = 0;
	if (!(psLambda->pcEnclosing && psLambda->pcEnclosing->IsShared()) || !psLambda->pcEnclosing->mpcDefinition || !LambdaNumber(psLambda->pcEnclosing->mpcDefinition, psLambda->pcSyntax, &uiNumber))
	{
		return;
	}
	mpcUnit->mcMangler.AppendFunctionLabel(&psLambda->szName, psLambda->pcEnclosing);
	psLambda->szName.Append("__L");
	psLambda->szName.Append((int)uiNumber);
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcLambda is in pcNode, with *puiNumber the lambdas before it
// (outer ones first: in the order they are written).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::LambdaNumber(CCPPSyntax* pcNode, CCPPSyntaxLambdaExpression* pcLambda, size* puiNumber)
{
	size	i;

	if (!pcNode)
	{
		return false;
	}
	if (pcNode == pcLambda)
	{
		return true;
	}
	if (pcNode->GetKind() == CPPS_LambdaExpression)
	{
		(*puiNumber)++;
	}
	for (i = 0; i < pcNode->NumChildren(); i++)
	{
		if (LambdaNumber(pcNode->GetChild(i), pcLambda, puiNumber))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// What a lambda captures: what its captures name, and with a default
// capture what its body uses of the enclosing function's (a variable, or
// this through a member or this itself) that it doesn't name.  Without a
// default, a use of what it doesn't capture is an error.  A lambda inside
// it is its body too.  One in a default argument captures nothing.
// [expr.prim.lambda] 11, 12, 13
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::LambdaCaptures(SCPPLambda* psLambda)
{
	CCPPSyntaxLambdaCapture*	pcCapture;
	CCPPSymbol*					pcSymbol;
	SCPPFunctionPack*			psPack;
	size						i;
	bool						bResult;

	bResult = true;
	for (i = 0; i < psLambda->pcSyntax->macpCaptures.NumElements(); i++)
	{
		pcCapture = (CCPPSyntaxLambdaCapture*)psLambda->pcSyntax->macpCaptures.GetPtr(i);
		if (pcCapture->meKind == CPPLC_This)
		{
			bResult &= AddThisCapture(psLambda, pcCapture);
			continue;
		}
		if (pcCapture->mpcInitializer)
		{
			bResult &= AddCapture(psLambda, pcCapture->mpcSymbol, pcCapture->mpcInitializer, pcCapture->meKind == CPPLC_Reference, true, pcCapture);
			continue;
		}
		pcSymbol = pcCapture->mpcName ? pcCapture->mpcName->mpcName->mpcSymbol : NULL;
		if (pcCapture->mbPack)
		{
			psPack = pcSymbol ? psLambda->cFunctionPacks.Find(pcSymbol) : NULL;
			if (!psPack)
			{
				bResult &= mpcUnit->mcDiagnostics.ReportOnce(pcCapture, "Only the parameter pack of the function the lambda is in can be captured as a pack");
				continue;
			}
			bResult &= CapturePack(psLambda, psPack, pcCapture->mpcName, pcCapture->meKind == CPPLC_Reference, pcCapture);
			continue;
		}
		if (!IsEnclosingLocal(pcSymbol, psLambda))
		{
			bResult &= mpcUnit->mcDiagnostics.ReportOnce(pcCapture, "Only a variable of the function the lambda is in can be captured");
			continue;
		}
		if (FindLambdaCapture(psLambda, pcSymbol))
		{
			bResult &= mpcUnit->mcDiagnostics.ReportOnce(pcCapture, "The variable is captured twice");
			continue;
		}
		bResult &= AddCapture(psLambda, pcSymbol, pcCapture->mpcName, pcCapture->meKind == CPPLC_Reference, false, pcCapture);
	}
	bResult = bResult && UsedCaptures(psLambda, psLambda->pcSyntax->mpcDefinition->mpcBody);
	if (bResult && (psLambda->asCaptures.NumElements() > 0) && mpcUnit->DefaultArgumentOf(psLambda->pcSyntax))
	{
		//[expr.prim.lambda] 13
		return mpcUnit->mcDiagnostics.ReportOnce(psLambda->pcSyntax, "A lambda in a default argument can't capture anything");
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// What the body (pcNode) uses that the lambda must capture.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::UsedCaptures(SCPPLambda* psLambda, CCPPSyntax* pcNode)
{
	CCPPSymbol*	pcSymbol;
	SCPPFunctionPack*	psPack;
	size		i;
	bool		bResult;

	if (!pcNode || (pcNode->GetKind() == CPPS_ClassSpecifier))
	{
		return true;
	}
	bResult = true;
	if (pcNode->GetKind() == CPPS_ThisExpression)
	{
		bResult = UseThis(psLambda, pcNode);
	}
	else if (pcNode->GetKind() == CPPS_IdExpression)
	{
		pcSymbol = ((CCPPSyntaxIdExpression*)pcNode)->mpcName->mpcSymbol;
		psPack = pcSymbol ? psLambda->cFunctionPacks.Find(pcSymbol) : NULL;
		if (psPack)
		{
			//An enclosing function's parameter pack (args in f(args...)): its elements are what is captured, all of them.
			if (IsPackCaptured(psLambda, psPack))
			{
				return true;
			}
			if (psLambda->pcSyntax->meDefault == CPPLC_None)
			{
				return mpcUnit->mcDiagnostics.ReportOnce(pcNode, "The lambda doesn't capture the parameter pack");
			}
			return CapturePack(psLambda, psPack, (CCPPSyntaxIdExpression*)pcNode, psLambda->pcSyntax->meDefault == CPPLC_Reference, pcNode);
		}
		if (IsEnclosingLocal(pcSymbol, psLambda) && !FindLambdaCapture(psLambda, pcSymbol))
		{
			//A constant whose value is used at once isn't odr-used: it isn't captured, even by a default capture.
			if (!IsOdrUse(pcNode) && IsEnclosingConstant(pcSymbol, psLambda))
			{
				return true;
			}

			if (psLambda->pcSyntax->meDefault == CPPLC_None)
			{
				return mpcUnit->mcDiagnostics.ReportOnce(pcNode, "The lambda doesn't capture the variable");
			}
			return AddCapture(psLambda, pcSymbol, pcNode, psLambda->pcSyntax->meDefault == CPPLC_Reference, false, pcNode);
		}
		if (IsThisMember(pcSymbol) && (((CCPPSyntaxIdExpression*)pcNode)->mpcName->macpComponents.NumElements() == 1))
		{
			bResult = UseThis(psLambda, pcNode);
		}
	}
	for (i = 0; bResult && (i < pcNode->NumChildren()); i++)
	{
		bResult = UsedCaptures(psLambda, pcNode->GetChild(i));
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Captures each element of psPack, the parameter pack of a function the
// lambda is in, which pcName names: [args...], [&args...], or a default
// capture whose body expands it.  An element is a variable of its own (FunctionPackElement), and
// is captured as one, by a copy of the name that names it.
// [expr.prim.lambda] 24
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::CapturePack(SCPPLambda* psLambda, SCPPFunctionPack* psPack, CCPPSyntaxIdExpression* pcName, bool bReference, CCPPSyntax* pcNode)
{
	CCPPSyntaxCopier	cCopier;
	CCPPSymbol*			pcElement;
	CCPPSyntax*			pcElementName;
	size				k;
	bool				bResult;

	if (IsPackCaptured(psLambda, psPack) && (psPack->apcElements.NumElements() > 0))
	{
		return mpcUnit->mcDiagnostics.ReportOnce(pcNode, "The variable is captured twice");
	}
	bResult = true;
	for (k = 0; bResult && (k < psPack->apcElements.NumElements()); k++)
	{
		pcElement = psPack->apcElements.GetPtr(k);
		cCopier.Init(mpcUnit->mpcTree->GetMemory());
		cCopier.Substitute(psPack->pcPack, pcElement);
		pcElementName = cCopier.Copy(pcName, pcName->mpcParent);
		cCopier.Kill();
		bResult = AddCapture(psLambda, pcElement, pcElementName, bReference, false, pcNode);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the lambda has captured an enclosing function's parameter
// pack: its elements, of which an empty pack has none to capture.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::IsPackCaptured(SCPPLambda* psLambda, SCPPFunctionPack* psPack)
{
	return (psPack->apcElements.NumElements() == 0) || (FindLambdaCapture(psLambda, psPack->apcElements.GetPtr(0)) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// The body uses this (itself, or a member of the enclosing class): with a
// default capture it is captured, and without one it must have been.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::UseThis(SCPPLambda* psLambda, CCPPSyntax* pcNode)
{
	if (psLambda->bThis)
	{
		return true;
	}
	if (psLambda->pcSyntax->meDefault == CPPLC_None)
	{
		return mpcUnit->mcDiagnostics.ReportOnce(pcNode, "The lambda doesn't capture this");
	}
	return AddThisCapture(psLambda, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the symbol is a variable or parameter of a function the lambda
// is in, not of the lambda itself, with automatic storage: what can be
// captured.  [expr.prim.lambda] 10
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::IsEnclosingLocal(CCPPSymbol* pcSymbol, SCPPLambda* psLambda)
{
	CCPPScope*		pcScope;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;

	if (!pcSymbol || ((pcSymbol->meKind != CPPSK_Variable) && (pcSymbol->meKind != CPPSK_Parameter)) || !pcSymbol->mpcScope)
	{
		return false;
	}
	if ((pcSymbol->mpcScope->meKind != CPPSC_Block) && (pcSymbol->mpcScope->meKind != CPPSC_Function))
	{
		return false;
	}
	for (pcScope = pcSymbol->mpcScope; pcScope; pcScope = pcScope->mpcParent)
	{
		if (pcScope == psLambda->pcSyntax->mpcDefinition->mpcScope)
		{
			return false;
		}
	}
	pcSpecifiers = pcSymbol->DeclSpecifiers();
	return !pcSpecifiers || ((pcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_EXTERN | CPPDS_THREAD_LOCAL)) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the symbol is a variable of a function the lambda is in whose
// value is a constant: a const or constexpr integer, bool or enum, not
// volatile, initialised with a constant.  Where its value is used at once
// it isn't odr-used, and a lambda that doesn't capture it may use it.
// [basic.def.odr] 3, [expr.const] 2.7, [expr.prim.lambda] 12
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::IsEnclosingConstant(CCPPSymbol* pcSymbol, SCPPLambda* psLambda)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	int64						lliValue;

	if (!IsEnclosingLocal(pcSymbol, psLambda) || !EnclosingConstantType(pcSymbol))
	{
		return false;
	}
	pcInit = (CCPPSyntaxInitDeclarator*)pcSymbol->mpcDeclaration;
	return mpcUnit->mcFolder.Constant(pcInit->mpcInitializer, &lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// True if a variable is an automatic one of a function the one being
// written is in: of the function a local class is defined in, when one of
// the class's member functions is written.  [class.local] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::IsOuterLocal(CCPPSymbol* pcSymbol)
{
	CCPPFunction*	pcFunction;
	CCPPScope*		pcScope;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;

	if (!pcSymbol || ((pcSymbol->meKind != CPPSK_Variable) && (pcSymbol->meKind != CPPSK_Parameter)) || !pcSymbol->mpcScope)
	{
		return false;
	}
	if ((pcSymbol->mpcScope->meKind != CPPSC_Block) && (pcSymbol->mpcScope->meKind != CPPSC_Function))
	{
		return false;
	}
	pcFunction = OuterFunction();
	if (!pcFunction || !pcFunction->mpcDefinition || !pcFunction->mpcDefinition->mpcScope)
	{
		return false;
	}
	for (pcScope = pcSymbol->mpcScope; pcScope; pcScope = pcScope->mpcParent)
	{
		if (pcScope == pcFunction->mpcDefinition->mpcScope)
		{
			return false;
		}
	}
	pcSpecifiers = pcSymbol->DeclSpecifiers();
	return !pcSpecifiers || ((pcSpecifiers->muiFlags & (CPPDS_STATIC | CPPDS_EXTERN | CPPDS_THREAD_LOCAL)) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// True if a variable is a constant of a function a local class is in
// (IsOuterLocal) whose value a member function of the class may use: it
// isn't odr-used.  [basic.def.odr] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::IsOuterConstant(CCPPSymbol* pcSymbol)
{
	int64	lliValue;

	if (!IsOuterLocal(pcSymbol) || !EnclosingConstantType(pcSymbol))
	{
		return false;
	}
	return mpcUnit->mcFolder.Constant(((CCPPSyntaxInitDeclarator*)pcSymbol->mpcDeclaration)->mpcInitializer, &lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// A static local as its function last defined it: a local class's member
// function names it while its function's variables aren't the ones being
// made.  NULL for a symbol that isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPVariable* CCPPClosures::EnclosingStatic(CCPPSymbol* pcSymbol)
{
	size	i;

	for (i = mpcUnit->GetGlobals()->GetStaticLocals()->NumElements(); i > 0; i--)
	{
		if (mpcUnit->GetGlobals()->GetStaticLocals()->GetPtr(i - 1)->mpcSymbol == pcSymbol)
		{
			return mpcUnit->GetGlobals()->GetStaticLocals()->GetPtr(i - 1);
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a local constant's value (IsEnclosingConstant): its declared
// type without const.  NULL if it isn't const (or constexpr), is volatile,
// has no initialiser, or isn't an integer, bool or enum.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPClosures::EnclosingConstantType(CCPPSymbol* pcSymbol)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	SCPP65Type*					psType;

	pcInit = (CCPPSyntaxInitDeclarator*)pcSymbol->mpcDeclaration;
	if ((pcSymbol->meKind != CPPSK_Variable) || !pcInit || (pcInit->GetKind() != CPPS_InitDeclarator) || !pcInit->mpcInitializer)
	{
		return NULL;
	}
	pcDeclaration = pcInit->GetDeclaration();
	if (!pcDeclaration || !pcDeclaration->mpcSpecifiers || !(pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_CONSTEXPR)))
	{
		return NULL;
	}
	psType = mpcUnit->mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	if (!psType || psType->bVolatile || !psType->IsInteger())
	{
		return NULL;
	}
	return psType->Unqualified();
}


//////////////////////////////////////////////////////////////////////////
//
// The type a local constant (IsEnclosingConstant) is declared with, which
// is what decltype of its name in a lambda that doesn't capture it gives:
// const, as constexpr makes it too.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPClosures::EnclosingConstantObjectType(CCPPSymbol* pcSymbol)
{
	return mpcUnit->mcTypes.Qualified(EnclosingConstantType(pcSymbol), true, false);
}


//////////////////////////////////////////////////////////////////////////
//
// True if a name (pcNode, a variable's) is odr-used where it is because
// its object is wanted, not its value: it is the operand of & or what a
// reference is bound to in its declaration.  A reference parameter it is
// an argument for is found when its place is asked for (the type checker's
// Place).  [basic.def.odr] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::IsOdrUse(CCPPSyntax* pcNode)
{
	CCPPSyntax*						pcParent;
	CCPPSyntaxUnaryExpression*		pcUnary;
	CCPPSyntaxInitDeclarator*		pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	SCPP65Type*						psType;

	pcParent = pcNode->mpcParent;
	while (pcParent && (pcParent->GetKind() == CPPS_ParenthesisedExpression))
	{
		pcNode = pcParent;
		pcParent = pcParent->mpcParent;
	}
	if (!pcParent)
	{
		return false;
	}
	if (pcParent->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcParent;
		return (pcUnary->meOperator == CT_BitwiseAnd) && (pcUnary->mpcOperand == pcNode);
	}
	if (pcParent->GetKind() == CPPS_InitDeclarator)
	{
		pcInit = (CCPPSyntaxInitDeclarator*)pcParent;
		pcDeclaration = pcInit->GetDeclaration();
		if ((pcInit->mpcInitializer != pcNode) || !pcDeclaration)
		{
			return false;
		}
		psType = mpcUnit->mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
		return psType && psType->IsReference();
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the symbol is a member of a class that isn't static: a name of
// it alone, in a member function, is this's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::IsThisMember(CCPPSymbol* pcSymbol)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;

	if (!pcSymbol || ((pcSymbol->meKind != CPPSK_Field) && (pcSymbol->meKind != CPPSK_Function)) || !pcSymbol->mpcScope || (pcSymbol->mpcScope->meKind != CPPSC_Class))
	{
		return false;
	}
	pcSpecifiers = pcSymbol->DeclSpecifiers();
	return !pcSpecifiers || !(pcSpecifiers->muiFlags & CPPDS_STATIC);
}


//////////////////////////////////////////////////////////////////////////
//
// The capture of the symbol (NULL: of this) the lambda has, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPPLambdaCapture* CCPPClosures::FindLambdaCapture(SCPPLambda* psLambda, CCPPSymbol* pcSymbol)
{
	SCPPLambdaCapture*	psCapture;
	size				i;

	for (i = 0; i < psLambda->asCaptures.NumElements(); i++)
	{
		psCapture = psLambda->asCaptures.Get(i);
		if (pcSymbol ? (psCapture->pcSymbol == pcSymbol) : psCapture->bThis)
		{
			return psCapture;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// A capture of a variable or an init-capture (bOwn), and its field: of
// the variable's type (a reference's, what it refers to; an array's, the
// array) or an init-capture's (what auto is from its initialiser), or a
// reference to it.  Worked out where the lambda is.
// [expr.prim.lambda] 11, 15, 16
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::AddCapture(SCPPLambda* psLambda, CCPPSymbol* pcSymbol, CCPPSyntax* pcInitialiser, bool bReference, bool bOwn, CCPPSyntax* pcNode)
{
	SCPPLambdaCapture*	psCapture;
	SCPP65Type*			psType;
	SCPP65Type*			psObject;

	if (bReference || !bOwn)
	{
		psType = mpcUnit->GetTypeChecker()->PlaceOf(pcInitialiser);
		if (!psType)
		{
			mpcUnit->GetTypeChecker()->ReportTypeOf(pcInitialiser);
			return mpcUnit->mcDiagnostics.ReportOnce(pcNode, "What the lambda captures can't be worked out");
		}
		psType = bReference ? mpcUnit->mcTypes.Reference(psType) : psType;
	}
	else
	{
		psType = mpcUnit->mcDeclarators.DeduceAuto(psLambda->pcSyntax->mpcDefinition->mpcSpecifiers, NULL, pcInitialiser, pcNode);
		if (!psType)
		{
			return false;
		}
	}
	for (psObject = psType; psObject->IsArray(); psObject = psObject->psBase)
	{
	}
	if (psObject->IsClass() && !mpcUnit->CompleteObjectClass(psObject->pcClass, pcNode))
	{
		return false;
	}

	psCapture = psLambda->asCaptures.Add();
	psCapture->pcSymbol = pcSymbol;
	psCapture->pcInitialiser = pcInitialiser;
	psCapture->uiField = AddClosureField(psLambda, psType);
	psCapture->bReference = bReference;
	psCapture->bThis = false;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The capture of this: a field that is the enclosing object's this.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::AddThisCapture(SCPPLambda* psLambda, CCPPSyntax* pcNode)
{
	SCPPLambdaCapture*	psCapture;
	CCPPClass*			pcThisClass;

	if (psLambda->bThis)
	{
		return true;
	}
	pcThisClass = mpcUnit->mcContext.HasThis() ? ThisClass() : NULL;
	if (!pcThisClass)
	{
		return mpcUnit->mcDiagnostics.ReportOnce(pcNode, "Only a lambda in a member function can capture this");
	}
	psCapture = psLambda->asCaptures.Add();
	psCapture->pcSymbol = NULL;
	psCapture->pcInitialiser = NULL;
	psCapture->uiField = AddClosureField(psLambda, mpcUnit->ThisType(pcThisClass));
	psCapture->bReference = false;
	psCapture->bThis = true;
	psLambda->bThis = true;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A field of the closure, of the type.  It has no name: only its capture
// knows it.
//
//////////////////////////////////////////////////////////////////////////
size CCPPClosures::AddClosureField(SCPPLambda* psLambda, SCPP65Type* psType)
{

	psLambda->pcClass->macFields.Add()->Init(NULL, psType);
	return psLambda->pcClass->macFields.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays the closure's fields out, in the order captured, as a class's are
// (CompleteMembers, PlaceMembers): it gets the destructor its fields need.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::LayoutClosure(SCPPLambda* psLambda)
{
	CCPPClass*	pcClass;

	pcClass = psLambda->pcClass;
	pcClass->meState = CPPCS_LayingOut;
	mpcUnit->CompleteMembers(pcClass);
	mpcUnit->PlaceMembers(pcClass, 0, 0);
	pcClass->meState = CPPCS_Complete;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator() of a lambda that isn't generic, and for one that captures
// nothing the conversion to a pointer to a function (AddLambdaConversion).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::LambdaCall(SCPPLambda* psLambda)
{
	CArrayCPP65Types			apsParameters;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	SCPP65Type*					psType;
	size						i;
	bool						bResult;

	apsParameters.Init();
	pcSuffix = psLambda->pcSyntax->mpcDefinition->mpcDeclarator->GetFunctionSuffix();
	bResult = true;
	for (i = 0; bResult && (i < pcSuffix->macpParameters.NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = (pcParameter->GetKind() == CPPS_Parameter) ? mpcUnit->mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter) : NULL;
		if (psType && psType->IsVoid() && (pcSuffix->macpParameters.NumElements() == 1))
		{
			break;
		}
		bResult = psType != NULL;
		if (bResult)
		{
			apsParameters.Add(mpcUnit->GetFunctionDeclarer()->ParameterType(psType));
		}
	}
	psLambda->pcCall = bResult ? LambdaFunction(psLambda, &apsParameters) : NULL;
	if (psLambda->pcCall)
	{
		AddLambdaConversion(psLambda, psLambda->pcCall);
	}
	apsParameters.Kill();
	return psLambda->pcCall != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The conversion of a closure that captures nothing to a pointer to a
// function of its operator()'s type (pcCall): a static function with the
// same body (the invoker), whose address the conversion gives.
// [expr.prim.lambda] 6
//
//////////////////////////////////////////////////////////////////////////
void CCPPClosures::AddLambdaConversion(SCPPLambda* psLambda, CCPPFunction* pcCall)
{
	CCPPFunction*	pcInvoker;
	CCPPFunction*	pcConversion;

	if ((psLambda->asCaptures.NumElements() != 0) || (pcCall->mpsReturn == &mpcUnit->mcTypes.msUndeduced))
	{
		return;
	}
	pcInvoker = AddLambdaFunction(psLambda, mpcUnit->mpcTokeniser->GetIdentifierID((char*)"__invoke"), &pcCall->mapsParameters, pcCall->mpsReturn);
	pcInvoker->mbStatic = true;
	pcConversion = mpcUnit->GetFunctionDeclarer()->AddImplicit(psLambda->pcClass, CPP_CONVERSION_NAME, CPPFK_Function, mpcUnit->mcTypes.Pointer(mpcUnit->mcTypes.Function(pcCall->mpsReturn, &pcCall->mapsParameters, false)), true);
	pcConversion->mbConstThis = true;
	pcConversion->mpcAddressOf = pcInvoker;
}


//////////////////////////////////////////////////////////////////////////
//
// An operator() of the closure for parameters of these types, its return
// type worked out from its body as auto's is.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPClosures::LambdaFunction(SCPPLambda* psLambda, CArrayCPP65Types* papsParameters)
{
	CCPPFunction*					pcFunction;
	CCPPSyntaxFunctionDefinition*	pcDefinition;

	pcDefinition = psLambda->pcSyntax->mpcDefinition;
	pcFunction = AddLambdaFunction(psLambda, mpcUnit->GetOperatorName(CT_OpenRoundBracket, false), papsParameters, &mpcUnit->mcTypes.msUndeduced);
	pcFunction->mbConstThis = !psLambda->bMutable;
	if (!mpcUnit->GetFunctionDeclarer()->ResolveReturn(pcFunction, pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator))
	{
		return NULL;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A function of the closure defined by the lambda's body, with the default
// arguments the lambda's parameters have.  [expr.prim.lambda] 5
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPClosures::AddLambdaFunction(SCPPLambda* psLambda, int64 lliName, CArrayCPP65Types* papsParameters, SCPP65Type* psReturn)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntax*					pcParameter;
	CCPPFunction*				pcFunction;
	size						i;

	pcSuffix = psLambda->pcSyntax->mpcDefinition->mpcDeclarator->GetFunctionSuffix();
	pcFunction = mpcUnit->AddFunction(psLambda->pcClass, lliName, CPPFK_Function, psReturn);
	for (i = 0; i < papsParameters->NumElements(); i++)
	{
		pcParameter = (i < pcSuffix->macpParameters.NumElements()) ? pcSuffix->macpParameters.GetPtr(i) : NULL;
		pcFunction->mapsParameters.Add(papsParameters->GetPtr(i));
		pcFunction->mapcDefaults.Add((pcParameter && (pcParameter->GetKind() == CPPS_Parameter)) ? ((CCPPSyntaxParameter*)pcParameter)->mpcDefault : NULL);
	}
	pcFunction->mpcDefinition = psLambda->pcSyntax->mpcDefinition;
	pcFunction->mpcDeclarator = psLambda->pcSyntax->mpcDefinition->mpcDeclarator;
	pcFunction->mpcBindings = psLambda->pcBindings;
	//[]() noexcept { }: the lambda's declarator is its operator()'s.  [expr.prim.lambda] 5
	mpcUnit->GetExceptions()->SetDeclaredExceptions(pcFunction, pcFunction->mpcDeclarator);
	pcFunction->mpcNamespace = psLambda->pcEnclosing ? psLambda->pcEnclosing->mpcNamespace : mpcUnit->mcContext.GetNamespace();
	pcFunction->mbDefined = true;
	pcFunction->mbInline = true;
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator() of a generic lambda for a call's arguments: each auto
// parameter's type is what auto is from its argument, as from a variable's
// initialiser; a parameter the call gives no argument takes its default,
// which says nothing of what auto is ([temp.deduct.type] 5).  Made once for
// each set of types, as a template's instance is.  [expr.prim.lambda] 5
//
//////////////////////////////////////////////////////////////////////////
void CCPPClosures::GenericLambdaCall(SCPPLambda* psLambda, CArraySyntax* papcArguments)
{
	CArrayCPP65Types			apsParameters;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPSyntax*					pcArgument;
	SCPP65Type*					psType;
	size						uiParameters;
	size						uiTypes;
	size						i;

	pcSuffix = psLambda->pcSyntax->mpcDefinition->mpcDeclarator->GetFunctionSuffix();
	if (!pcSuffix->TakesArgumentCount(papcArguments->NumElements()) || pcSuffix->mbVariadic)
	{
		return;
	}

	//Its parameters: those declared, or with a last one that is a pack (auto... args) one for each argument.
	uiParameters = pcSuffix->NumParameters();
	uiTypes = pcSuffix->EndsWithPack() ? ((papcArguments->NumElements() + 1 > uiParameters) ? papcArguments->NumElements() : uiParameters - 1) : uiParameters;
	apsParameters.Init();
	for (i = 0; i < uiTypes; i++)
	{
		pcParameter = pcSuffix->ArgumentParameter(i);
		pcArgument = (i < papcArguments->NumElements()) ? papcArguments->GetPtr(i) : NULL;
		psType = NULL;
		if ((pcParameter->GetKind() == CPPS_Parameter) && (pcArgument || !mpcUnit->mcDeclarators.IsAuto(pcParameter->mpcSpecifiers)))
		{
			psType = mpcUnit->mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter, pcArgument);
		}
		if (!psType)
		{
			apsParameters.Kill();
			return;
		}
		apsParameters.Add(mpcUnit->GetFunctionDeclarer()->ParameterType(psType));
	}

	GenericLambdaInstance(psLambda, &apsParameters);
	apsParameters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// A generic lambda's operator() for parameters of these types: the one made
// for them before, or a new one.  NULL if its body is wrong for them.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPClosures::GenericLambdaInstance(SCPPLambda* psLambda, CArrayCPP65Types* papsParameters)
{
	CCPPFunction*	pcFunction;
	size			i;

	for (i = 0; i < psLambda->pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = psLambda->pcClass->mapcMembers.GetPtr(i);
		if ((pcFunction->mlliName != mpcUnit->GetOperatorName(CT_OpenRoundBracket, false)) || (pcFunction->mapsParameters.NumElements() != papsParameters->NumElements()))
		{
			continue;
		}
		if (pcFunction->TakesParameters(papsParameters))
		{
			return pcFunction;
		}
	}
	return LambdaFunction(psLambda, papsParameters);
}


//////////////////////////////////////////////////////////////////////////
//
// A generic lambda that captures nothing converts to a pointer to a function
// (psFunction, its type) as a function template is deduced from one: its
// operator() for the function's parameters, where each parameter's
// declaration takes that type (auto anything, auto& a reference, auto* a
// pointer, a type that isn't auto itself; a last one that is a pack, auto...
// args, takes each of the function's from its place on), has a conversion
// to a pointer to it if it returns what the function does.  Made once for
// each type; nothing is made, quietly, when it doesn't fit.
// [expr.prim.lambda] 6
//
//////////////////////////////////////////////////////////////////////////
void CCPPClosures::GenericLambdaConversion(SCPPLambda* psLambda, SCPP65Type* psFunction)
{
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPFunction*				pcCall;
	CCPPFunction*				pcConversion;
	size						uiTypes;
	size						i;

	pcSuffix = psLambda->pcSyntax->mpcDefinition->mpcDeclarator->GetFunctionSuffix();
	uiTypes = psFunction->papsParameters->NumElements();
	if ((psLambda->asCaptures.NumElements() != 0) || psFunction->bVariadic || (pcSuffix->EndsWithPack() ? (uiTypes + 1 < pcSuffix->NumParameters()) : (uiTypes != pcSuffix->NumParameters())))
	{
		return;
	}
	for (i = 0; i < uiTypes; i++)
	{
		if (!ParameterTakesType(pcSuffix->ArgumentParameter(i), psFunction->papsParameters->GetPtr(i)))
		{
			return;
		}
	}
	for (pcConversion = mpcUnit->FirstFunctionOfName(CPP_CONVERSION_NAME); pcConversion; pcConversion = pcConversion->mpcNextOfName)
	{
		if ((pcConversion->mpcClass == psLambda->pcClass) && pcConversion->mpsReturn->psBase->Same(psFunction))
		{
			return;
		}
	}
	mpcUnit->mcDiagnostics.BeginQuiet();
	pcCall = GenericLambdaInstance(psLambda, psFunction->papsParameters);
	mpcUnit->mcDiagnostics.EndQuiet();
	if (pcCall && pcCall->mpsReturn->Same(psFunction->psBase))
	{
		AddLambdaConversion(psLambda, pcCall);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a generic lambda's parameter, as it is declared, is of psType
// for some type auto can be: one that isn't auto is its own type, auto alone
// is any, and auto with a declarator is a reference or a pointer as its
// last operator says (const auto& one to const).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClosures::ParameterTakesType(CCPPSyntaxParameter* pcParameter, SCPP65Type* psType)
{
	SCPP65Type*	psDeclared;
	size		uiPointers;
	bool		bConst;

	if (pcParameter->GetKind() != CPPS_Parameter)
	{
		return false;
	}
	if (!mpcUnit->mcDeclarators.IsAuto(pcParameter->mpcSpecifiers))
	{
		psDeclared = mpcUnit->mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter);
		return psDeclared && mpcUnit->GetFunctionDeclarer()->ParameterType(psDeclared)->Same(psType);
	}
	uiPointers = pcParameter->mpcDeclarator ? pcParameter->mpcDeclarator->NumPointers() : 0;
	if (uiPointers == 0)
	{
		return true;
	}
	bConst = (uiPointers == 1) && ((pcParameter->mpcSpecifiers->muiFlags & CPPDS_CONST) != 0);
	switch (pcParameter->mpcDeclarator->GetPointer(uiPointers - 1)->meKind)
	{
	case CPPP_Reference:
		return psType->IsReference() && !psType->bRvalue && (!bConst || psType->psBase->bConst);
	case CPPP_RValueReference:
		//auto&& is a reference to what has a name too.
		return psType->IsReference() && (!bConst || (psType->bRvalue && psType->psBase->bConst));
	case CPPP_Pointer:
		return psType->IsPointer() && (!bConst || psType->psBase->bConst);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The closure whose operator() (or invoker) is being written, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPPLambda* CCPPClosures::WritingLambda(void)
{
	return BodyLambda(mpcUnit->mcContext.GetFunction());
}


//////////////////////////////////////////////////////////////////////////
//
// The lambda whose body the function is (its closure's operator() or
// invoker), or NULL: the closure's destructor and conversion are its own.
//
//////////////////////////////////////////////////////////////////////////
SCPPLambda* CCPPClosures::BodyLambda(CCPPFunction* pcFunction)
{
	SCPPLambda*	psLambda;

	psLambda = (pcFunction && pcFunction->mpcClass) ? pcFunction->mpcClass->mpsLambda : NULL;
	return (psLambda && (pcFunction->mpcDefinition == psLambda->pcSyntax->mpcDefinition)) ? psLambda : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What a name in the body of the lambda being written is, if the lambda
// captures it: its capture (the closure's field).
//
//////////////////////////////////////////////////////////////////////////
SCPPLambdaCapture* CCPPClosures::FindCapture(CCPPSymbol* pcSymbol)
{
	SCPPLambda*	psLambda;

	psLambda = WritingLambda();
	return (psLambda && pcSymbol) ? FindLambdaCapture(psLambda, pcSymbol) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The function whose body the one being written is in: itself, or for a
// lambda's operator() the function the lambda is in (and out through
// lambdas in lambdas).  Names are looked up, and access is decided, as in
// it.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPClosures::OuterFunction(void)
{
	CCPPFunction*	pcFunction;

	pcFunction = mpcUnit->mcContext.GetFunction();
	while (BodyLambda(pcFunction))
	{
		pcFunction = BodyLambda(pcFunction)->pcEnclosing;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The class this is an object of where the function being written is: its
// own, or in a lambda that captures this the enclosing member function's.
// NULL where there is no this.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPClosures::ThisClass(void)
{
	CCPPFunction*	pcFunction;

	if (mpcUnit->mcContext.GetObject())
	{
		//An aggregate being filled is this for its default member initialisers.
		return mpcUnit->mcContext.GetObject();
	}
	pcFunction = mpcUnit->mcContext.GetFunction();
	while (BodyLambda(pcFunction))
	{
		if (!BodyLambda(pcFunction)->bThis)
		{
			return NULL;
		}
		pcFunction = BodyLambda(pcFunction)->pcEnclosing;
	}
	return (pcFunction && !pcFunction->mbStatic) ? pcFunction->mpcClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a lambda expression: its closure's class.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPClosures::LambdaType(CCPPSyntaxLambdaExpression* pcSyntax)
{
	SCPPLambda*	psLambda;

	psLambda = GetLambda(pcSyntax);
	return psLambda ? psLambda->pcClass->mpsType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPClosures::FreeLambdas(void)
{
	size	i;

	for (i = 0; i < mapsLambdas.NumElements(); i++)
	{
		mapsLambdas.GetPtr(i)->asCaptures.Kill();
		mapsLambdas.GetPtr(i)->cFunctionPacks.Kill();
		mapsLambdas.GetPtr(i)->szName.Kill();
		free(mapsLambdas.GetPtr(i));
	}
	mapsLambdas.Kill();
}

