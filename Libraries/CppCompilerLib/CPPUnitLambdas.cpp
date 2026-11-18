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
#include "CPPSyntaxes.h"


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
SCPPLambda* CCPPUnit::GetLambda(CCPPSyntaxLambdaExpression* pcSyntax)
{
	SCPPLambda*					psLambda;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	size						i;

	for (i = 0; i < mapsLambdas.NumElements(); i++)
	{
		psLambda = mapsLambdas.GetPtr(i);
		if ((psLambda->pcSyntax == pcSyntax) && (psLambda->pcEnclosing == mcContext.GetFunction()) && (psLambda->pcBindings == mcContext.GetBindings()))
		{
			if (psLambda->bFailed)
			{
				ReportOnce(psLambda->pcErrorNode, psLambda->szError.Text());
				return NULL;
			}
			return psLambda;
		}
	}

	psLambda = NewMalloc<SCPPLambda>();
	psLambda->pcSyntax = pcSyntax;
	psLambda->pcEnclosing = mcContext.GetFunction();
	psLambda->pcBindings = mcContext.GetBindings();
	psLambda->asCaptures.Init();
	psLambda->bThis = false;
	psLambda->bMutable = pcSyntax->IsMutable();
	psLambda->bFailed = false;
	psLambda->pcErrorNode = pcSyntax;
	psLambda->szError.Init();
	psLambda->pcCall = NULL;
	psLambda->pcClass = NewClass(NULL, -1);
	psLambda->pcClass->mpsLambda = psLambda;
	psLambda->pcClass->mpcBindings = psLambda->pcBindings;
	mapsLambdas.Add(psLambda);

	psLambda->bGeneric = false;
	pcSuffix = pcSyntax->mpcDefinition->mpcDeclarator->GetFunctionSuffix();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psLambda->bGeneric |= (pcParameter->GetKind() == CPPS_Parameter) && mcDeclarators.IsAuto(pcParameter->mpcSpecifiers);
	}

	if (!LambdaCaptures(psLambda) || !LayoutClosure(psLambda) || (!psLambda->bGeneric && !LambdaCall(psLambda)))
	{
		LambdaError(psLambda, pcSyntax, "The lambda's closure can't be made");
		psLambda->bFailed = true;
		return NULL;
	}
	return psLambda;
}


//////////////////////////////////////////////////////////////////////////
//
// An error in making a lambda's closure: the first is the one kept, and
// said again where the closure is asked for again.  Always false.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::LambdaError(SCPPLambda* psLambda, CCPPSyntax* pcNode, const char* szMessage)
{
	if (psLambda->szError.Empty())
	{
		psLambda->pcErrorNode = pcNode;
		psLambda->szError.Append(szMessage);
	}
	return ReportOnce(pcNode, szMessage);
}


//////////////////////////////////////////////////////////////////////////
//
// What a lambda captures: what its captures name, and with a default
// capture what its body uses of the enclosing function's (a variable, or
// this through a member or this itself) that it doesn't name.  Without a
// default, a use of what it doesn't capture is an error.  A lambda inside
// it is its body too.  [expr.prim.lambda] 11, 12
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::LambdaCaptures(SCPPLambda* psLambda)
{
	CCPPSyntaxLambdaCapture*	pcCapture;
	CCPPSymbol*					pcSymbol;
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
		if (!IsEnclosingLocal(pcSymbol, psLambda))
		{
			bResult &= LambdaError(psLambda, pcCapture, "Only a variable of the function the lambda is in can be captured");
			continue;
		}
		if (FindLambdaCapture(psLambda, pcSymbol))
		{
			bResult &= LambdaError(psLambda, pcCapture, "The variable is captured twice");
			continue;
		}
		bResult &= AddCapture(psLambda, pcSymbol, pcCapture->mpcName, pcCapture->meKind == CPPLC_Reference, false, pcCapture);
	}
	return bResult && UsedCaptures(psLambda, psLambda->pcSyntax->mpcDefinition->mpcBody);
}


//////////////////////////////////////////////////////////////////////////
//
// What the body (pcNode) uses that the lambda must capture.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::UsedCaptures(SCPPLambda* psLambda, CCPPSyntax* pcNode)
{
	CCPPSymbol*	pcSymbol;
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
		if (IsEnclosingLocal(pcSymbol, psLambda) && !FindLambdaCapture(psLambda, pcSymbol))
		{
			//A constant whose value is used at once isn't odr-used: it isn't captured, even by a default capture.
			if (!IsOdrUse(pcNode) && IsEnclosingConstant(pcSymbol, psLambda))
			{
				return true;
			}

			if (psLambda->pcSyntax->meDefault == CPPLC_None)
			{
				return LambdaError(psLambda, pcNode, "The lambda doesn't capture the variable");
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
// The body uses this (itself, or a member of the enclosing class): with a
// default capture it is captured, and without one it must have been.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::UseThis(SCPPLambda* psLambda, CCPPSyntax* pcNode)
{
	if (psLambda->bThis)
	{
		return true;
	}
	if (psLambda->pcSyntax->meDefault == CPPLC_None)
	{
		return LambdaError(psLambda, pcNode, "The lambda doesn't capture this");
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
bool CCPPUnit::IsEnclosingLocal(CCPPSymbol* pcSymbol, SCPPLambda* psLambda)
{
	CCPPScope*		pcScope;
	CCPPSyntax*		pcDeclaration;
	uint32			uiFlags;

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
	pcDeclaration = pcSymbol->mpcDeclaration;
	uiFlags = 0;
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator) && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_SimpleDeclaration) && ((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers)
	{
		uiFlags = ((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers->muiFlags;
	}
	return (uiFlags & (CPPDS_STATIC | CPPDS_EXTERN | CPPDS_THREAD_LOCAL)) == 0;
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
bool CCPPUnit::IsEnclosingConstant(CCPPSymbol* pcSymbol, SCPPLambda* psLambda)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	int64						lliValue;

	if (!IsEnclosingLocal(pcSymbol, psLambda) || !EnclosingConstantType(pcSymbol))
	{
		return false;
	}
	pcInit = (CCPPSyntaxInitDeclarator*)pcSymbol->mpcDeclaration;
	return Constant(pcInit->mpcInitializer, &lliValue);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a local constant's value (IsEnclosingConstant): its declared
// type without const.  NULL if it isn't const (or constexpr), is volatile,
// has no initialiser, or isn't an integer, bool or enum.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::EnclosingConstantType(CCPPSymbol* pcSymbol)
{
	CCPPSyntaxInitDeclarator*	pcInit;
	CCPPSyntaxSimpleDeclaration*	pcDeclaration;
	SCPP65Type*					psType;

	pcInit = (CCPPSyntaxInitDeclarator*)pcSymbol->mpcDeclaration;
	if ((pcSymbol->meKind != CPPSK_Variable) || !pcInit || (pcInit->GetKind() != CPPS_InitDeclarator) || !pcInit->mpcInitializer)
	{
		return NULL;
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
	if (!pcDeclaration || (pcDeclaration->GetKind() != CPPS_SimpleDeclaration) || !pcDeclaration->mpcSpecifiers || !(pcDeclaration->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_CONSTEXPR)))
	{
		return NULL;
	}
	psType = mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	if (!psType || psType->bVolatile || !psType->IsInteger())
	{
		return NULL;
	}
	return psType->Unqualified();
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
bool CCPPUnit::IsOdrUse(CCPPSyntax* pcNode)
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
		pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
		if ((pcInit->mpcInitializer != pcNode) || !pcDeclaration || (pcDeclaration->GetKind() != CPPS_SimpleDeclaration))
		{
			return false;
		}
		psType = mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
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
bool CCPPUnit::IsThisMember(CCPPSymbol* pcSymbol)
{
	CCPPSyntax*					pcDeclaration;
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;

	if (!pcSymbol || ((pcSymbol->meKind != CPPSK_Field) && (pcSymbol->meKind != CPPSK_Function)) || !pcSymbol->mpcScope || (pcSymbol->mpcScope->meKind != CPPSC_Class))
	{
		return false;
	}
	pcDeclaration = pcSymbol->mpcDeclaration;
	pcSpecifiers = NULL;
	if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_InitDeclarator) && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_SimpleDeclaration))
	{
		pcSpecifiers = ((CCPPSyntaxSimpleDeclaration*)pcDeclaration->mpcParent)->mpcSpecifiers;
	}
	else if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_FunctionDefinition))
	{
		pcSpecifiers = ((CCPPSyntaxFunctionDefinition*)pcDeclaration)->mpcSpecifiers;
	}
	return !pcSpecifiers || !(pcSpecifiers->muiFlags & CPPDS_STATIC);
}


//////////////////////////////////////////////////////////////////////////
//
// The capture of the symbol (NULL: of this) the lambda has, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPPLambdaCapture* CCPPUnit::FindLambdaCapture(SCPPLambda* psLambda, CCPPSymbol* pcSymbol)
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
// the variable's type (a reference's, what it refers to) or an init-
// capture's (what auto is from its initialiser), or a reference to it.
// Worked out where the lambda is.  [expr.prim.lambda] 11, 15, 16
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::AddCapture(SCPPLambda* psLambda, CCPPSymbol* pcSymbol, CCPPSyntax* pcInitialiser, bool bReference, bool bOwn, CCPPSyntax* pcNode)
{
	SCPPLambdaCapture*	psCapture;
	SCPP65Type*			psType;

	if (bReference || !bOwn)
	{
		psType = PlaceOf(pcInitialiser);
		if (!psType)
		{
			ReportTypeOf(pcInitialiser);
			return LambdaError(psLambda, pcNode, "What the lambda captures can't be worked out");
		}
		psType = bReference ? mcTypes.Reference(psType) : psType;
	}
	else
	{
		psType = mcDeclarators.DeduceAuto(psLambda->pcSyntax->mpcDefinition->mpcSpecifiers, NULL, pcInitialiser, pcNode);
		if (!psType)
		{
			return false;
		}
	}
	if (psType->IsArray())
	{
		return LambdaError(psLambda, pcNode, "Capturing an array is not supported by the compiler yet");
	}
	if (psType->IsClass() && !CompleteObjectClass(psType->pcClass, pcNode))
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
bool CCPPUnit::AddThisCapture(SCPPLambda* psLambda, CCPPSyntax* pcNode)
{
	SCPPLambdaCapture*	psCapture;
	CCPPClass*			pcThisClass;

	if (psLambda->bThis)
	{
		return true;
	}
	pcThisClass = mcContext.HasThis() ? ThisClass() : NULL;
	if (!pcThisClass)
	{
		return LambdaError(psLambda, pcNode, "Only a lambda in a member function can capture this");
	}
	psCapture = psLambda->asCaptures.Add();
	psCapture->pcSymbol = NULL;
	psCapture->pcInitialiser = NULL;
	psCapture->uiField = AddClosureField(psLambda, mcTypes.Pointer(mcTypes.Qualified(pcThisClass->mpsType, IsConstThis(), false)));
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
size CCPPUnit::AddClosureField(SCPPLambda* psLambda, SCPP65Type* psType)
{
	SCPPField*	psField;

	psField = psLambda->pcClass->macFields.Add();
	memset(psField, 0, sizeof(SCPPField));
	psField->lliName = -1;
	psField->psType = psType;
	psField->iBitWidth = -1;
	return psLambda->pcClass->macFields.NumElements() - 1;
}


//////////////////////////////////////////////////////////////////////////
//
// Lays the closure's fields out, in the order captured, as a class's are;
// it gets the destructor its fields need.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::LayoutClosure(SCPPLambda* psLambda)
{
	CCPPClass*			pcClass;
	CCPPFunction*		pcFunction;
	CCPPClassLayout		cLayout;

	pcClass = psLambda->pcClass;
	pcClass->meState = CPPCS_LayingOut;
	if (pcClass->NeedsDestructor())
	{
		pcFunction = AddFunction(pcClass, pcClass->mlliName, CPPFK_Destructor, &mcTypes.msVoid);
		pcFunction->mbImplicit = true;
		pcClass->mpcDestructor = pcFunction;
	}
	pcClass->mcVtable.LayoutOwn(pcClass);
	cLayout.Init(pcClass, 0, mpcTarget->GetPointerBytes());
	cLayout.Layout(0);
	mcTypes.SetClassBytes(pcClass->mpsType, cLayout.GetBytes());
	cLayout.Kill();
	pcClass->meState = CPPCS_Complete;
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator() of a lambda that isn't generic, and for one that captures
// nothing the conversion to a pointer to a function: a static function
// with the same body (the invoker), whose address the conversion gives.
// [expr.prim.lambda] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::LambdaCall(SCPPLambda* psLambda)
{
	CArrayCPP65Types			apsParameters;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPFunction*				pcInvoker;
	CCPPFunction*				pcConversion;
	SCPP65Type*					psType;
	size						i;
	bool						bResult;

	apsParameters.Init();
	pcSuffix = psLambda->pcSyntax->mpcDefinition->mpcDeclarator->GetFunctionSuffix();
	bResult = true;
	for (i = 0; bResult && (i < pcSuffix->macpParameters.NumElements()); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = (pcParameter->GetKind() == CPPS_Parameter) ? mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter) : NULL;
		if (psType && psType->IsVoid() && (pcSuffix->macpParameters.NumElements() == 1))
		{
			break;
		}
		bResult = psType != NULL;
		if (bResult)
		{
			apsParameters.Add(ParameterType(psType));
		}
	}
	psLambda->pcCall = bResult ? LambdaFunction(psLambda, &apsParameters) : NULL;
	if (psLambda->pcCall && (psLambda->asCaptures.NumElements() == 0) && (psLambda->pcCall->mpsReturn != &mcTypes.msUndeduced))
	{
		pcInvoker = AddLambdaFunction(psLambda, mpcTokeniser->GetIdentifierID((char*)"__invoke"), &apsParameters, psLambda->pcCall->mpsReturn);
		pcInvoker->mbStatic = true;
		pcConversion = AddFunction(psLambda->pcClass, CPP_CONVERSION_NAME, CPPFK_Function, mcTypes.Pointer(mcTypes.Function(psLambda->pcCall->mpsReturn, &apsParameters, false)));
		pcConversion->mbImplicit = true;
		pcConversion->mbDefined = true;
		pcConversion->mbConstThis = true;
		pcConversion->mpcAddressOf = pcInvoker;
	}
	apsParameters.Kill();
	return psLambda->pcCall != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An operator() of the closure for parameters of these types, its return
// type worked out from its body as auto's is.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::LambdaFunction(SCPPLambda* psLambda, CArrayCPP65Types* papsParameters)
{
	CCPPFunction*					pcFunction;
	CCPPSyntaxFunctionDefinition*	pcDefinition;

	pcDefinition = psLambda->pcSyntax->mpcDefinition;
	pcFunction = AddLambdaFunction(psLambda, GetOperatorName(CT_OpenRoundBracket, false), papsParameters, &mcTypes.msUndeduced);
	pcFunction->mbConstThis = !psLambda->bMutable;
	if (!ResolveReturn(pcFunction, pcDefinition->mpcSpecifiers, pcDefinition->mpcDeclarator))
	{
		return NULL;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// A function of the closure defined by the lambda's body.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::AddLambdaFunction(SCPPLambda* psLambda, int64 lliName, CArrayCPP65Types* papsParameters, SCPP65Type* psReturn)
{
	CCPPFunction*	pcFunction;
	size			i;

	pcFunction = AddFunction(psLambda->pcClass, lliName, CPPFK_Function, psReturn);
	for (i = 0; i < papsParameters->NumElements(); i++)
	{
		pcFunction->mapsParameters.Add(papsParameters->GetPtr(i));
		pcFunction->mapcDefaults.Add(NULL);
	}
	pcFunction->mpcDefinition = psLambda->pcSyntax->mpcDefinition;
	pcFunction->mpcDeclarator = psLambda->pcSyntax->mpcDefinition->mpcDeclarator;
	pcFunction->mpcBindings = psLambda->pcBindings;
	pcFunction->mpcNamespace = psLambda->pcEnclosing ? psLambda->pcEnclosing->mpcNamespace : mcContext.GetNamespace();
	pcFunction->mbDefined = true;
	pcFunction->mbInline = true;
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// The operator() of a generic lambda for a call's arguments: each auto
// parameter's type is what auto is from its argument, as from a variable's
// initialiser.  Made once for each set of types, as a template's instance
// is.  [expr.prim.lambda] 5
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::GenericLambdaCall(SCPPLambda* psLambda, CArraySyntax* papcArguments)
{
	CArrayCPP65Types			apsParameters;
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxParameter*		pcParameter;
	CCPPFunction*				pcFunction;
	SCPP65Type*					psType;
	size						i;
	size						j;
	bool						bSame;

	pcSuffix = psLambda->pcSyntax->mpcDefinition->mpcDeclarator->GetFunctionSuffix();
	if (pcSuffix->macpParameters.NumElements() != papcArguments->NumElements())
	{
		return;
	}
	apsParameters.Init();
	for (i = 0; i < pcSuffix->macpParameters.NumElements(); i++)
	{
		pcParameter = (CCPPSyntaxParameter*)pcSuffix->macpParameters.GetPtr(i);
		psType = (pcParameter->GetKind() == CPPS_Parameter) ? mcDeclarators.GetType(pcParameter->mpcSpecifiers, pcParameter->mpcDeclarator, pcParameter, papcArguments->GetPtr(i)) : NULL;
		if (!psType)
		{
			apsParameters.Kill();
			return;
		}
		apsParameters.Add(ParameterType(psType));
	}

	for (i = 0; i < psLambda->pcClass->mapcMembers.NumElements(); i++)
	{
		pcFunction = psLambda->pcClass->mapcMembers.GetPtr(i);
		if ((pcFunction->mlliName != GetOperatorName(CT_OpenRoundBracket, false)) || (pcFunction->mapsParameters.NumElements() != apsParameters.NumElements()))
		{
			continue;
		}
		bSame = true;
		for (j = 0; bSame && (j < apsParameters.NumElements()); j++)
		{
			bSame = pcFunction->mapsParameters.GetPtr(j)->Same(apsParameters.GetPtr(j));
		}
		if (bSame)
		{
			apsParameters.Kill();
			return;
		}
	}
	LambdaFunction(psLambda, &apsParameters);
	apsParameters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The closure whose operator() (or invoker) is being written, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPPLambda* CCPPUnit::WritingLambda(void)
{
	return BodyLambda(mcContext.GetFunction());
}


//////////////////////////////////////////////////////////////////////////
//
// The lambda whose body the function is (its closure's operator() or
// invoker), or NULL: the closure's destructor and conversion are its own.
//
//////////////////////////////////////////////////////////////////////////
SCPPLambda* CCPPUnit::BodyLambda(CCPPFunction* pcFunction)
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
SCPPLambdaCapture* CCPPUnit::FindCapture(CCPPSymbol* pcSymbol)
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
CCPPFunction* CCPPUnit::OuterFunction(void)
{
	CCPPFunction*	pcFunction;

	pcFunction = mcContext.GetFunction();
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
CCPPClass* CCPPUnit::ThisClass(void)
{
	CCPPFunction*	pcFunction;

	pcFunction = mcContext.GetFunction();
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
SCPP65Type* CCPPUnit::LambdaType(CCPPSyntaxLambdaExpression* pcSyntax)
{
	SCPPLambda*	psLambda;

	psLambda = GetLambda(pcSyntax);
	return psLambda ? psLambda->pcClass->mpsType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::FreeLambdas(void)
{
	size	i;

	for (i = 0; i < mapsLambdas.NumElements(); i++)
	{
		mapsLambdas.GetPtr(i)->asCaptures.Kill();
		mapsLambdas.GetPtr(i)->szError.Kill();
		free(mapsLambdas.GetPtr(i));
	}
	mapsLambdas.Kill();
}
