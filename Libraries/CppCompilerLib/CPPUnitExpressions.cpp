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
#include "CTInt.h"
#include "CTShort.h"
#include "CTLongLong.h"
#include "CTChar.h"
#include "CTBoolean.h"
#include "CTString.h"
#include "CTIdentifier.h"
#include "CTUserLiteral.h"
#include "CppTokeniser.h"
#include "CPP65IRFrontEnd.h"
#include "CPPSyntaxTree.h"
#include "CPPUnit.h"
#include "CPPConstants.h"
#include "CPPSemanticPass.h"
#include "CPPTypeWriter.h"
#include "LiteralHelper.h"


//////////////////////////////////////////////////////////////////////////
//
// Numbers, characters, true, false and nullptr.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::Literal(CCPPSyntaxLiteralExpression* pcLiteral)
{
	CCPPToken*		pcToken;

	pcToken = mpcTokeniser->Get(pcLiteral->miLiteral);
	if (pcToken->IsUserLiteral())
	{
		mcDiagnostics.Error(pcLiteral, "No literal operator takes the literal");
		return NULL;
	}
	if (pcToken->IsNumber())
	{
		switch (((CCTNumber*)pcToken)->meType)
		{
		case CTNT_Int:
			return ((CCTInt*)pcToken)->mbChar32 ? &mcTypes.msChar32 : IntegerLiteralType((CCTInt*)pcToken);
		case CTNT_Short:
			//L'x' and u'x'.
			return ((CCTShort*)pcToken)->mbChar16 ? &mcTypes.msChar16 : &mcTypes.msWChar;
		case CTNT_LongLong:
			return ((CCTLongLong*)pcToken)->IsSigned() ? &mcTypes.msLongLong : &mcTypes.msUnsignedLongLong;
		case CTNT_Float:
			return &mcTypes.msFloat;
		case CTNT_Double:
			return &mcTypes.msDouble;
		case CTNT_LongDouble:
			return &mcTypes.msLongDouble;
		default:
			mcDiagnostics.Error(pcLiteral, "This literal is not supported by the compiler yet");
			return NULL;
		}
	}
	if (pcToken->IsChar())
	{
		return &mcTypes.msChar;
	}
	if (pcToken->IsBool())
	{
		return &mcTypes.msBool;
	}
	//nullptr is a std::nullptr_t.  [lex.nullptr]
	return &mcTypes.msNullPointer;
}


//////////////////////////////////////////////////////////////////////////
//
// An int literal is int (or unsigned int) if it fits and has no L suffix, otherwise long (or unsigned long): only
// different where int is narrower than 32 bits (the W65C816).  A hex or octal one too big for an int is an unsigned
// int if it fits one ([lex.icon]).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::IntegerLiteralType(CCTInt* pcInt)
{
	uint64	ulliValue;
	uint64	ulliIntMax;

	//An L literal is long (unsigned long if it's unsigned), whatever int is.
	ulliValue = (uint32)pcInt->mcValue;
	ulliIntMax = (uint64)mcTypes.msInt.Most();
	if (pcInt->IsSigned())
	{
		if (!pcInt->mbLong && (ulliValue <= ulliIntMax))
		{
			return &mcTypes.msInt;
		}
		if (!pcInt->mbLong && !pcInt->mbDecimal && (ulliValue <= ulliIntMax * 2 + 1))
		{
			return &mcTypes.msUnsignedInt;
		}
		return &mcTypes.msLong;
	}
	if (!pcInt->mbLong && (ulliValue <= ulliIntMax * 2 + 1))
	{
		return &mcTypes.msUnsignedInt;
	}
	return &mcTypes.msUnsignedLong;
}


//////////////////////////////////////////////////////////////////////////
//
// __func__ and __FUNCTION__ are the function's name.  __PRETTY_FUNCTION__ and
// __FUNCSIG__ are its class and name ("CClass::Name"), not the whole signature.
// Returns false if lliName is not one of them.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::FunctionNameText(int64 lliName, CChars* pszText)
{
	char*	szName;
	int		iLength;
	bool	bPretty;

	bPretty = (lliName == mpcTokeniser->mlliPrettyFunction) || (lliName == mpcTokeniser->mlliFuncSig);
	if (!bPretty && (lliName != mpcTokeniser->mlliFunc) && (lliName != mpcTokeniser->mlliFunction))
	{
		return false;
	}

	if (mcContext.GetFunction())
	{
		if (bPretty && mcContext.GetFunction()->mpcClass)
		{
			szName = mpcTokeniser->GetIdentifier(mcContext.GetFunction()->mpcClass->mlliName, &iLength);
			pszText->Append(szName, iLength);
			pszText->Append("::");
		}
		if (!mcContext.GetFunction()->IsOperator())
		{
			szName = mpcTokeniser->GetIdentifier(mcContext.GetFunction()->mlliName, &iLength);
			pszText->Append(szName, iLength);
		}
		else
		{
			pszText->Append("operator");
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// True for __func__ and the other names of the enclosing function, which the parser
// declares as globals.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsFunctionName(CCPPSyntaxIdExpression* pcId)
{
	CCPPSymbol*	pcSymbol;
	CChars		szName;
	bool		bName;

	pcSymbol = pcId->mpcName->mpcSymbol;
	if ((pcId->mpcName->macpComponents.NumElements() != 1) || (pcSymbol && ((pcSymbol->meKind != CPPSK_Variable) || GetVariable(pcSymbol))))
	{
		return false;
	}
	szName.Init();
	bName = FunctionNameText(GetComponentName(pcId->mpcName->GetLast()), &szName);
	szName.Kill();
	return bName;
}


//The children of an expression CCPPUnit::MayThrow asks about.
class CCPPThrowFinder : public CCPPSyntaxChildVisitor
{
public:
	CCPPUnit*	mpcUnit;
	bool		mbThrows;

	void	Init(CCPPUnit* pcUnit) { mpcUnit = pcUnit; mbThrows = false; }
	void	Visit(CCPPSyntax* pcChild) override { mbThrows = mbThrows || mpcUnit->MayThrow(pcChild); }
};


//////////////////////////////////////////////////////////////////////////
//
// f or &f (a function's name) when the value is for psTo, a pointer to a
// function: its address is the overload of f with psTo's function type.  NULL
// for any other expression or type.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPUnit::FunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo)
{
	if (!pcExpression || !psTo || !psTo->IsPointer() || !psTo->psBase || !psTo->psBase->IsFunction() || psTo->IsMemberPointer())
	{
		return NULL;
	}
	return FunctionNameOf(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// &Class::f when the value is for psTo, a pointer to a member function:
// the member is the overload of f with psTo's function type.  NULL for any
// other expression or type.  [over.over] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPUnit::MemberFunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo)
{
	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression || !psTo || !psTo->IsMemberPointer() || !psTo->psBase->IsFunction() || (pcExpression->GetKind() != CPPS_UnaryExpression) || (((CCPPSyntaxUnaryExpression*)pcExpression)->meOperator != CT_BitwiseAnd))
	{
		return NULL;
	}
	return MemberPointerName((CCPPSyntaxUnaryExpression*)pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The name &Class::member forms a pointer to a member from, or NULL: the
// operand of & as it is written, a name a class qualifies with no brackets
// about it (&(Class::member) is the member's own address, where it has
// one).  [expr.unary.op] 4
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPUnit::MemberPointerName(CCPPSyntaxUnaryExpression* pcAddress)
{
	CCPPSyntax*	pcOperand;

	pcOperand = pcAddress->mpcOperand;
	if (!pcOperand || (pcOperand->GetKind() != CPPS_IdExpression) || !((CCPPSyntaxIdExpression*)pcOperand)->mpcName->IsQualifiedByClass())
	{
		return NULL;
	}
	return (CCPPSyntaxIdExpression*)pcOperand;
}


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
// The function suffix of the function a declarator's pointer or reference
// is to: void (*p)() noexcept's "() noexcept".  NULL for a declarator of
// anything else.  The specification is the pointer's, not its type's
// ([except.spec] 13), so it is read from where the pointer is declared.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxFunctionSuffix* CCPPUnit::PointedFunctionSuffix(CCPPSyntaxDeclarator* pcDeclarator)
{
	CCPPSyntax*	pcSuffix;

	while (pcDeclarator)
	{
		if (pcDeclarator->mpcNested && !pcDeclarator->GetFunctionSuffix() && (pcDeclarator->macpSuffixes.NumElements() > 0))
		{
			pcSuffix = pcDeclarator->macpSuffixes.GetPtr(0);
			if (pcSuffix->GetKind() == CPPS_FunctionSuffix)
			{
				return (CCPPSyntaxFunctionSuffix*)pcSuffix;
			}
		}
		pcDeclarator = pcDeclarator->mpcNested;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What a call through pcCallee may throw, when pcCallee names a pointer
// or a reference to a function (p, *p, object.p, object->p): what its
// declarator says, anything if it says nothing.  False if pcCallee isn't
// such a name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::PointerExceptions(CCPPSyntax* pcCallee, CCPPExceptionSpec* pcDest)
{
	CCPPSyntaxExceptionSpecification*	pcSpecification;
	CCPPSyntaxDeclarator*				pcDeclarator;
	CCPPSymbol*							pcSymbol;

	pcCallee = SkipParentheses(pcCallee);
	while (pcCallee && (pcCallee->GetKind() == CPPS_UnaryExpression) && (((CCPPSyntaxUnaryExpression*)pcCallee)->meOperator == CT_Multiply))
	{
		pcCallee = SkipParentheses(((CCPPSyntaxUnaryExpression*)pcCallee)->mpcOperand);
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
	pcSpecification = WrittenSpecification(PointedFunctionSuffix(pcDeclarator));
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
bool CCPPUnit::CheckPointerExceptions(CCPPSyntaxInitDeclarator* pcInit)
{
	CCPPSyntaxExceptionSpecification*	pcSpecification;
	CCPPSyntaxSimpleDeclaration*		pcDeclaration;
	CCPPSyntax*							pcValue;
	CArraySyntax*						papcElements;
	CCPPExceptionSpec					cPointer;
	SCPP65Type*							psType;
	bool								bAllowed;

	pcSpecification = WrittenSpecification(PointedFunctionSuffix(pcInit->mpcDeclarator));
	pcValue = pcInit->mpcInitializer;
	if (!pcSpecification || !pcValue || !pcInit->mpcParent || (pcInit->mpcParent->GetKind() != CPPS_SimpleDeclaration))
	{
		return true;
	}
	if ((pcValue->GetKind() == CPPS_ParenthesisedInitializer) || (pcValue->GetKind() == CPPS_InitializerList))
	{
		papcElements = ListElements(pcValue);
		if (!papcElements || (papcElements->NumElements() != 1))
		{
			return true;
		}
		pcValue = papcElements->GetPtr(0);
	}
	pcDeclaration = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;

	cPointer.Init();
	WrittenExceptions(pcSpecification, &cPointer);
	mcDiagnostics.BeginSilent();
	psType = mcDeclarators.GetType(pcDeclaration->mpcSpecifiers, pcInit->mpcDeclarator, pcInit, pcInit->mpcInitializer);
	bAllowed = !psType || ValueAllowsOnly(pcValue, psType, &cPointer);
	mcDiagnostics.EndSilent();
	cPointer.Kill();
	if (!bAllowed)
	{
		return mcDiagnostics.Error(pcInit, "The pointer is declared to allow fewer exceptions than the function it is given");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// p = f;  As CheckPointerExceptions, for a pointer to a function that is
// assigned to.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckAssignedExceptions(CCPPSyntaxBinaryExpression* pcAssignment)
{
	CCPPExceptionSpec	cPointer;
	SCPP65Type*			psType;
	bool				bAllowed;

	cPointer.Init();
	bAllowed = true;
	mcDiagnostics.BeginSilent();
	if (PointerExceptions(pcAssignment->mpcLeft, &cPointer) && !cPointer.IsAnything())
	{
		psType = TypeOf(pcAssignment->mpcLeft);
		bAllowed = !psType || ValueAllowsOnly(pcAssignment->mpcRight, psType, &cPointer);
	}
	mcDiagnostics.EndSilent();
	cPointer.Kill();
	if (!bAllowed)
	{
		return mcDiagnostics.Error(pcAssignment, "The pointer is declared to allow fewer exceptions than the function it is given");
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
bool CCPPUnit::ValueAllowsOnly(CCPPSyntax* pcValue, SCPP65Type* psPointer, CCPPExceptionSpec* pcPointer)
{
	CCPPSyntaxIdExpression*	pcName;
	CCPPFunction*			pcFunction;
	CCPPExceptionSpec		cValue;
	bool					bOverloaded;
	bool					bAllowed;

	if (psPointer->IsReference())
	{
		psPointer = mcTypes.Pointer(psPointer->psBase);
	}
	pcName = FunctionNameFor(pcValue, psPointer);
	if (pcName)
	{
		pcFunction = mcBinder.FindAddressedFunction(pcName->mpcName, pcName->mpcName->mpcSymbol->mlliName, psPointer->psBase, pcName, &bOverloaded);
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
bool CCPPUnit::ConstructionMayThrow(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode, bool bTemporary)
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
		psArgument = TypeOf(papcArguments->GetPtr(0));
		psArgument = psArgument ? psArgument->WithoutReference() : NULL;
		if (psArgument && psArgument->IsClass() && (psArgument->pcClass == pcClass))
		{
			return ClassMayThrow(pcClass, CPPSM_Copy);
		}
	}
	apcNone.Init();
	bThrows = true;
	if (mcBinder.BindConstruction(pcClass, papcArguments ? papcArguments : &apcNone, pcNode, &cBound, false))
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
bool CCPPUnit::ArgumentMayThrow(CCPPSyntax* pcArgument, SCPP65Type* psParameter)
{
	CCPPBoundConversion	cConversion;
	CArraySyntax		apcArgument;
	SCPP65Type*			psTo;
	SCPP65Type*			psFrom;
	int					iOffset;
	bool				bThrows;

	psTo = (psParameter->IsReference() ? psParameter->psBase : psParameter)->Unqualified();
	psFrom = TypeOf(pcArgument);
	if (!psFrom)
	{
		return false;
	}
	psFrom = (psFrom->IsReference() ? psFrom->psBase : psFrom)->Unqualified();
	if (psTo->IsClass())
	{
		if (psFrom->IsClass() && ((psFrom->pcClass == psTo->pcClass) || IsDerived(psFrom->pcClass, psTo->pcClass, &iOffset)))
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
		mcBinder.BindConversion(pcArgument, psParameter, false, &cConversion);
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
bool CCPPUnit::CallMayThrow(CCPPSyntaxCallExpression* pcCall)
{
	CCPPBoundCall		cBound;
	CCPPExceptionSpec	cPointer;
	size				i;
	bool				bThrows;

	if (!mcBinder.Bind(pcCall, &cBound, false))
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
bool CCPPUnit::NewMayThrow(CCPPSyntaxNewExpression* pcNew)
{
	CCPPFunction*	pcAllocation;
	SCPP65Type*		psType;
	CArraySyntax	apcArguments;
	bool			bFailed;

	psType = NewElementType(pcNew);
	if (!psType)
	{
		return true;
	}
	bFailed = false;
	apcArguments.Init();
	NewArguments(pcNew, &apcArguments);
	pcAllocation = mcBinder.FindAllocation(pcNew, psType, &apcArguments, &bFailed);
	apcArguments.Kill();
	if (bFailed || (pcAllocation ? !IsNoexcept(pcAllocation) : !pcNew->mbPlacement))
	{
		return true;
	}
	while (psType->IsArray())
	{
		psType = psType->psBase;
	}
	return psType->IsClass() && ConstructionMayThrow(psType->pcClass, pcNew->mpcInitializer ? ListElements(pcNew->mpcInitializer) : NULL, pcNew, false);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether delete may throw: the destructor of what is deleted, and the
// class's operator delete if it has one (the program's doesn't).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::DeleteMayThrow(CCPPSyntaxDeleteExpression* pcDelete)
{
	CCPPFunction*	pcDeallocation;
	SCPP65Type*		psType;

	psType = TypeOf(pcDelete->mpcOperand);
	psType = psType ? psType->WithoutReference() : NULL;
	psType = (psType && psType->IsPointer()) ? psType->psBase->Unqualified() : NULL;
	if (!psType || !psType->IsClass())
	{
		return false;
	}
	pcDeallocation = mcBinder.FindDeallocation(psType->pcClass, pcDelete->mbArray);
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
bool CCPPUnit::MayThrow(CCPPSyntax* pcExpression)
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
	mcDiagnostics.BeginSilent();
	switch (pcExpression->GetKind())
	{
	case CPPS_ThrowExpression:
		bThrows = true;
		break;
	case CPPS_SizeofExpression:
	case CPPS_TypeTraitExpression:
	case CPPS_NoexceptExpression:
	case CPPS_LambdaExpression:
		mcDiagnostics.EndSilent();
		return false;
	case CPPS_CallExpression:
		bThrows = CallMayThrow((CCPPSyntaxCallExpression*)pcExpression);
		break;
	case CPPS_UnaryExpression:
		pcUnary = (CCPPSyntaxUnaryExpression*)pcExpression;
		bThrows = IsOperatorOperand(pcUnary->mpcOperand, pcUnary->meOperator) && mcBinder.BindOperator(pcUnary->meOperator, pcUnary->mpcOperand, NULL, pcUnary, &cBound, false) && cBound.mpcFunction && !IsNoexcept(cBound.mpcFunction);
		break;
	case CPPS_BinaryExpression:
		pcBinary = (CCPPSyntaxBinaryExpression*)pcExpression;
		bThrows = (IsOperatorOperand(pcBinary->mpcLeft, pcBinary->meOperator) || IsOperatorOperand(pcBinary->mpcRight, pcBinary->meOperator)) && mcBinder.BindOperator(pcBinary->meOperator, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary, &cBound, false) && cBound.mpcFunction && !IsNoexcept(cBound.mpcFunction);
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
		psType = mcDeclarators.GetTypeIdType(pcConstruct->mpcType);
		bThrows = psType && psType->IsClass() && ConstructionMayThrow(psType->pcClass, pcConstruct->mpcInitializer ? ListElements(pcConstruct->mpcInitializer) : NULL, pcConstruct, true);
		break;
	case CPPS_NamedCastExpression:
		//dynamic_cast<T&> throws std::bad_cast when the object isn't a T.  [expr.dynamic.cast] 9
		pcCast = (CCPPSyntaxNamedCastExpression*)pcExpression;
		psType = (pcCast->meCast == CT_dynamic_cast) ? mcDeclarators.GetTypeIdType(pcCast->mpcType) : NULL;
		bThrows = psType && psType->IsReference();
		break;
	case CPPS_TypeidExpression:
		//typeid of an object of a polymorphic class throws std::bad_typeid for *null.  [expr.typeid] 2
		pcTypeid = (CCPPSyntaxTypeidExpression*)pcExpression;
		psType = pcTypeid->mpcOperand ? TypeOf(pcTypeid->mpcOperand) : NULL;
		psType = psType ? psType->WithoutReference() : NULL;
		bThrows = psType && psType->IsClass() && psType->pcClass->IsPolymorphic();
		break;
	default:
		break;
	}
	mcDiagnostics.EndSilent();
	if (bThrows)
	{
		return true;
	}

	cFinder.Init(this);
	pcExpression->VisitFields(&cFinder);
	return cFinder.mbThrows;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is a string literal, its characters: not a
// user-defined one ("text"_s), which is its literal operator's call.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsStringLiteral(CCPPSyntax* pcExpression)
{
	return pcExpression && (pcExpression->GetKind() == CPPS_StringExpression) && !UserLiteralToken(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// The user-defined literal token of a literal (10_km, 'x'_c) or a string
// ("text"_s, or "te" "xt"_s), or NULL for one without a suffix.
//
//////////////////////////////////////////////////////////////////////////
CCTUserLiteral* CCPPUnit::UserLiteralToken(CCPPSyntax* pcLiteral)
{
	CCPPToken*	pcToken;
	int32		iToken;

	if (pcLiteral->GetKind() == CPPS_LiteralExpression)
	{
		pcToken = mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcLiteral)->miLiteral);
		return pcToken->IsUserLiteral() ? (CCTUserLiteral*)pcToken : NULL;
	}
	if (pcLiteral->GetKind() == CPPS_StringExpression)
	{
		for (iToken = pcLiteral->miFirstToken; iToken <= pcLiteral->miLastToken; iToken++)
		{
			pcToken = mpcTokeniser->Get(iToken);
			if (pcToken->IsUserString())
			{
				return (CCTUserLiteral*)pcToken;
			}
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of a character literal's token (char, wchar_t, char16_t or
// char32_t), or NULL if it is no character literal of one character.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::CharacterTokenType(CCPPToken* pcToken)
{
	if (pcToken->IsChar())
	{
		return &mcTypes.msChar;
	}
	if (pcToken->IsNumber() && (((CCTNumber*)pcToken)->meType == CTNT_Short))
	{
		return ((CCTShort*)pcToken)->mbChar16 ? &mcTypes.msChar16 : &mcTypes.msWChar;
	}
	if (pcToken->IsNumber() && (((CCTNumber*)pcToken)->meType == CTNT_Int) && ((CCTInt*)pcToken)->mbChar32)
	{
		return &mcTypes.msChar32;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The call a user-defined literal is, worked out once for its node: its
// suffix's literal operator and what it is given ([lex.ext]).  "text"_s is
// operator "" _s("text", 4) and 'c'_c operator "" _c('c').  12_k is
// operator "" _k(12ULL) if that operator takes an unsigned long long (1.5_k
// a long double), else operator "" _k("12") if it takes a const char*, else
// operator "" _k<'1', '2'>().  NULL if no literal operator takes it.
//
//////////////////////////////////////////////////////////////////////////
SCPPLiteralCall* CCPPUnit::GetLiteralCall(CCPPSyntax* pcLiteral)
{
	SCPPLiteralCall*				psCall;
	SCPPLiteralCall					sCall;
	CCTUserLiteral*					pcUser;
	CCPPSyntaxImplicitArgument*		pcArgument;
	CArraySyntax*					papcArguments;
	CArraySyntax					apcExplicit;
	CArrayUint8						auiBytes;
	CChars							szSpelling;
	SCPP65Type*						psFloat;
	int64							lliName;
	int64							lliValue;
	double							dValue;
	size								i;

	psCall = mcLiteralCalls.Get((int64)(size)pcLiteral);
	if (psCall)
	{
		return psCall->pcFunction ? psCall : NULL;
	}
	pcUser = UserLiteralToken(pcLiteral);
	if (!pcUser)
	{
		return NULL;
	}

	sCall.pcFunction = NULL;
	sCall.pcArguments = mpcTree->GetMemory()->Create<CCPPSyntaxParenthesisedInitializer>(pcLiteral);
	sCall.pcArguments->SetTokens(pcLiteral->miFirstToken, pcLiteral->miLastToken);
	papcArguments = &sCall.pcArguments->macpElements;
	lliName = CPP_LITERAL_NAME(pcUser->miSuffix);
	if (pcLiteral->GetKind() == CPPS_StringExpression)
	{
		auiBytes.Init();
		if (mcStrings.DecodeString(pcLiteral, &auiBytes))
		{
			LiteralArgument(pcLiteral, mcTypes.Pointer(mcTypes.Const(mcStrings.StringElementType(pcLiteral))), papcArguments)->mpcString = pcLiteral;
			pcArgument = LiteralArgument(pcLiteral, mcTypes.Size(), papcArguments);
			pcArgument->mbConstant = true;
			pcArgument->mlliValue = (int64)(auiBytes.NumElements() / mcStrings.StringElementBytes(pcLiteral));
			sCall.pcFunction = mcBinder.FindLiteralOperator(lliName, NULL, papcArguments, pcLiteral);
		}
		auiBytes.Kill();
	}
	else if (CharacterTokenType(pcUser->mpcLiteral))
	{
		//'c'_x is given a char, L'c'_x a wchar_t, u'c'_x a char16_t and U'c'_x a char32_t.  [lex.ext] 6
		pcArgument = LiteralArgument(pcLiteral, CharacterTokenType(pcUser->mpcLiteral), papcArguments);
		pcArgument->mbConstant = mcFolder.TokenConstant(pcUser->mpcLiteral, &pcArgument->mlliValue);
		sCall.pcFunction = mcBinder.FindLiteralOperator(lliName, NULL, papcArguments, pcLiteral);
	}
	else if (pcUser->mpcLiteral->IsNumber())
	{
		psFloat = NULL;
		dValue = 0;
		if (mcFolder.TokenConstant(pcUser->mpcLiteral, &lliValue))
		{
			pcArgument = LiteralArgument(pcLiteral, &mcTypes.msUnsignedLongLong, papcArguments);
			pcArgument->mlliValue = lliValue;
		}
		else
		{
			pcArgument = LiteralArgument(pcLiteral, mcTypes.LongDouble(), papcArguments);
			pcArgument->mdValue = mcFolder.TokenFloat(pcUser->mpcLiteral, &dValue, &psFloat) ? dValue : 0;
		}
		pcArgument->mbConstant = true;
		sCall.pcFunction = mcBinder.FindLiteralOperator(lliName, NULL, papcArguments, pcLiteral);

		//The raw operator, given the literal's spelling.
		if (!sCall.pcFunction)
		{
			papcArguments->ReInit();
			LiteralArgument(pcLiteral, mcTypes.Pointer(mcTypes.Const(&mcTypes.msChar)), papcArguments)->miSpelling = ((CCPPSyntaxLiteralExpression*)pcLiteral)->miLiteral;
			sCall.pcFunction = mcBinder.FindLiteralOperator(lliName, NULL, papcArguments, pcLiteral);
		}

		//The template operator, given its characters as its arguments.
		if (!sCall.pcFunction)
		{
			papcArguments->ReInit();
			apcExplicit.Init();
			szSpelling.Init();
			UserLiteralSpelling(((CCPPSyntaxLiteralExpression*)pcLiteral)->miLiteral, &szSpelling);
			for (i = 0; i < szSpelling.Length(); i++)
			{
				pcArgument = LiteralArgument(pcLiteral, &mcTypes.msChar, &apcExplicit);
				pcArgument->mbConstant = true;
				pcArgument->mlliValue = szSpelling.GetChar(i);
			}
			sCall.pcFunction = mcBinder.FindLiteralOperator(lliName, &apcExplicit, papcArguments, pcLiteral);
			szSpelling.Kill();
			apcExplicit.Kill();
		}
	}
	mcLiteralCalls.Put((int64)(size)pcLiteral, &sCall);
	return sCall.pcFunction ? mcLiteralCalls.Get((int64)(size)pcLiteral) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// An argument of psType for a user-defined literal's call, added to
// papcArguments: the caller says what it holds.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxImplicitArgument* CCPPUnit::LiteralArgument(CCPPSyntax* pcLiteral, SCPP65Type* psType, CArraySyntax* papcArguments)
{
	CCPPSyntaxImplicitArgument*	pcArgument;

	pcArgument = mpcTree->GetMemory()->Create<CCPPSyntaxImplicitArgument>(NULL);
	pcArgument->SetTokens(pcLiteral->miFirstToken, pcLiteral->miLastToken);
	pcArgument->mpsType = psType;
	papcArguments->Add(pcArgument);
	return pcArgument;
}


//////////////////////////////////////////////////////////////////////////
//
// A user-defined literal token's spelling without its suffix: 0x1F of
// 0x1F_k, what a raw literal operator is given.  [lex.ext] 3
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::UserLiteralSpelling(int32 iToken, CChars* pszSpelling)
{
	CCPPToken*			pcToken;
	SPreprocessedToken*	psSource;
	int					iSuffix;

	pcToken = mpcTokeniser->Get(iToken);
	psSource = mpcTokeniser->GetSource(pcToken);
	iSuffix = 0;
	if (pcToken->IsUserLiteral())
	{
		mpcTokeniser->GetIdentifier(((CCTUserLiteral*)pcToken)->miSuffix, &iSuffix);
	}
	pszSpelling->Append(psSource->szText, (int)psSource->uiLength - iSuffix);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name that isn't a function's is ambiguous: lookup finds two
// different things where it finds it (CCPPScope::IsAmbiguous, or for a
// name qualified by a namespace IsAmbiguousMember).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsAmbiguousName(CCPPSyntaxName* pcName)
{
	CCPPSyntaxNameComponent*	pcQualifier;
	int64						lliName;

	if (!pcName->mpcSymbol || pcName->mpcSymbol->IsFunction() || (pcName->GetLast()->meKind != CPPNC_Identifier))
	{
		return false;
	}
	lliName = pcName->mpcSymbol->mlliName;
	pcQualifier = pcName->GetQualifier();
	if (pcQualifier)
	{
		return pcQualifier->mpcSymbol && (pcQualifier->mpcSymbol->meKind == CPPSK_Namespace) && pcQualifier->mpcSymbol->mpcMembers && pcQualifier->mpcSymbol->mpcMembers->IsAmbiguousMember(lliName);
	}
	return !pcName->mbGlobal && pcName->mpcLookup && pcName->mpcLookup->IsAmbiguous(lliName);
}


//////////////////////////////////////////////////////////////////////////
//
// What a new expression makes: its type-id's type (auto's is its
// initialiser's), and for new Type[count][N]... arrays of the inner bounds
// (constants).  A type-id that is an array by itself (NewTypeIdArray) makes
// the array's elements.  NULL after an error.  [expr.new] 2, 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::NewElementType(CCPPSyntaxNewExpression* pcNew)
{
	CArraySyntax*	papcElements;
	CCPPSyntax*		pcInitialiser;
	SCPP65Type*		psType;
	int64			lliCount;
	size			i;

	//new auto(x) is of x's type: the one thing in its brackets.  [expr.new] 2
	pcInitialiser = NULL;
	if (mcDeclarators.IsAuto(pcNew->mpcType->mpcSpecifiers) && pcNew->mpcInitializer)
	{
		papcElements = ListElements(pcNew->mpcInitializer);
		pcInitialiser = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : NULL;
	}
	psType = pcInitialiser ? mcDeclarators.GetType(pcNew->mpcType->mpcSpecifiers, pcNew->mpcType->mpcDeclarator, pcNew->mpcType, pcInitialiser) : mcDeclarators.GetTypeIdType(pcNew->mpcType);
	for (i = pcNew->macpArraySizes.NumElements(); psType && (i > 1); i--)
	{
		if (!mcFolder.Constant(pcNew->macpArraySizes.GetPtr(i - 1), &lliCount) || (lliCount <= 0))
		{
			mcFolder.ConstantError(pcNew->macpArraySizes.GetPtr(i - 1), "A new array's bounds after the first must be positive constants");
			return NULL;
		}
		psType = mcTypes.Array(psType, (int)lliCount);
	}
	return (psType && psType->IsArray() && (pcNew->macpArraySizes.NumElements() == 0)) ? psType->psBase : psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The array a new expression's type-id is by itself, with no bound after
// it: new (int[3]), or new A where A is a typedef of an array.  It makes the
// array's elements, as new int[3] does, and gives a pointer to the first.
// NULL if it isn't one.  [expr.new] 5
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::NewTypeIdArray(CCPPSyntaxNewExpression* pcNew)
{
	SCPP65Type*	psType;

	if ((pcNew->macpArraySizes.NumElements() > 0) || mcDeclarators.IsAuto(pcNew->mpcType->mpcSpecifiers))
	{
		return NULL;
	}
	mcDiagnostics.BeginQuiet();
	psType = mcDeclarators.GetTypeIdType(pcNew->mpcType);
	mcDiagnostics.EndQuiet();
	return (psType && psType->IsArray()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a new expression makes an array: by operator new[], its elements
// each constructed.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsArrayNew(CCPPSyntaxNewExpression* pcNew)
{
	return (pcNew->macpArraySizes.NumElements() > 0) || (NewTypeIdArray(pcNew) != NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// A new expression's allocation arguments: its size (NewSize), then its
// placement arguments.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::NewArguments(CCPPSyntaxNewExpression* pcNew, CArraySyntax* papcArguments)
{
	size	i;

	papcArguments->Add(NewSize(pcNew));
	for (i = 0; i < pcNew->macpPlacement.NumElements(); i++)
	{
		papcArguments->Add(pcNew->macpPlacement.GetPtr(i));
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The int 0 a postfix operator function is called with, made the first
// time it is wanted (in no tree's nodes: it has no parent).
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::ImplicitZero(void)
{
	if (!mpcImplicitZero)
	{
		mpcImplicitZero = mpcTree->GetMemory()->Create<CCPPSyntaxImplicitArgument>(NULL);
		mpcImplicitZero->mpsType = &mcTypes.msInt;
		mpcImplicitZero->mbConstant = true;
	}
	return mpcImplicitZero;
}


//////////////////////////////////////////////////////////////////////////
//
// An argument the IR front end gives a value, pcFor's own iWhich'th (as a
// new expression in a new expression's placement arguments mustn't change
// its size's), of the type psType: the size a new expression's allocation
// function is given, or the address, and then the size, that delete gives
// a class's operator delete.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxImplicitArgument* CCPPUnit::ImplicitValue(CCPPSyntax* pcFor, SCPP65Type* psType, int iWhich)
{
	CCPPSyntaxImplicitArgument**	ppcValue;
	CCPPSyntaxImplicitArgument*		pcValue;
	int64							lliKey;

	//A node's address and those of the few bytes after it are no other node's.
	lliKey = (int64)(size)pcFor + iWhich;
	ppcValue = mcImplicitValues.Get(lliKey);
	if (ppcValue)
	{
		return *ppcValue;
	}
	pcValue = mpcTree->GetMemory()->Create<CCPPSyntaxImplicitArgument>(NULL);
	pcValue->mpsType = psType;
	mcImplicitValues.Put(lliKey, &pcValue);
	return pcValue;
}


//////////////////////////////////////////////////////////////////////////
//
// The size a new expression's allocation function is given: a size_t.
// [expr.new] 9
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::NewSize(CCPPSyntaxNewExpression* pcNew)
{
	return ImplicitValue(pcNew, mcTypes.Size());
}


//////////////////////////////////////////////////////////////////////////
//
// f or &f: the name of a function (overloaded or not), else NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPUnit::FunctionNameOf(CCPPSyntax* pcExpression)
{
	CCPPSyntax*				pcFunction;
	CCPPSyntaxIdExpression*	pcId;
	CCPPSymbol*				pcSymbol;

	pcFunction = SkipParentheses(pcExpression);
	if (pcFunction && (pcFunction->GetKind() == CPPS_UnaryExpression) && (((CCPPSyntaxUnaryExpression*)pcFunction)->meOperator == CT_BitwiseAnd))
	{
		pcFunction = SkipParentheses(((CCPPSyntaxUnaryExpression*)pcFunction)->mpcOperand);
	}
	if (!pcFunction || (pcFunction->GetKind() != CPPS_IdExpression))
	{
		return NULL;
	}
	pcId = (CCPPSyntaxIdExpression*)pcFunction;
	pcSymbol = pcId->mpcName->mpcSymbol;
	if (pcSymbol && (pcSymbol->meKind == CPPSK_FunctionTemplate) && !pcId->mpcName->GetLast()->mbTemplateArguments)
	{
		//A function template's name without template arguments is an overload set too, its instances' arguments
		//deduced from the type wanted.  [over.over] 2
		return pcId;
	}
	if (!pcSymbol || (pcSymbol->meKind != CPPSK_Function))
	{
		return NULL;
	}
	return pcId;
}


//////////////////////////////////////////////////////////////////////////
//
// A name reserved for the compiler (__debugbreak): the host compiler's
// intrinsics, which no header declares.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsIntrinsic(int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	return szName && (iLength > 2) && (szName[0] == '_') && (szName[1] == '_');
}


//////////////////////////////////////////////////////////////////////////
//
// True for the names of GCC's builtin functions, which are not declared.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsBuiltin(int64 lliName)
{
	char*	szName;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szName = mpcTokeniser->GetIdentifier(lliName, &iLength);
	return szName && (iLength > CPP65_BUILTIN_PREFIX_LENGTH) && (memcmp(szName, CPP65_BUILTIN_PREFIX, CPP65_BUILTIN_PREFIX_LENGTH) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Constant, without reporting an error when the expression isn't constant
// (most aren't: it is only looking), and without changing whether an
// earlier quiet evaluation failed.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::QuietConstant(CCPPSyntax* pcExpression, int64* plliValue)
{
	bool	bConstant;

	mcDiagnostics.BeginTrial();
	bConstant = mcFolder.Constant(pcExpression, plliValue);
	mcDiagnostics.EndTrial();
	return bConstant;
}


#define CPP65_BCD_ONLY	"__builtin_bcd_add and __builtin_bcd_subtract are the W65C816's decimal mode: use BaseLib's BCDAdd and BCDSubtract"


//The builtins the compiler does: the type checker and the IR front end both look them up here.
static const SCPPBuiltin gasCPP65Builtins[] =
{
	{ "unreachable",	CPPB_Unreachable,	0, CPPBO_None,				NULL },
	{ "trap",			CPPB_Unreachable,	0, CPPBO_None,				NULL },
	{ "ia32_pause",		CPPB_Pause,		0, CPPBO_None,				NULL },
	{ "expect",			CPPB_Expect,		2, CPPBO_None,				NULL },
	{ "memcpy",			CPPB_Memcpy,		3, CPPBO_None,				NULL },
	{ "alloca",			CPPB_Alloca,		1, CPPBO_None,				NULL },
	{ "bcd_add",		CPPB_BCDAdd,		4, CPPBO_None,				CPP65_BCD_ONLY },
	{ "bcd_subtract",	CPPB_BCDSubtract,	4, CPPBO_None,				CPP65_BCD_ONLY },
	{ "ctz",			CPPB_Ctz,			1, CPPBO_UnsignedInt,			NULL },
	{ "ctzl",			CPPB_Ctz,			1, CPPBO_UnsignedLong,		NULL },
	{ "ctzll",			CPPB_Ctz,			1, CPPBO_UnsignedLongLong,	NULL },
	{ "clz",			CPPB_Clz,			1, CPPBO_UnsignedInt,			NULL },
	{ "clzl",			CPPB_Clz,			1, CPPBO_UnsignedLong,		NULL },
	{ "clzll",			CPPB_Clz,			1, CPPBO_UnsignedLongLong,	NULL },
	{ "popcount",		CPPB_Popcount,	1, CPPBO_UnsignedInt,			NULL },
	{ "popcountl",		CPPB_Popcount,	1, CPPBO_UnsignedLong,		NULL },
	{ "popcountll",		CPPB_Popcount,	1, CPPBO_UnsignedLongLong,	NULL },
	{ NULL,				CPPB_Unreachable,	0, CPPBO_None,				NULL },
};


//////////////////////////////////////////////////////////////////////////
//
// The builtin lliName names (__builtin_ and an entry of gasCPP65Builtins),
// NULL for one the compiler doesn't do (or a name that isn't a builtin's).
//
//////////////////////////////////////////////////////////////////////////
const SCPPBuiltin* CCPPUnit::FindBuiltin(int64 lliName)
{
	char*	szName;
	int		iLength;
	size	i;

	if (!IsBuiltin(lliName))
	{
		return NULL;
	}
	szName = BuiltinName(lliName, &iLength);
	for (i = 0; gasCPP65Builtins[i].szName; i++)
	{
		if ((StrLen(gasCPP65Builtins[i].szName) == (size)iLength) && (memcmp(gasCPP65Builtins[i].szName, szName, iLength) == 0))
		{
			return &gasCPP65Builtins[i];
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type ctz, clz or popcount counts the bits of (NULL for the others).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::BuiltinOperandType(const SCPPBuiltin* psBuiltin)
{
	switch (psBuiltin->eOperand)
	{
	case CPPBO_UnsignedInt:		return &mcTypes.msUnsignedInt;
	case CPPBO_UnsignedLong:		return mcTypes.Long(false);
	case CPPBO_UnsignedLongLong:	return &mcTypes.msUnsignedLongLong;
	default:						return NULL;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// A builtin's name without "__builtin_" (IsBuiltin) and its length.
//
//////////////////////////////////////////////////////////////////////////
char* CCPPUnit::BuiltinName(int64 lliName, int* piLength)
{
	char*	szName;

	szName = mpcTokeniser->GetIdentifier(lliName, piLength);
	*piLength -= CPP65_BUILTIN_PREFIX_LENGTH;
	return szName + CPP65_BUILTIN_PREFIX_LENGTH;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a call to lliName is memcpy (or __builtin_memcpy) with a constant count
// of 1 to CPP65_INLINE_MEMCPY bytes (CPP65_INLINE_MEMCPY_SIZE preferring size), which InlineMemcpy copies in
// place.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsInlineMemcpy(int64 lliName, CArraySyntax* papcArguments)
{
	int64	lliBytes;

	if (((lliName != mpcTokeniser->mlliMemcpy) && (lliName != mpcTokeniser->mlliBuiltinMemcpy)) || (papcArguments->NumElements() != 3))
	{
		return false;
	}
	return mcFolder.Constant(papcArguments->GetPtr(2), &lliBytes) && (lliBytes > 0) && (lliBytes <= (mpcOptions->PrefersSize() ? CPP65_INLINE_MEMCPY_SIZE : CPP65_INLINE_MEMCPY));
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the class or a base has a member function of the name.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasMember(CCPPClass* pcClass, int64 lliName)
{
	CCPPFunction*	pcFunction;
	size			i;

	if (!CompleteClass(pcClass, NULL))
	{
		return false;
	}
	for (pcFunction = FirstFunctionOfName(lliName); pcFunction; pcFunction = pcFunction->mpcNextOfName)
	{
		if ((pcFunction->mpcClass == pcClass) && (pcFunction->meKind == CPPFK_Function))
		{
			return true;
		}
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if (HasMember(pcClass->macBases.Get(i)->pcClass, lliName))
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcClass (or a base) has an operator function named lliName: a member function, or a member template taking
// uiArguments.  The class is laid out to find out, if it hasn't been: its members and its using-declarations are
// known only then.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::HasOperator(CCPPClass* pcClass, int64 lliName, size uiArguments)
{
	size	i;

	if (lliName == GetOperatorName(CT_Assignment, false))
	{
		//A base's operator= is hidden by the copy assignment operator every class declares: only the class's own
		//are, and those a using-declaration in it names (using Base::operator=;).  [over.ass] 1, [namespace.udecl] 4
		if (!CompleteClass(pcClass, NULL))
		{
			return false;
		}
		for (i = 0; i < pcClass->mapcMembers.NumElements(); i++)
		{
			if (pcClass->mapcMembers.GetPtr(i)->mlliName == lliName)
			{
				return true;
			}
		}
		for (i = 0; i < pcClass->macUsings.NumElements(); i++)
		{
			if (pcClass->macUsings.Get(i)->lliName == lliName)
			{
				return true;
			}
		}
		return mcBinder.HasOwnMemberTemplate(pcClass, lliName, uiArguments);
	}
	return HasMember(pcClass, lliName) || mcBinder.HasMemberTemplate(pcClass, lliName, uiArguments);
}


//////////////////////////////////////////////////////////////////////////
//
// The enumerator Class::Name names.  See QualifiedMember.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::QualifiedEnumerator(CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPClass** ppcClass)
{
	CCPPSymbol*	pcSymbol;

	pcSymbol = QualifiedMember(pcName, CPPSK_Enumerator, pcNode, ppcClass);
	if (!pcSymbol)
	{
		*ppcClass = NULL;
		pcSymbol = DecltypeEnumerator(pcName);
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// decltype(e)::name where e is of an enumeration: the enumeration's
// enumerator of the name, or NULL.  [dcl.type.simple] 4, [dcl.enum] 11
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::DecltypeEnumerator(CCPPSyntaxName* pcName)
{
	SCPP65Type*	psType;
	CCPPSymbol*	pcEnum;
	CCPPSymbol*	pcSymbol;

	if (!pcName->IsQualifiedByDecltype() || (pcName->macpComponents.NumElements() != 2))
	{
		return NULL;
	}
	mcDiagnostics.BeginSilent();
	psType = mcDeclarators.GetTypeIdType(((CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(0))->mpcType);
	mcDiagnostics.EndSilent();
	pcEnum = psType ? psType->WithoutReference()->pcEnum : NULL;
	if (!pcEnum)
	{
		return NULL;
	}

	//A scoped enumeration's enumerators are its members; one that isn't scoped has them in the scope it is in too.
	pcSymbol = pcEnum->mpcMembers ? pcEnum->mpcMembers->FindHere(GetComponentName(pcName->GetLast())) : NULL;
	pcSymbol = pcSymbol ? pcSymbol : (pcEnum->mpcScope ? pcEnum->mpcScope->FindHere(GetComponentName(pcName->GetLast())) : NULL);
	psType = (pcSymbol && (pcSymbol->meKind == CPPSK_Enumerator)) ? EnumeratorType(pcSymbol) : NULL;
	return (psType && (psType->pcEnum == pcEnum)) ? pcSymbol : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type sizeof or typeid is asked of: its type-id's (pcType), or its
// expression's, which isn't evaluated.  decltype(d)::name, which the parser
// takes for a type where one can be written (only the type of d says which
// it is), is the member it names when it names no type.  NULL after an
// error.  [expr.sizeof] 1, [expr.typeid] 3, 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::OperandType(CCPPSyntaxTypeId* pcType, CCPPSyntax* pcOperand)
{
	SCPP65Type*	psType;
	CCPPSyntax*	pcMember;

	if (!pcType)
	{
		return UnevaluatedTypeOf(pcOperand);
	}
	pcMember = DecltypeMember(pcType);
	if (!pcMember)
	{
		return mcDeclarators.GetTypeIdType(pcType);
	}
	mcDiagnostics.BeginSilent();
	psType = mcDeclarators.GetTypeIdType(pcType);
	mcDiagnostics.EndSilent();
	return psType ? psType : UnevaluatedTypeOf(pcMember);
}


//////////////////////////////////////////////////////////////////////////
//
// The expression a type-id is if it is just decltype(e)::name, which may
// name a member that is no type: the name as an expression, made once.
// NULL for any other type-id.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxIdExpression* CCPPUnit::DecltypeMember(CCPPSyntaxTypeId* pcType)
{
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxIdExpression**	ppcMember;
	CCPPSyntaxIdExpression*		pcMember;

	pcSpecifiers = pcType->mpcSpecifiers;
	if (!pcSpecifiers || !pcSpecifiers->mpcType || (pcSpecifiers->mpcType->GetKind() != CPPS_NamedTypeSpecifier) || (pcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_VOLATILE)) ||
		(pcType->mpcDeclarator && !pcType->mpcDeclarator->IsEmpty()))
	{
		return NULL;
	}
	pcName = ((CCPPSyntaxNamedTypeSpecifier*)pcSpecifiers->mpcType)->mpcName;
	if (!pcName->IsQualifiedByDecltype())
	{
		return NULL;
	}
	ppcMember = mcDecltypeMembers.Get((int64)(size)pcType);
	if (ppcMember)
	{
		return *ppcMember;
	}
	pcMember = mpcTree->GetMemory()->Create<CCPPSyntaxIdExpression>(pcType->mpcParent);
	pcMember->SetTokens(pcType->miFirstToken, pcType->miLastToken);
	pcMember->mpcName = pcName;
	mcDecltypeMembers.Put((int64)(size)pcType, &pcMember);
	return pcMember;
}


//////////////////////////////////////////////////////////////////////////
//
// object.operator=(value) where the object's class declares no operator=:
// a call of the copy assignment operator the compiler declares, which is
// object = value (made once, and kept).  NULL for any other call.
// [over.oper] 4, [class.copy] 18
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxBinaryExpression* CCPPUnit::ImplicitAssignment(CCPPSyntaxCallExpression* pcCall)
{
	CCPPSyntaxMemberExpression*		pcMember;
	CCPPSyntaxBinaryExpression**	ppcAssignment;
	CCPPSyntaxBinaryExpression*		pcAssignment;
	SCPP65Type*						psType;
	int64							lliName;
	size							i;

	pcMember = (pcCall->mpcCallee && (pcCall->mpcCallee->GetKind() == CPPS_MemberExpression)) ? (CCPPSyntaxMemberExpression*)pcCall->mpcCallee : NULL;
	lliName = GetOperatorName(CT_Assignment, false);
	if (!pcMember || pcMember->mbArrow || (pcCall->macpArguments.NumElements() != 1) || !pcMember->mpcName || (pcMember->mpcName->macpComponents.NumElements() != 1) ||
		(GetFunctionName(pcMember->mpcName->GetLast()) != lliName))
	{
		return NULL;
	}
	ppcAssignment = mcImplicitAssignments.Get((int64)(size)pcCall);
	if (ppcAssignment)
	{
		return *ppcAssignment;
	}
	psType = ValueTypeOf(pcMember->mpcObject);
	if (!psType || !psType->IsClass() || !CompleteClass(psType->pcClass, pcCall))
	{
		return NULL;
	}
	for (i = 0; i < psType->pcClass->mapcMembers.NumElements(); i++)
	{
		if (psType->pcClass->mapcMembers.GetPtr(i)->mlliName == lliName)
		{
			return NULL;
		}
	}
	pcAssignment = mpcTree->GetMemory()->Create<CCPPSyntaxBinaryExpression>(pcCall);
	pcAssignment->SetTokens(pcCall->miFirstToken, pcCall->miLastToken);
	pcAssignment->meOperator = CT_Assignment;
	pcAssignment->mpcLeft = pcMember->mpcObject;
	pcAssignment->mpcRight = pcCall->macpArguments.GetPtr(0);
	mcImplicitAssignments.Put((int64)(size)pcCall, &pcAssignment);
	return pcAssignment;
}


//////////////////////////////////////////////////////////////////////////
//
// The value of a variable template's instance that is a const or
// constexpr integer (size<long>): its initialiser, worked out with its
// template's parameters its arguments.  False if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::VariableTemplateValue(CCPPSyntaxIdExpression* pcId, int64* plliValue)
{
	CCPPVariable*					pcVariable;
	CCPPSyntaxSimpleDeclaration*	pcSimple;
	CCPPSyntaxInitDeclarator*		pcInit;
	bool							bResult;

	pcVariable = VariableInstance(pcId->mpcName, pcId);
	if (!pcVariable || !pcVariable->mpsType->IsInteger())
	{
		return false;
	}
	pcInit = VariableDeclarator(pcVariable);
	pcSimple = (CCPPSyntaxSimpleDeclaration*)pcInit->mpcParent;
	if (!pcSimple->mpcSpecifiers || !(pcSimple->mpcSpecifiers->muiFlags & (CPPDS_CONST | CPPDS_CONSTEXPR)) || !pcInit->mpcInitializer)
	{
		return false;
	}
	mcContext.EnterBindings(pcVariable->mpcBindings);
	bResult = mcFolder.Constant(pcInit->mpcInitializer, plliValue);
	mcContext.LeaveBindings();
	if (bResult)
	{
		*plliValue = mcFolder.ConstantOfType(pcVariable->mpsType, *plliValue);
	}
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// The member of the kind eKind Class::Name names, where Class is a class, an instance of a class template, a typedef of
// one or a template parameter, and in ppsClass the class that declares it: a member of an instance can depend on its
// template arguments, so it is worked out with that class's bindings.  NULL if it isn't one.  A qualifier after a
// class is found in that class (QualifiersClass): TEMPLATE::SizePolicy::NotFound and W<int>::Type::N.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::QualifiedMember(CCPPSyntaxName* pcName, ECPPSymbolKind eKind, CCPPSyntax* pcNode, CCPPClass** ppcClass)
{
	CCPPSyntaxNameComponent*	pcQualifier;
	CCPPClass*				pcClass;
	CCPPSymbol*					pcSymbol;
	bool						bAmbiguous;

	if (pcName->macpComponents.NumElements() < 2)
	{
		return NULL;
	}
	pcQualifier = pcName->GetQualifier();
	if (!pcQualifier)
	{
		return NULL;
	}
	if (pcQualifier->mpcSymbol)
	{
		switch (pcQualifier->mpcSymbol->meKind)
		{
		case CPPSK_Class:
		case CPPSK_ClassTemplate:
		case CPPSK_Typedef:
		case CPPSK_TypeParameter:
			break;
		default:
			return NULL;
		}
	}
	else if ((pcName->macpComponents.NumElements() < 3) && (pcQualifier->meKind != CPPNC_Decltype))
	{
		return NULL;
	}

	mcDiagnostics.BeginSilent();
	pcClass = (pcQualifier->mpcSymbol && (pcName->macpComponents.NumElements() == 2)) ? GetComponentClass(pcQualifier, NULL, pcNode) : QualifiersClass(pcName, pcNode);
	pcSymbol = (pcClass && CompleteClass(pcClass, pcNode)) ? pcClass->FindMemberSymbol(GetComponentName(pcName->GetLast()), eKind, ppcClass) : NULL;
	bAmbiguous = pcSymbol && (pcClass->macBases.NumElements() > 1) && IsAmbiguousInBases(pcClass, GetComponentName(pcName->GetLast()));
	mcDiagnostics.EndSilent();
	if (bAmbiguous)
	{
		//C::T, where two of C's bases each have a T.  [class.member.lookup] 6
		mcDiagnostics.ReportOnce(pcNode, "The name is a member of more than one of the class's bases");
	}
	return pcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name looked up in a class is found as different things in
// its bases, neither hidden by one of the class's own (or of a class
// nearer than both): struct C : A, B { } where A and B each have a T, or
// are two instances of the template that has it.  The same class's member
// reached through two bases is one thing, and one in a virtual base of
// another class that has the name is hidden by that class's (there is one
// of the base).  [class.member.lookup] 3 - 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsAmbiguousInBases(CCPPClass* pcClass, int64 lliName)
{
	CArrayCPPClasses	apcFound;
	CCPPClass*			pcFound;
	size				uiMeanings;
	size				i;
	size				j;
	bool				bHidden;

	mpcStatistics->Count(CPPCC_BaseWalks);
	if (pcClass->macBases.NumElements() == 0)
	{
		return false;
	}
	apcFound.Init();
	FindInBases(pcClass, lliName, &apcFound);
	uiMeanings = 0;
	for (i = 0; i < apcFound.NumElements(); i++)
	{
		pcFound = apcFound.GetPtr(i);
		bHidden = false;
		for (j = 0; !bHidden && (j < apcFound.NumElements()); j++)
		{
			bHidden = (j != i) && apcFound.GetPtr(j)->GetVirtualBase(pcFound);
		}
		uiMeanings += bHidden ? 0 : 1;
	}
	apcFound.Kill();
	return uiMeanings > 1;
}


//////////////////////////////////////////////////////////////////////////
//
// A name not qualified, in a member function, that the parser found as a
// member of a base of the function's class (a type, a static member, an
// enumerator): an error if more than one of the class's bases has it, as
// lookup in the class then finds more than one.  Fields and functions are
// checked where they are bound (CCPPBinder::HasOneMeaning).
// [class.member.lookup] 6
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckBaseMemberName(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSymbol*		pcSymbol;
	CCPPScope*		pcScope;
	CCPPFunction*	pcFunction;
	CCPPClass*		pcClass;

	pcSymbol = pcName->mpcSymbol;
	pcFunction = mcContext.GetFunction();
	pcClass = pcFunction ? pcFunction->mpcClass : NULL;
	if (!pcSymbol || !pcClass || (pcName->macpComponents.NumElements() != 1) || !pcSymbol->mpcScope || (pcClass->macBases.NumElements() < 2))
	{
		return true;
	}
	pcScope = pcSymbol->mpcScope;
	if ((pcScope->meKind == CPPSC_Enum) && pcScope->mpcParent)
	{
		//An unscoped enum's enumerator is its class's member.
		pcScope = pcScope->mpcParent;
	}
	if ((pcScope->meKind != CPPSC_Class) || !pcScope->mpcOwner || (pcScope->mpcOwner->meKind != CPPSK_Class) || (pcClass->mpcSymbol == pcScope->mpcOwner))
	{
		return true;
	}
	return !IsAmbiguousInBases(pcClass, pcSymbol->mlliName) || mcDiagnostics.Error(pcNode, "The name is a member of more than one of the class's bases");
}


//////////////////////////////////////////////////////////////////////////
//
// The classes whose declaration of a name lookup in a class finds: the
// class itself, whose own hides its bases', or else each base's.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::FindInBases(CCPPClass* pcClass, int64 lliName, CArrayCPPClasses* papcFound)
{
	CCPPScope*	pcScope;
	size		i;

	mpcStatistics->Count(CPPCC_BaseWalks);
	pcScope = pcClass->mpcSpecifier ? pcClass->mpcSpecifier->mpcScope : ((pcClass->mpcSymbol) ? pcClass->mpcSymbol->mpcMembers : NULL);
	if (pcScope && pcScope->FindHere(lliName))
	{
		if (papcFound->Find(pcClass) == ARRAY_ELEMENT_NOT_FOUND)
		{
			papcFound->Add(pcClass);
		}
		return;
	}
	if (!ResolveBases(pcClass, NULL))
	{
		return;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		FindInBases(pcClass->macBases.Get(i)->pcClass, lliName, papcFound);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The class a qualified name the parser couldn't resolve names: typename
// C::In or typename C::template In<long>, with C bound to a class, is that
// class's member (MemberClass).  NULL if it isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::QualifiedClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPClass*	pcClass;

	pcClass = (pcName->macpComponents.NumElements() > 1) ? QualifiersClass(pcName, pcNode) : NULL;
	return pcClass ? MemberClass(pcClass, pcName->GetLast(), pcNode) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The member typedef or class of pcClass a name component names, worked out
// with the bindings of the class that declares it, or the instance of its
// member class template (In<long>, its arguments as they are where the name
// is), or the base of pcClass it names.  NULL if it is none of them.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::MemberClass(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode)
{
	CCPPClass*		pcDeclaring;
	CCPPClass*		pcBase;
	CCPPSymbol*		pcMember;
	CCPPTemplate*	pcTemplate;
	SCPP65Type*		psType;
	int64			lliName;

	if (!CompleteClass(pcClass, pcNode))
	{
		return NULL;
	}
	lliName = GetComponentName(pcComponent);
	pcDeclaring = NULL;
	pcMember = pcClass->FindMemberSymbol(lliName, CPPSK_ClassTemplate, &pcDeclaring);
	pcTemplate = (pcMember && pcComponent->mbTemplateArguments) ? MemberClassTemplate(pcMember, pcDeclaring) : NULL;
	if (pcTemplate)
	{
		return InstantiateClass(pcTemplate, &pcComponent->macpTemplateArguments, pcNode);
	}
	pcMember = pcMember ? NULL : pcClass->FindMemberSymbol(lliName, CPPSK_Typedef, &pcDeclaring);
	if (!pcMember)
	{
		pcMember = pcClass->FindMemberSymbol(lliName, CPPSK_Class, &pcDeclaring);
	}
	if (!pcMember || (pcMember == pcDeclaring->mpcSymbol))
	{
		//C::X<char>::Type, where X<char> is C's base: a base's name is found in the class (its injected class name).
		//[class.qual] 1, [temp.local] 1
		pcMember = pcComponent->mpcSymbol;
		pcBase = (pcMember && ((pcMember->meKind == CPPSK_Class) || (pcMember->meKind == CPPSK_ClassTemplate))) ? GetComponentClass(pcComponent, NULL, pcNode) : NULL;
		return (pcBase && IsDerived(pcClass, pcBase, NULL)) ? pcBase : NULL;
	}
	mcContext.EnterBindings(pcDeclaring->mpcBindings);
	if (pcMember->meKind == CPPSK_Typedef)
	{
		psType = mcDeclarators.GetSymbolType(pcMember, pcNode);
		pcClass = (psType && psType->IsClass()) ? psType->pcClass : NULL;
	}
	else
	{
		pcClass = GetComponentClass(pcComponent, pcMember, pcNode);
	}
	mcContext.LeaveBindings();
	return pcClass;
}




//////////////////////////////////////////////////////////////////////////
//
// The class pcName's qualifiers (its components before the last) name.  A component after a class is the member
// typedef, class or class template instance of that name of the class (MemberClass): in
// TEMPLATE::SizePolicy, SizePolicy is a typedef of the class TEMPLATE is bound to, or of one of its bases (the parser
// leaves it unresolved), and in W<int>::Type, Type is W<int>'s typedef, whatever W's T is (the parser found W's).
// NULL if one isn't a class.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::QualifiersClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSyntaxNameComponent*	pcComponent;
	CCPPClass*					pcClass;
	size						i;

	pcClass = NULL;
	for (i = 0; i + 1 < pcName->macpComponents.NumElements(); i++)
	{
		pcComponent = (CCPPSyntaxNameComponent*)pcName->macpComponents.GetPtr(i);
		if (pcComponent->mpcSymbol && (pcComponent->mpcSymbol->meKind == CPPSK_Namespace))
		{
			pcClass = NULL;
			continue;
		}
		pcClass = pcClass ? MemberClass(pcClass, pcComponent, pcNode) : GetComponentClass(pcComponent, NULL, pcNode);
		if (!pcClass)
		{
			return NULL;
		}
	}
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
// The class a qualified name's last qualifier names (GetComponentClass).
// One the parser couldn't resolve is found in the class before it
// (TEXT::SizePolicy::Copy(...)), and so is a member class named through a
// class template's instance, which is that instance's own
// (Outer<char>::Inner::Count()): QualifiersClass.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::NameQualifierClass(CCPPSyntaxName* pcName, CCPPSyntax* pcNode)
{
	CCPPSyntaxNameComponent*	pcQualifier;

	pcQualifier = pcName->GetQualifier();
	if (pcQualifier->mpcSymbol && !((pcQualifier->mpcSymbol->meKind == CPPSK_Class) && pcName->IsQualifiedByInstance()))
	{
		return GetComponentClass(pcQualifier, NULL, pcNode);
	}
	return QualifiersClass(pcName, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is object.*member or pointer->*member.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsMemberPointerOperator(CCPPSyntax* pcExpression)
{
	ECppTokens	eOperator;

	if (pcExpression->GetKind() != CPPS_BinaryExpression)
	{
		return false;
	}
	eOperator = ((CCPPSyntaxBinaryExpression*)pcExpression)->meOperator;
	return (eOperator == CT_MemberFunctionPointerIndirection) || (eOperator == CT_MemberByPointerFunctionPointerIndirection);
}


//////////////////////////////////////////////////////////////////////////
//
// A call to a function whose declaration could not be generated: what
// declaring it said is said now (as if said here: in quiet code it only
// fails).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ReportFailedDeclaration(int64 lliName)
{
	CCPPSyntaxDeclarator*	pcDeclarator;
	size					i;

	for (i = 0; i < masFailed.NumElements(); i++)
	{
		pcDeclarator = masFailed.Get(i)->pcDeclarator;
		if (pcDeclarator->GetName() && (GetFunctionName(pcDeclarator->GetName()->GetLast()) == lliName))
		{
			mcDiagnostics.Repeat(masFailed.Get(i)->pcErrors);
			return;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// TypeOf, a reference as the type it refers to (NULL if it is wrong).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ValueTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = TypeOf(pcExpression);
	return psType ? psType->WithoutReference() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an expression (NULL if it is wrong), worked out without its code by
// CCPP65TypeChecker.  Its errors are dropped: they are reported where its code is
// written (ReportTypeOf).  An expression's type only depends on the template
// arguments and the function it is in, so it is remembered.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::TypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*		psType;
	SCPPTypeOf*	psTypeOf;
	SCPPTypeOf	sTypeOf;

	mpcStatistics->Count(CPPCC_TypeOf);
	psTypeOf = mcTypeOfs.Get((int64)(size)pcExpression);
	if (psTypeOf && (psTypeOf->pcBindings == mcContext.GetBindings()) && (psTypeOf->pcFunction == mcContext.GetFunction()))
	{
		return psTypeOf->psType;
	}

	mpcStatistics->Count(CPPCC_TypeOfWorkedOut);
	mcDiagnostics.BeginKeeping();
	mpcStatistics->Begin(CPPCP_Types);
	psType = mcTypeChecker.Value(pcExpression, &sTypeOf.psDeclared);
	mpcStatistics->End(CPPCP_Types);
	sTypeOf.pcErrors = mcDiagnostics.EndKeeping();

	sTypeOf.pcBindings = mcContext.GetBindings();
	sTypeOf.pcFunction = mcContext.GetFunction();
	sTypeOf.psType = psType;
	mcTypeOfs.Put((int64)(size)pcExpression, &sTypeOf);
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of an operand that isn't evaluated and isn't converted as a
// value is (sizeof's and typeid's): an lvalue stays what it is, so an array
// (a variable, a field, a row of another array, a string) is not the
// pointer it would become, and a function is not a pointer to it.  A name
// is the thing it names (CCPPDeclarators::EntityType), which needs no
// object: S::m alone, and m in a static member function, are the member's
// type.  NULL if it is wrong.  [expr.sizeof] 4, [expr.typeid] 3,
// [expr.prim.general] 13
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::UnevaluatedTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	mcContext.EnterUnevaluated();
	pcExpression = SkipParentheses(pcExpression);
	psType = mcDeclarators.EntityType(pcExpression);
	if (!psType)
	{
		psType = PlaceTypeOf(pcExpression);
	}
	if (!psType)
	{
		psType = TypeOf(pcExpression);
	}
	mcContext.LeaveUnevaluated();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcNode is the operand of a decltype, or the right of a comma that
// is (in brackets or not): what decltype asks only the type of, so a call
// there makes no temporary.  [expr.call] 11, [dcl.type.simple] 5
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsDecltypeOperand(CCPPSyntax* pcNode)
{
	CCPPSyntax*	pcParent;

	for (pcParent = pcNode->mpcParent; pcParent; pcNode = pcParent, pcParent = pcParent->mpcParent)
	{
		switch (pcParent->GetKind())
		{
		case CPPS_DecltypeSpecifier:
			return ((CCPPSyntaxDecltypeSpecifier*)pcParent)->mpcOperand == pcNode;
		case CPPS_ParenthesisedExpression:
			break;
		case CPPS_BinaryExpression:
			if ((((CCPPSyntaxBinaryExpression*)pcParent)->meOperator != CT_Comma) || (((CCPPSyntaxBinaryExpression*)pcParent)->mpcRight != pcNode))
			{
				return false;
			}
			break;
		default:
			return false;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The parameter whose default argument pcNode is written in (not in the
// body of a lambda written there, which is a function of its own), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxParameter* CCPPUnit::DefaultArgumentOf(CCPPSyntax* pcNode)
{
	CCPPSyntax*	pcParent;

	for (pcParent = pcNode->mpcParent; pcParent; pcNode = pcParent, pcParent = pcParent->mpcParent)
	{
		switch (pcParent->GetKind())
		{
		case CPPS_Parameter:
			return (((CCPPSyntaxParameter*)pcParent)->mpcDefault == pcNode) ? (CCPPSyntaxParameter*)pcParent : NULL;
		case CPPS_FunctionDefinition:
		case CPPS_LambdaExpression:
			return NULL;
		default:
			break;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcNode is written in an operand that isn't evaluated, of sizeof,
// decltype, noexcept or typeid, in the function (or lambda) it is in.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsInUnevaluatedOperand(CCPPSyntax* pcNode)
{
	CCPPSyntax*	pcParent;

	for (pcParent = pcNode->mpcParent; pcParent; pcParent = pcParent->mpcParent)
	{
		switch (pcParent->GetKind())
		{
		case CPPS_SizeofExpression:
		case CPPS_DecltypeSpecifier:
		case CPPS_NoexceptExpression:
		case CPPS_TypeidExpression:
			return true;
		case CPPS_FunctionDefinition:
		case CPPS_LambdaExpression:
			return false;
		default:
			break;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is a function's name: the function itself, an
// lvalue, where it isn't converted to a pointer to it.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::NamesFunction(CCPPSyntax* pcExpression)
{
	CCPPSymbol*	pcSymbol;

	if (pcExpression->GetKind() != CPPS_IdExpression)
	{
		return false;
	}
	pcSymbol = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName->mpcSymbol;
	return pcSymbol && (pcSymbol->meKind == CPPSK_Function);
}


//////////////////////////////////////////////////////////////////////////
//
// The type the expression is declared to give: TypeOf's, or the reference
// to it that a call of a function returning T& or T&&, or a cast to one,
// gives.  A variable declared as a reference is its own type here, as its
// name is the thing it refers to.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::DeclaredTypeOf(CCPPSyntax* pcExpression)
{
	SCPPTypeOf*	psTypeOf;

	TypeOf(pcExpression);
	psTypeOf = mcTypeOfs.Get((int64)(size)pcExpression);
	return psTypeOf ? psTypeOf->psDeclared : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the place an lvalue is (an array stays an array), or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::PlaceTypeOf(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	mcDiagnostics.BeginQuiet();
	mpcStatistics->Begin(CPPCP_Types);
	psType = mcTypeChecker.Place(pcExpression);
	mpcStatistics->End(CPPCP_Types);
	mcDiagnostics.EndQuiet();
	return psType;
}


//////////////////////////////////////////////////////////////////////////
//
// Reports what is wrong with an expression whose type the IR front end needed
// and TypeOf couldn't work out, in quiet mode too: a call that no function
// matches, or that more than one does, is then an error where it is ("No
// function matches the arguments": an unsigned short* passed for an unsigned
// long*), not only "doesn't support yet".  The errors are the ones TypeOf
// kept when it worked the type out: nothing is worked out again.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ReportTypeOf(CCPPSyntax* pcExpression)
{
	SCPPTypeOf*	psTypeOf;

	TypeOf(pcExpression);
	psTypeOf = mcTypeOfs.Get((int64)(size)pcExpression);
	mcDiagnostics.Report(psTypeOf ? psTypeOf->pcErrors : NULL);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue can initialise a psTo without losing const or volatile
// (CCPP65TypeChecker::KeepsQualifiers), for the IR front end's initialisers
// and returns.  It is an error where it is, in quiet mode too, as ReportTypeOf's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckInitialiser(CCPPSyntax* pcValue, SCPP65Type* psTo)
{
	bool	bKept;

	mcDiagnostics.BeginLoud();
	bKept = mcTypeChecker.KeepsQualifiers(pcValue, psTo);
	mcDiagnostics.EndLoud();
	return bKept;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcValue, the one value in braces that initialises a psTo, is not
// narrowed by its conversion (CCPP65TypeChecker::NotNarrowed), for the
// variables' initialisers the unit converts itself.  An error
// where it is, as CheckInitialiser's.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckBracedValue(CCPPSyntax* pcValue, SCPP65Type* psTo)
{
	bool	bNotNarrowed;

	mcDiagnostics.BeginLoud();
	bNotNarrowed = mcTypeChecker.NotNarrowed(pcValue, psTo);
	mcDiagnostics.EndLoud();
	return bNotNarrowed;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::Supported(SCPP65Type* psType, CCPPSyntax* pcNode)
{
	if (psType->IsScalar() && (psType->iBytes > 8))
	{
		return mcDiagnostics.Error(pcNode, "Integers wider than 64 bits are not supported by the compiler");
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// A local declared constexpr is initialised with a constant expression:
// one of an integer type is worked out here, and any other as a global's is
// (CCPPConstants::CheckLocal), and is an error (and false) where it has no
// value.  The local is still made when the function runs.
// [dcl.constexpr] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CheckConstexprLocal(CCPPVariable* pcVariable, CCPPSyntaxInitDeclarator* pcInit)
{
	CChars	szError;
	int64	lliValue;

	if (!pcInit->mpcInitializer)
	{
		return mcDiagnostics.ReportError(pcInit, "A constexpr variable needs an initialiser");
	}
	if (!pcVariable->mpsType->IsInteger())
	{
		return mpcConstants->CheckLocal(pcVariable, pcInit);
	}
	if (mcFolder.Constant(pcInit->mpcInitializer, &lliValue))
	{
		return true;
	}
	szError.Init();
	mcFolder.ConstantMessage("A constexpr variable's initialiser must be a constant", &szError);
	mcDiagnostics.ReportError(pcInit, szError.Text());
	szError.Kill();
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// What is in the brackets or braces of Type(arguments) or Type{arguments}, or
// NULL if it isn't either.
//
//////////////////////////////////////////////////////////////////////////
CArraySyntax* CCPPUnit::ConstructElements(CCPPSyntaxConstructExpression* pcConstruct)
{
	CCPPSyntax*	pcInitialiser;

	pcInitialiser = pcConstruct->mpcInitializer;
	if (pcInitialiser && ((pcInitialiser->GetKind() == CPPS_ParenthesisedInitializer) || (pcInitialiser->GetKind() == CPPS_InitializerList)))
	{
		return ListElements(pcInitialiser);
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if pcExpression is an integer constant that an implicit conversion to
// psTo truncates: it fits neither psTo's signed nor its unsigned range (so -1
// to an unsigned type and 0xFFFF to a 16 bit signed one are bit patterns, not
// warned about).  89037124 passed to an int parameter is.  With
// SetWarnNarrowing a non constant integer converted to a narrower type is
// warned about too: a 32 bit index stored in a 16 bit one.  A multiply or
// left shift converted to a wider type is warned about (WideningArithmetic).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ConversionOverflow(CCPPSyntax* pcExpression, SCPP65Type* psTo)
{
	SCPP65Type*	psFrom;
	int64		lliValue;
	bool		bConstant;
	CChars		szMessage;

	if (!psTo || !psTo->IsInteger() || (psTo->eKind == CPP65TK_Bool) || !mcContext.EnterWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	psFrom = TypeOf(pcExpression);
	mcContext.LeaveWorking();
	if (!psFrom || !psFrom->IsInteger())
	{
		return;
	}

	//Most expressions aren't constant; looking must not leave an error.
	bConstant = QuietConstant(pcExpression, &lliValue);
	if (!bConstant)
	{
		WideningArithmetic(pcExpression, psFrom, psTo);
		if (mpcOptions->mbWarnNarrowing && (psFrom->eKind == CPP65TK_Integer) && (psFrom->iBytes > psTo->iBytes))
		{
			szMessage.Init("Integer conversion narrows: a ");
			szMessage.Append(psFrom->iBytes * 8)->Append(" bit value is converted to the ")->Append(psTo->iBytes * 8)->Append(" bit type and may be truncated.");
			mcDiagnostics.Warning(pcExpression, szMessage.Text());
			szMessage.Kill();
		}
		return;
	}
	if (psTo->iBytes >= 8)
	{
		return;
	}

	//It fits the type's width, as signed or as unsigned.
	if ((lliValue >= CPPIntegerLeast(psTo->iBytes, true)) && (lliValue <= CPPIntegerMost(psTo->iBytes, false)))
	{
		return;
	}

	szMessage.Init("Integer conversion overflow: the constant ");
	szMessage.Append(lliValue)->Append(" doesn't fit the ")->Append(psTo->iBytes * 8)->Append(" bit type it is converted to and is truncated.");
	mcDiagnostics.Warning(pcExpression, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if pcExpression (of type psFrom) is a multiply or a left shift done
// in an integer type narrower than psTo, which it is implicitly converted
// to: the result may have wrapped before it was widened.  iKey = ui * 0x39e7
// with a 16 bit ui and a 64 bit iKey multiplies in 16 bits.  Converting an
// operand first ((int64)ui * 0x39e7) does the arithmetic in the wider type.
// An explicit cast of the result ((int64)(ui * 0x39e7)) is warned about too:
// it doesn't make the multiply wider.  Constants are ConstantOverflow's.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::WideningArithmetic(CCPPSyntax* pcExpression, SCPP65Type* psFrom, SCPP65Type* psTo)
{
	CCPPSyntaxBinaryExpression*	pcBinary;
	CChars						szMessage;
	int64						lliValue;
	bool						bConstant;

	if ((psFrom->eKind != CPP65TK_Integer) || (psTo->eKind != CPP65TK_Integer) || (psTo->iBytes <= psFrom->iBytes) || mcContext.IsWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	pcBinary = NarrowMultiply(pcExpression, psFrom);
	if (!pcBinary)
	{
		return;
	}

	bConstant = QuietConstant(pcBinary, &lliValue);
	if (bConstant)
	{
		return;
	}

	szMessage.Init("Integer arithmetic widened after it may have wrapped: the ");
	szMessage.Append(psFrom->iBytes * 8)->Append(" bit result of '")->Append((pcBinary->meOperator == CT_Multiply) ? "*" : "<<");
	szMessage.Append("' is converted to the ")->Append(psTo->iBytes * 8)->Append(" bit type.  Convert an operand first to do it in ")->Append(psTo->iBytes * 8)->Append(" bits.");
	mcDiagnostics.Warning(pcExpression, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if a shift's count is a constant not less than iBits, the width of
// the (promoted) value shifted: the result is undefined.  On the W65C816 an
// int is 16 bits, so a uint8 or int promoted to it and shifted by 16 or more
// ((uint32)(b << 16) with a uint8 b) is zero, not the 32 bit value meant.
// Converting the operand first ((uint32)b << 16) shifts in the wider type.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ShiftCount(CCPPSyntaxBinaryExpression* pcBinary, int iBits)
{
	CChars	szMessage;
	int64	lliCount;
	bool	bConstant;

	if (mcContext.IsWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	bConstant = QuietConstant(pcBinary->mpcRight, &lliCount);
	if (!bConstant || (lliCount < iBits))
	{
		return;
	}

	szMessage.Init("Shift count ");
	szMessage.Append(lliCount)->Append(" is not less than the ")->Append(iBits)->Append(" bit width of the value shifted, so the result is undefined.  Convert the operand first to shift in a wider type.");
	mcDiagnostics.Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The multiply or left shift WideningArithmetic warns about in pcExpression:
// the expression itself, or one under +, -, |, ^ or & that is done in the
// same type (psType): y * uiRowStride + x * uiPixelStride with 16 bit
// operands made a 16 bit offset before it was widened to the pointer's
// ptrdiff.  NULL if there isn't one.
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntaxBinaryExpression* CCPPUnit::NarrowMultiply(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
	CCPPSyntax*					pcInner;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxBinaryExpression*	pcFound;
	SCPP65Type*					psSide;
	int							i;

	pcInner = SkipParentheses(pcExpression);
	if (!pcInner || (pcInner->GetKind() != CPPS_BinaryExpression))
	{
		return NULL;
	}
	pcBinary = (CCPPSyntaxBinaryExpression*)pcInner;
	if ((pcBinary->meOperator == CT_Multiply) || (pcBinary->meOperator == CT_LeftShift))
	{
		return pcBinary;
	}
	if ((pcBinary->meOperator != CT_Add) && (pcBinary->meOperator != CT_Subtract) && (pcBinary->meOperator != CT_BitwiseOr) &&
		(pcBinary->meOperator != CT_BitwiseXor) && (pcBinary->meOperator != CT_BitwiseAnd))
	{
		return NULL;
	}

	//Only a side already the type the whole is done in: a wider side would have made the whole wider.
	for (i = 0; i < 2; i++)
	{
		mcContext.EnterWorking(CPPWK_Warning, NULL);
		psSide = TypeOf((i == 0) ? pcBinary->mpcLeft : pcBinary->mpcRight);
		mcContext.LeaveWorking();
		if (!psSide || (psSide->eKind != CPP65TK_Integer) || (psSide->iBytes != psType->iBytes))
		{
			continue;
		}
		pcFound = NarrowMultiply((i == 0) ? pcBinary->mpcLeft : pcBinary->mpcRight, psType);
		if (pcFound)
		{
			return pcFound;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if a for loop's counter (a variable its increment steps with ++,
// --, += or -=) is compared in its condition with a wider integer that isn't
// a constant: for (size i = 0; i < aoi.NumElements(); i++) with a 16 bit i
// and a 32 bit count wraps i at 65535 and goes round forever once the count
// passes it.  Converting the bound to the counter's type ((size)uiCount) says
// it fits.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::LoopCounterWidth(CCPPSyntaxForStatement* pcFor)
{
	CArrayVoidPtr	apcCounters;

	if (!pcFor->mpcCondition || !pcFor->mpcIncrement || mcContext.IsWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	apcCounters.Init();
	LoopCounters(pcFor->mpcIncrement, &apcCounters);
	if (apcCounters.NumElements() > 0)
	{
		LoopCounterComparisons(pcFor->mpcCondition, &apcCounters);
	}
	apcCounters.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The variables pcIncrement steps (through commas): i++, --i, i += 2.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::LoopCounters(CCPPSyntax* pcIncrement, CArrayVoidPtr* papcCounters)
{
	CCPPSyntax*					pcInner;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntaxUnaryExpression*	pcUnary;
	CCPPSymbol*					pcSymbol;

	pcInner = SkipParentheses(pcIncrement);
	if (!pcInner)
	{
		return;
	}
	pcSymbol = NULL;
	if (pcInner->GetKind() == CPPS_UnaryExpression)
	{
		pcUnary = (CCPPSyntaxUnaryExpression*)pcInner;
		if ((pcUnary->meOperator == CT_Increment) || (pcUnary->meOperator == CT_Decrement))
		{
			pcSymbol = LoopCounterSymbol(pcUnary->mpcOperand);
		}
	}
	else if (pcInner->GetKind() == CPPS_BinaryExpression)
	{
		pcBinary = (CCPPSyntaxBinaryExpression*)pcInner;
		if (pcBinary->meOperator == CT_Comma)
		{
			LoopCounters(pcBinary->mpcLeft, papcCounters);
			LoopCounters(pcBinary->mpcRight, papcCounters);
		}
		else if ((pcBinary->meOperator == CT_AssignmentAdd) || (pcBinary->meOperator == CT_AssignmentSubtract))
		{
			pcSymbol = LoopCounterSymbol(pcBinary->mpcLeft);
		}
	}
	if (pcSymbol)
	{
		papcCounters->Add(pcSymbol);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The comparisons in pcCondition (through && and ||) of a counter with a
// wider integer that isn't a constant.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::LoopCounterComparisons(CCPPSyntax* pcCondition, CArrayVoidPtr* papcCounters)
{
	CCPPSyntax*					pcInner;
	CCPPSyntaxBinaryExpression*	pcBinary;
	CCPPSyntax*					pcCounter;
	CCPPSyntax*					pcBound;
	CCPPSymbol*					pcSymbol;
	SCPP65Type*					psCounter;
	SCPP65Type*					psBound;
	int64						lliValue;
	bool						bConstant;
	CChars						szMessage;
	ECppTokens					eOperator;

	pcInner = SkipParentheses(pcCondition);
	if (!pcInner || (pcInner->GetKind() != CPPS_BinaryExpression))
	{
		return;
	}
	pcBinary = (CCPPSyntaxBinaryExpression*)pcInner;
	eOperator = pcBinary->meOperator;
	if ((eOperator == CT_LogicalAnd) || (eOperator == CT_LogicalOr))
	{
		LoopCounterComparisons(pcBinary->mpcLeft, papcCounters);
		LoopCounterComparisons(pcBinary->mpcRight, papcCounters);
		return;
	}
	if ((eOperator != CT_LessThan) && (eOperator != CT_LessThanEqualTo) && (eOperator != CT_GreaterThan) &&
		(eOperator != CT_GreaterThanEqualTo) && (eOperator != CT_NotEqualTo))
	{
		return;
	}

	pcSymbol = LoopCounterSymbol(pcBinary->mpcLeft);
	if (pcSymbol && (papcCounters->Find(pcSymbol) != ARRAY_ELEMENT_NOT_FOUND))
	{
		pcCounter = pcBinary->mpcLeft;
		pcBound = pcBinary->mpcRight;
	}
	else
	{
		pcSymbol = LoopCounterSymbol(pcBinary->mpcRight);
		if (!pcSymbol || (papcCounters->Find(pcSymbol) == ARRAY_ELEMENT_NOT_FOUND))
		{
			return;
		}
		pcCounter = pcBinary->mpcRight;
		pcBound = pcBinary->mpcLeft;
	}

	mcContext.EnterWorking(CPPWK_Warning, NULL);
	psCounter = TypeOf(pcCounter);
	psBound = TypeOf(pcBound);
	mcContext.LeaveWorking();
	if (!psCounter || !psBound || (psCounter->eKind != CPP65TK_Integer) || (psBound->eKind != CPP65TK_Integer) || (psBound->iBytes <= psCounter->iBytes))
	{
		return;
	}

	//A constant bound is ComparisonRange's, and a choice of constants (b ? 7 : 3) is two of them.
	mcDiagnostics.BeginTrial();
	pcInner = SkipParentheses(pcBound);
	if (pcInner && (pcInner->GetKind() == CPPS_ConditionalExpression))
	{
		bConstant = mcFolder.Constant(((CCPPSyntaxConditionalExpression*)pcInner)->mpcTrue, &lliValue) &&
					mcFolder.Constant(((CCPPSyntaxConditionalExpression*)pcInner)->mpcFalse, &lliValue);
	}
	else
	{
		bConstant = mcFolder.Constant(pcBound, &lliValue);
	}
	mcDiagnostics.EndTrial();
	if (bConstant || mcDiagnostics.HasWarning(pcBinary, "Loop counter"))
	{
		return;
	}

	szMessage.Init("Loop counter narrower than its bound: the ");
	szMessage.Append(psCounter->iBytes * 8)->Append(" bit counter is compared with a ")->Append(psBound->iBytes * 8);
	szMessage.Append(" bit value and wraps before it can pass one that doesn't fit.  Widen the counter, or convert the bound if it always fits.");
	mcDiagnostics.Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// The variable pcExpression names (i, or a member m), or NULL.
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::LoopCounterSymbol(CCPPSyntax* pcExpression)
{
	CCPPSyntax*				pcInner;
	CCPPSyntaxIdExpression*	pcId;

	pcInner = SkipParentheses(pcExpression);
	if (!pcInner || (pcInner->GetKind() != CPPS_IdExpression))
	{
		return NULL;
	}
	pcId = (CCPPSyntaxIdExpression*)pcInner;
	if (!pcId->mpcName)
	{
		return NULL;
	}
	return pcId->mpcName->mpcSymbol;
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if one side of an integer comparison is a constant outside the range
// of the other side's type, so the comparison is always true or always false:
// a 16 bit size compared with 0xFFFFFFFF (a 32 bit "not found").  The constant
// is converted to the comparison's type first, so -1 compared with a 16 bit
// unsigned int is 0xFFFF and fine.  A side the comparison's type can't hold
// every value of (signed compared as unsigned, or a 32 bit unsigned enum
// compared as a 32 bit long) wraps, so it isn't looked at.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ComparisonRange(CCPPSyntaxBinaryExpression* pcBinary)
{
	SCPP65Type*		psLeft;
	SCPP65Type*		psRight;
	SCPP65Type*		psVariable;
	SCPP65Type*		psCommon;
	CCPPSyntax*		pcConstant;
	int64			lliValue;
	int64			lliMin;
	int64			lliMax;
	bool			bConstant;
	bool			bAlways;
	CChars			szMessage;

	if (!mcContext.EnterWorking(CPPWK_Warning, NULL))
	{
		return;
	}
	psLeft = TypeOf(pcBinary->mpcLeft);
	psRight = TypeOf(pcBinary->mpcRight);
	mcContext.LeaveWorking();
	if (!psLeft || !psRight || (psLeft->eKind != CPP65TK_Integer) || (psRight->eKind != CPP65TK_Integer))
	{
		return;
	}

	//Most comparisons have no constant side; looking must not leave an error.
	mcDiagnostics.BeginTrial();
	pcConstant = pcBinary->mpcRight;
	psVariable = psLeft;
	bConstant = mcFolder.Constant(pcConstant, &lliValue);
	if (!bConstant)
	{
		pcConstant = pcBinary->mpcLeft;
		psVariable = psRight;
		bConstant = mcFolder.Constant(pcConstant, &lliValue) && !mcFolder.Constant(pcBinary->mpcRight, &lliMin);
	}
	else if (mcFolder.Constant(pcBinary->mpcLeft, &lliMin))
	{
		bConstant = false;
	}
	mcDiagnostics.EndTrial();
	if (!bConstant)
	{
		return;
	}

	psCommon = mcTypes.Arithmetic(psLeft, psRight);
	if (!psCommon || (psCommon->iBytes >= 8) || (psVariable->bSigned && !psCommon->bSigned) || (!psVariable->bSigned && psCommon->bSigned && (psCommon->iBytes <= psVariable->iBytes)))
	{
		return;
	}

	//The constant as the comparison's type sees it.
	lliValue = mcFolder.ConstantOfType(psCommon, lliValue);

	lliMin = psVariable->Least();
	lliMax = psVariable->Most();
	if ((lliValue >= lliMin) && (lliValue <= lliMax))
	{
		ComparisonAtLimit(pcBinary, pcConstant, psVariable, lliValue, lliMin, lliMax);
		return;
	}

	//Out of range: the result is the same for every value of the variable side.
	switch (pcBinary->meOperator)
	{
	case CT_EqualTo:				bAlways = false;	break;
	case CT_NotEqualTo:				bAlways = true;		break;
	case CT_LessThan:
	case CT_LessThanEqualTo:		bAlways = (lliValue > lliMax) == (pcConstant == pcBinary->mpcRight);	break;
	case CT_GreaterThan:
	case CT_GreaterThanEqualTo:		bAlways = (lliValue < lliMin) == (pcConstant == pcBinary->mpcRight);	break;
	default:						return;
	}

	szMessage.Init("Comparison is always ");
	szMessage.Append(bAlways ? "true" : "false")->Append(": the constant ")->Append(lliValue)->Append(" is out of the range of the ")->Append(psVariable->iBytes * 8)->Append(" bit ")->Append(psVariable->bSigned ? "signed" : "unsigned")->Append(" value it is compared with.");
	mcDiagnostics.Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Warns if a <= (or >=) comparison's constant is the largest value the
// variable side's type holds (or a signed type's smallest), so it is always
// true: for (i = -32768; i <= 32767; i++) with a 16 bit int never ends.
// Always false comparisons at a limit (ui > MAX_UINT32, a guard for wider
// builds) and an unsigned value compared with 0 (u >= 0, written to say what
// is expected) aren't warned about.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::ComparisonAtLimit(CCPPSyntaxBinaryExpression* pcBinary, CCPPSyntax* pcConstant, SCPP65Type* psVariable, int64 lliValue, int64 lliMin, int64 lliMax)
{
	ECppTokens	eOperator;
	bool		bAlways;
	CChars		szMessage;

	//As variable OP constant.
	eOperator = pcBinary->meOperator;
	if (pcConstant == pcBinary->mpcLeft)
	{
		switch (eOperator)
		{
		case CT_LessThan:				eOperator = CT_GreaterThan;			break;
		case CT_LessThanEqualTo:		eOperator = CT_GreaterThanEqualTo;	break;
		case CT_GreaterThan:			eOperator = CT_LessThan;			break;
		case CT_GreaterThanEqualTo:		eOperator = CT_LessThanEqualTo;		break;
		default:						break;
		}
	}

	if ((lliValue == lliMax) && (eOperator == CT_LessThanEqualTo))
	{
		bAlways = true;
	}
	else if (psVariable->bSigned && (lliValue == lliMin) && (eOperator == CT_GreaterThanEqualTo))
	{
		bAlways = true;
	}
	else
	{
		return;
	}

	szMessage.Init("Comparison is always ");
	szMessage.Append(bAlways ? "true" : "false")->Append(": the constant ")->Append(lliValue)->Append(" is the ")->Append((lliValue == lliMax) ? "largest" : "smallest");
	szMessage.Append(" value of the ")->Append(psVariable->iBytes * 8)->Append(" bit ")->Append(psVariable->bSigned ? "signed" : "unsigned")->Append(" value it is compared with.");
	mcDiagnostics.Warning(pcBinary, szMessage.Text());
	szMessage.Kill();
}


//The compiler's type traits of one type.
enum ECPP65TypeTrait
{
	CPP65TT_None,
	CPP65TT_Class,
	CPP65TT_Union,
	CPP65TT_Enum,
	CPP65TT_Empty,
	CPP65TT_Polymorphic,
	CPP65TT_Abstract,
	CPP65TT_TrivialConstructor,
	CPP65TT_TrivialDestructor,
	CPP65TT_Trivial,
	CPP65TT_TriviallyCopyable,
	CPP65TT_Pod,
	CPP65TT_StandardLayout,
	CPP65TT_LiteralType,
	CPP65TT_Final,
	CPP65TT_BaseOf,
	CPP65TT_ConvertibleTo,
	CPP65TT_Constructible,
	CPP65TT_Assignable,
};


struct SCPP65TypeTraitName
{
	const char*			szName;
	ECPP65TypeTrait		eTrait;
};


static SCPP65TypeTraitName gasCPP65TypeTraits[] =
{
	{ "__is_class", CPP65TT_Class },
	{ "__is_union", CPP65TT_Union },
	{ "__is_enum", CPP65TT_Enum },
	{ "__is_empty", CPP65TT_Empty },
	{ "__is_polymorphic", CPP65TT_Polymorphic },
	{ "__is_abstract", CPP65TT_Abstract },
	{ "__is_trivially_constructible", CPP65TT_TrivialConstructor },
	{ "__has_trivial_constructor", CPP65TT_TrivialConstructor },
	{ "__is_trivially_destructible", CPP65TT_TrivialDestructor },
	{ "__has_trivial_destructor", CPP65TT_TrivialDestructor },
	{ "__is_pod", CPP65TT_Pod },
	{ "__is_trivial", CPP65TT_Trivial },
	{ "__is_trivially_copyable", CPP65TT_TriviallyCopyable },
	{ "__has_trivial_copy", CPP65TT_TriviallyCopyable },
	{ "__is_standard_layout", CPP65TT_StandardLayout },
	{ "__is_literal_type", CPP65TT_LiteralType },
	{ "__is_literal", CPP65TT_LiteralType },
	{ "__is_final", CPP65TT_Final },
	{ "__is_sealed", CPP65TT_Final },
	{ "__is_base_of", CPP65TT_BaseOf },
	{ "__is_convertible_to", CPP65TT_ConvertibleTo },
	{ "__is_convertible", CPP65TT_ConvertibleTo },
	{ "__is_constructible", CPP65TT_Constructible },
	{ "__is_assignable", CPP65TT_Assignable },
};


//////////////////////////////////////////////////////////////////////////
//
// The type trait called pszName, CPP65TT_None if there is none this
// compiler knows.
//
//////////////////////////////////////////////////////////////////////////
static ECPP65TypeTrait CPP65FindTypeTrait(CChars* pszName)
{
	size	ui;

	for (ui = 0; ui < sizeof(gasCPP65TypeTraits) / sizeof(gasCPP65TypeTraits[0]); ui++)
	{
		if (pszName->Equals(gasCPP65TypeTraits[ui].szName))
		{
			return gasCPP65TypeTraits[ui].eTrait;
		}
	}
	return CPP65TT_None;
}


//////////////////////////////////////////////////////////////////////////
//
// A compiler type trait's value (see gasCPP65TypeTraits), of its types.  A
// class is abstract when one of its virtual functions is still pure; it is
// trivially copyable when copying, assigning and destroying it is only its
// bytes (nothing the compiler writes runs, and it has no vtable), trivial
// when making one is too, standard layout when its fields are laid out as
// C's are, and a POD when both.  [meta.unary.prop]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::TypeTrait(CCPPSyntaxTypeTraitExpression* pcTrait, int64* plliValue)
{
	CArrayCPP65Types	apsTypes;
	SCPP65Type*			psType;
	CCPPClass*			pcClass;
	CChars				szTrait;
	ECPP65TypeTrait		eTrait;
	size				i;
	bool				bResult;

	szTrait.Init();
	mpcTokeniser->GetSource(mpcTokeniser->Get(pcTrait->miName))->Append(&szTrait);
	eTrait = CPP65FindTypeTrait(&szTrait);
	szTrait.Kill();
	if (eTrait == CPP65TT_None)
	{
		return false;
	}

	apsTypes.Init();
	bResult = pcTrait->macpArguments.NumElements() >= 1;
	for (i = 0; bResult && (i < pcTrait->macpArguments.NumElements()); i++)
	{
		psType = (pcTrait->macpArguments.GetPtr(i)->GetKind() == CPPS_TypeId) ? mcDeclarators.GetTypeIdType((CCPPSyntaxTypeId*)pcTrait->macpArguments.GetPtr(i)) : NULL;
		pcClass = (psType && psType->WithoutReference()->IsClass()) ? psType->WithoutReference()->pcClass : NULL;
		bResult = psType && (!pcClass || CompleteClass(pcClass, pcTrait));
		if (bResult)
		{
			apsTypes.Add(psType);
		}
	}
	if (bResult && (apsTypes.NumElements() < (size)(((eTrait == CPP65TT_BaseOf) || (eTrait == CPP65TT_ConvertibleTo) || (eTrait == CPP65TT_Assignable)) ? 2 : 1)))
	{
		bResult = mcDiagnostics.Error(pcTrait, "The type trait needs more types");
	}
	if (bResult)
	{
		*plliValue = TypeTraitValue(eTrait, &apsTypes);
	}
	apsTypes.Kill();
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
// A type trait's value for its types (each complete).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::TypeTraitValue(int iTrait, CArrayCPP65Types* papsTypes)
{
	SCPP65Type*			psType;
	SCPP65Type*			psOther;
	CCPPClass*			pcClass;
	CArrayCPP65Types	apsArguments;
	size				i;
	bool				bResult;

	psType = papsTypes->GetPtr(0);
	psOther = (papsTypes->NumElements() > 1) ? papsTypes->GetPtr(1) : NULL;
	pcClass = psType->IsClass() ? psType->pcClass : NULL;
	switch ((ECPP65TypeTrait)iTrait)
	{
	case CPP65TT_Class:					return pcClass && !pcClass->mbUnion;
	case CPP65TT_Union:					return pcClass && pcClass->mbUnion;
	case CPP65TT_Enum:					return psType->pcEnum != NULL;
	case CPP65TT_Empty:					return pcClass && pcClass->mbEmpty;
	case CPP65TT_Polymorphic:			return pcClass && pcClass->IsPolymorphic();
	case CPP65TT_Abstract:				return pcClass && pcClass->IsAbstract();
	case CPP65TT_Final:					return pcClass && pcClass->mpcSpecifier && pcClass->mpcSpecifier->mbFinal;
	case CPP65TT_TrivialConstructor:	return !pcClass || !pcClass->NeedsConstructor();
	case CPP65TT_TrivialDestructor:		return !pcClass || !pcClass->NeedsDestructor();
	case CPP65TT_TriviallyCopyable:		return IsTriviallyCopyableType(psType);
	case CPP65TT_Trivial:				return IsTriviallyCopyableType(psType) && (!pcClass || !pcClass->NeedsConstructor());
	case CPP65TT_StandardLayout:		return IsStandardLayoutType(psType);
	case CPP65TT_Pod:					return IsTriviallyCopyableType(psType) && (!pcClass || !pcClass->NeedsConstructor()) && IsStandardLayoutType(psType);
	case CPP65TT_LiteralType:			return mpcConstants->IsLiteralType(psType);
	case CPP65TT_BaseOf:
		//A class is its own base here.  [meta.rel] Table 49
		return pcClass && psOther->IsClass() && ((pcClass == psOther->pcClass) || IsDerived(psOther->pcClass, pcClass, NULL));
	case CPP65TT_ConvertibleTo:
		return mcBinder.IsConvertible(psOther, psType);
	case CPP65TT_Constructible:
		apsArguments.Init();
		for (i = 1; i < papsTypes->NumElements(); i++)
		{
			apsArguments.Add(papsTypes->GetPtr(i));
		}
		bResult = mcBinder.IsConstructible(psType, &apsArguments);
		apsArguments.Kill();
		return bResult;
	case CPP65TT_Assignable:
		return mcBinder.IsAssignable(psType, psOther);
	default:
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if copying, assigning and destroying an object of the type is only
// its bytes.  [basic.types] 9
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsTriviallyCopyableType(SCPP65Type* psType)
{
	CCPPClass*	pcClass;

	while (psType->IsArray())
	{
		psType = psType->psBase;
	}
	if (!psType->IsClass())
	{
		return !psType->IsReference();
	}
	pcClass = psType->pcClass;
	return pcClass->IsTriviallyCopyable() && pcClass->IsTriviallyAssignable() && !pcClass->NeedsDestructor() && !pcClass->IsPolymorphic();
}


//////////////////////////////////////////////////////////////////////////
//
// True if the type is laid out as C's types are: no vtable, no reference,
// standard layout fields and bases, and fields in only one class of its
// hierarchy.  [class] 7
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsStandardLayoutType(SCPP65Type* psType)
{
	CCPPClass*	pcClass;
	CCPPClass*	pcFields;
	size		i;

	while (psType->IsArray())
	{
		psType = psType->psBase;
	}
	if (!psType->IsClass())
	{
		return !psType->IsReference();
	}
	pcClass = psType->pcClass;
	if (pcClass->IsPolymorphic())
	{
		return false;
	}
	for (i = 0; i < pcClass->macFields.NumElements(); i++)
	{
		if (!IsStandardLayoutType(pcClass->macFields.Get(i)->psType))
		{
			return false;
		}
	}
	pcFields = (pcClass->macFields.NumElements() > 0) ? pcClass : NULL;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		if (!IsStandardLayoutType(pcClass->macBases.Get(i)->pcClass->mpsType))
		{
			return false;
		}
		if (!pcClass->macBases.Get(i)->pcClass->mbEmpty)
		{
			if (pcFields)
			{
				return false;
			}
			pcFields = pcClass->macBases.Get(i)->pcClass;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// An enumerator's type: its enum's (an unnamed enum's is its underlying type).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::EnumeratorType(CCPPSymbol* pcSymbol)
{
	CCPPSyntax*					pcEnumerator;
	CCPPSyntaxEnumSpecifier*	pcSpecifier;

	pcEnumerator = pcSymbol->mpcDeclaration;
	if (!pcEnumerator || !pcEnumerator->mpcParent || (pcEnumerator->mpcParent->GetKind() != CPPS_EnumSpecifier))
	{
		return &mcTypes.msInt;
	}
	pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcEnumerator->mpcParent;
	return EnumType(pcSpecifier->mpcSymbol, pcSpecifier);
}


//////////////////////////////////////////////////////////////////////////
//
// An enum's type (pcEnum's, or an unnamed enum's from pcSpecifier).  Its
// underlying type is the one given ("enum E : uint8"), int for a scoped enum,
// or else [dcl.enum] the first of int, unsigned int, long, unsigned long and
// long long that holds all its enumerators: TRIERROR = 0xffffffff makes
// TRISTATE an unsigned long on the W65C816, where int is 16 bits.
// pcSpecifier can be NULL or a forward declaration.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::EnumType(CCPPSymbol* pcEnum, CCPPSyntaxEnumSpecifier* pcSpecifier)
{
	SCPP65Type*				psType;
	SCPP65Type*				psBase;
	CCPPSyntaxEnumerator*	pcEnumerator;
	CCPPSymbol**			ppcMember;
	SMapIterator			sIter;
	CCPPSyntax*				pcDeclaration;
	int64					lliValue;
	int64					lliMin;
	int64					lliMax;
	int						iBytes;
	bool					bSigned;
	bool					bNonNegative;
	size					i;

	bNonNegative = false;
	if (pcEnum)
	{
		psType = mcTypes.FindEnum(pcEnum);
		if (psType)
		{
			return psType;
		}
	}

	//The specifier with the enumerators: the symbol's declaration, or the parent of one of its enumerators.
	if ((!pcSpecifier || !pcSpecifier->mbBody) && pcEnum)
	{
		pcSpecifier = NULL;
		pcDeclaration = pcEnum->mpcDeclaration;
		if (pcDeclaration && (pcDeclaration->GetKind() == CPPS_EnumSpecifier) && ((CCPPSyntaxEnumSpecifier*)pcDeclaration)->mbBody)
		{
			pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcDeclaration;
		}
		else if (pcEnum->mpcMembers && pcEnum->mpcMembers->mcSymbols.StartIteration(&sIter, NULL, NULL, (void**)&ppcMember, NULL) && ppcMember && *ppcMember)
		{
			pcDeclaration = (*ppcMember)->mpcDeclaration;
			if (pcDeclaration && pcDeclaration->mpcParent && (pcDeclaration->mpcParent->GetKind() == CPPS_EnumSpecifier))
			{
				pcSpecifier = (CCPPSyntaxEnumSpecifier*)pcDeclaration->mpcParent;
			}
		}
	}

	iBytes = mcTypes.msInt.iBytes;
	bSigned = true;
	psBase = NULL;
	if (pcSpecifier && pcSpecifier->mpcBase)
	{
		//Fixed: enum E : short is a short underneath, though int is as wide.  Each value given is one of that type's.
		//[dcl.enum] 5
		psBase = mcDeclarators.GetTypeIdType(pcSpecifier->mpcBase);
		psBase = (psBase && psBase->IsInteger()) ? psBase->Unqualified() : NULL;
		for (i = 0; psBase && (i < pcSpecifier->macpEnumerators.NumElements()); i++)
		{
			pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i);
			if (pcEnumerator->mpcValue && mcFolder.Constant(pcEnumerator->mpcValue, &lliValue))
			{
				CheckEnumeratorFits(pcSpecifier, pcEnumerator, lliValue);
			}
		}
	}
	else if (pcSpecifier && !pcSpecifier->mbScoped)
	{
		lliValue = 0;
		lliMin = 0;
		lliMax = 0;
		bNonNegative = true;
		for (i = 0; i < pcSpecifier->macpEnumerators.NumElements(); i++)
		{
			pcEnumerator = (CCPPSyntaxEnumerator*)pcSpecifier->macpEnumerators.GetPtr(i);
			if (pcEnumerator->mpcValue && !QuietConstant(pcEnumerator->mpcValue, &lliValue))
			{
				//One that can't be worked out here (sizeof(T) in a class template's enum, with no T) leaves it an int:
				//what is wrong with it is said where it is checked (CheckEnumerators).
				break;
			}
			lliMin = (lliValue < lliMin) ? lliValue : lliMin;
			lliMax = (lliValue > lliMax) ? lliValue : lliMax;
			lliValue++;
		}
		bNonNegative = (i == pcSpecifier->macpEnumerators.NumElements()) && (lliMin >= 0);

		if ((lliMin >= mcTypes.msInt.Least()) && (lliMax <= mcTypes.msInt.Most()))
		{
			//int.
		}
		else if ((lliMin >= 0) && (lliMax <= mcTypes.msUnsignedInt.Most()))
		{
			bSigned = false;
		}
		else if ((lliMin >= CPPIntegerLeast(4, true)) && (lliMax <= CPPIntegerMost(4, true)))
		{
			iBytes = 4;
		}
		else if ((lliMin >= 0) && (lliMax <= CPPIntegerMost(4, false)))
		{
			iBytes = 4;
			bSigned = false;
		}
		else
		{
			iBytes = 8;
		}
	}

	if (pcEnum)
	{
		psType = mcTypes.Enum(pcEnum, psBase ? psBase : mcTypes.Integer(iBytes, bSigned), pcSpecifier && pcSpecifier->mbScoped);
		psType->bNonNegativeEnum = bNonNegative;
		if (mpcSemanticPass)
		{
			mpcSemanticPass->EnumMade(this, psType);
		}
		return psType;
	}
	return psBase ? psBase : mcTypes.Integer(iBytes, bSigned);
}


//////////////////////////////////////////////////////////////////////////
//
// An enumerator is one more than the one before it unless it is given a
// value: the nearest one at or before it that is given a value (or the
// first, 0) plus how far before it that one is.  Only that value is worked
// out.  Working out every value before it, each of which can name the one
// before it (urlmon.h's BINDSTATUS_CONNECTING = (BINDSTATUS_FINDINGRESOURCE
// + 1) and so on 78 times), doubled the work with each enumerator.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue)
{
	CCPPSyntaxEnumerator*		pcEnumerator;
	CCPPSyntaxEnumerator*		pcOther;
	CCPPSyntaxEnumSpecifier*	pcEnum;
	size						i;
	size						uiIndex;
	size						uiValued;
	size						uiNumEnumerators;
	int64						lliValue;

	pcEnumerator = (CCPPSyntaxEnumerator*)pcSymbol->mpcDeclaration;
	if (!pcEnumerator || !pcEnumerator->mpcParent || (pcEnumerator->mpcParent->GetKind() != CPPS_EnumSpecifier))
	{
		return false;
	}
	pcEnum = (CCPPSyntaxEnumSpecifier*)pcEnumerator->mpcParent;

	uiNumEnumerators = pcEnum->macpEnumerators.NumElements();
	for (uiIndex = 0; uiIndex < uiNumEnumerators; uiIndex++)
	{
		if (pcEnum->macpEnumerators.GetPtr(uiIndex) == pcEnumerator)
		{
			break;
		}
	}
	if (uiIndex == uiNumEnumerators)
	{
		return false;
	}

	//Without one given a value at or before it, it is as far from the first as the first's 0.
	lliValue = 0;
	uiValued = 0;
	for (i = uiIndex + 1; i > 0; i--)
	{
		pcOther = (CCPPSyntaxEnumerator*)pcEnum->macpEnumerators.GetPtr(i - 1);
		if (pcOther->mpcValue)
		{
			if (!mcFolder.Constant(pcOther->mpcValue, &lliValue))
			{
				return false;
			}
			CheckEnumeratorFits(pcEnum, pcOther, lliValue);
			uiValued = i - 1;
			break;
		}
	}
	*plliValue = lliValue + (int64)(uiIndex - uiValued);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The value an enumerator is given in an enum with a fixed underlying type
// (enum E : unsigned char) is one of that type's: an error if not.
// [dcl.enum] 5
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CheckEnumeratorFits(CCPPSyntaxEnumSpecifier* pcEnum, CCPPSyntaxEnumerator* pcEnumerator, int64 lliValue)
{
	SCPP65Type*	psBase;

	psBase = pcEnum->mpcBase ? mcDeclarators.GetTypeIdType(pcEnum->mpcBase) : NULL;
	if (psBase && !ConstantFits(psBase->Unqualified(), lliValue))
	{
		mcDiagnostics.ReportOnce(pcEnumerator->mpcValue, "The constant's value isn't one of the type it is converted to");
	}
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the field Class::field names, with no object: what sizeof
// and the like may be given, as nothing is read.  NULL if the expression
// isn't that.  [expr.prim.general] 13
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::QualifiedFieldType(CCPPSyntax* pcExpression)
{
	CCPPSyntaxName*	pcName;
	CCPPClass*		pcClass;
	SCPPField*		psField;
	int				iOffset;

	if (!pcExpression || (pcExpression->GetKind() != CPPS_IdExpression))
	{
		return NULL;
	}
	pcName = ((CCPPSyntaxIdExpression*)pcExpression)->mpcName;
	if (pcName->IsQualifiedByDecltype() && !pcName->mpcSymbol)
	{
		//decltype(d)::m: the parser doesn't know d's class, so the field is found by its name.
		pcClass = QualifiersClass(pcName, pcExpression);
		psField = (pcClass && CompleteClass(pcClass, pcExpression)) ? pcClass->FindField(GetComponentName(pcName->GetLast()), &iOffset) : NULL;
		return psField ? psField->psType : NULL;
	}
	if (!pcName->mpcSymbol || (pcName->mpcSymbol->meKind != CPPSK_Field) || !pcName->IsQualifiedByClass())
	{
		return NULL;
	}
	pcClass = NameQualifierClass(pcName, pcExpression);
	psField = (pcClass && CompleteClass(pcClass, pcExpression)) ? pcClass->FindFieldSymbol(pcName->mpcSymbol, &iOffset) : NULL;
	return psField ? psField->psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the expression is a null pointer constant: nullptr, or an
// integer literal whose value is 0.  [conv.ptr] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsNullPointerConstant(CCPPSyntax* pcExpression)
{
	CCPPToken*	pcToken;
	int64		lliValue;

	pcExpression = pcExpression ? pcExpression->WithoutParentheses() : NULL;
	if (!pcExpression || (pcExpression->GetKind() != CPPS_LiteralExpression))
	{
		return false;
	}
	pcToken = mpcTokeniser->Get(((CCPPSyntaxLiteralExpression*)pcExpression)->miLiteral);
	if (pcToken->Is(CT_nullptr))
	{
		return true;
	}
	return pcToken->IsNumber() && !pcToken->IsUserLiteral() && (((CCTNumber*)pcToken)->meType != CTNT_Float) && (((CCTNumber*)pcToken)->meType != CTNT_Double) &&
		   (((CCTNumber*)pcToken)->meType != CTNT_LongDouble) && mcFolder.Constant(pcExpression, &lliValue) && (lliValue == 0);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::SkipParentheses(CCPPSyntax* pcExpression)
{
	return pcExpression ? pcExpression->WithoutParentheses() : NULL;
}


