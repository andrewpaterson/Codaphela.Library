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
#include "CTIdentifier.h"
#include "CppTokeniser.h"
#include "CPPSyntaxTree.h"
#include "CPPUnit.h"


//////////////////////////////////////////////////////////////////////////
//
// Objects as values: how a class is copied, whether an expression is an object
// (or makes one), and the operator functions that are not members.  Their code
// (temporaries, copies, objects made in place) is the IR front end's.
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
// The expression whose value, and whether it is an lvalue, an expression
// has: itself out of its brackets, and of a, b (the built-in comma) b,
// whatever is done first.  [expr.comma] 1
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::CommaResult(CCPPSyntax* pcExpression)
{
	CCPPSyntaxBinaryExpression*	pcBinary;

	for (;;)
	{
		pcExpression = SkipParentheses(pcExpression);
		pcBinary = (pcExpression->GetKind() == CPPS_BinaryExpression) ? (CCPPSyntaxBinaryExpression*)pcExpression : NULL;
		if (!pcBinary || (pcBinary->meOperator != CT_Comma) || IsOverloadedComma(pcBinary))
		{
			return pcExpression;
		}
		pcExpression = pcBinary->mpcRight;
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is an rvalue: a value with no name (3, a + b, an
// object a call or a constructor makes) or something said to be about to
// go (what a function returning T&& returns, a cast to T&&, a member of
// either).  What has a name is an lvalue, a variable declared T&& too,
// but for a local object where it is returned.  An rvalue binds to T&&
// and not to T&; an lvalue to T& and not to T&&.  The expression's
// category is the type checker's (CCPP65TypeChecker::Category).
// [basic.lval], [class.copy] 32
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsRvalue(CCPPSyntax* pcExpression)
{
	mpcStatistics->Count(CPPCC_IsRvalue);
	pcExpression = CommaResult(pcExpression);
	if ((pcExpression->GetKind() == CPPS_IdExpression) && IsReturnedLocal((CCPPSyntaxIdExpression*)pcExpression))
	{
		return true;
	}
	return mcTypeChecker.Category(pcExpression) != CPPVC_Lvalue;
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcId is return object; of one of the function's own objects (a
// local or a parameter, not a reference): it is going, so it is moved to
// the result if its class has a move.  [class.copy] 32
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsReturnedLocal(CCPPSyntaxIdExpression* pcId)
{
	CCPPSyntax*		pcParent;
	CCPPBoundName	cBound;

	for (pcParent = pcId->mpcParent; pcParent && (pcParent->GetKind() == CPPS_ParenthesisedExpression); pcParent = pcParent->mpcParent)
	{
	}
	if (!pcParent || (pcParent->GetKind() != CPPS_ReturnStatement))
	{
		return false;
	}
	mcBinder.BindName(pcId, &cBound);
	return (cBound.meWhat == CPPBN_Variable) && ((cBound.mpcVariable->meStorage == CPPVS_Local) || (cBound.mpcVariable->meStorage == CPPVS_Parameter)) && cBound.mpcVariable->mpsType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression makes an object of the class (a call, or an operator, that
// returns one): it can be made where the object it initialises is.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsMadeObject(CCPPSyntax* pcExpression, CCPPClass* pcClass)
{
	SCPP65Type*	psType;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression || !MakesObject(pcExpression))
	{
		return false;
	}
	psType = TypeOf(pcExpression);
	return psType && psType->IsClass() && (psType->pcClass == pcClass);
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression makes an object: a class prvalue (a call, a
// cast or an operator function's call that gives a class by value, a
// construction, a lambda).  CCPP65TypeChecker::Category.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::MakesObject(CCPPSyntax* pcExpression)
{
	mpcStatistics->Count(CPPCC_MakesObject);
	return mcTypeChecker.Category(pcExpression) == CPPVC_ClassPrvalue;
}


//////////////////////////////////////////////////////////////////////////
//
// The class of the object in object.member or pointer->member, or NULL if
// it isn't an object of a class.  Nothing is reported.
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::MemberObjectClass(CCPPSyntaxMemberExpression* pcMember)
{
	SCPP65Type*	psObject;

	mcDiagnostics.BeginQuiet();
	psObject = pcMember->mbArrow ? ArrowType(pcMember->mpcObject, pcMember) : ValueTypeOf(pcMember->mpcObject);
	mcDiagnostics.EndQuiet();
	if (psObject && pcMember->mbArrow)
	{
		psObject = psObject->IsPointer() ? psObject->psBase : NULL;
	}
	return (psObject && psObject->IsClass()) ? psObject->pcClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What the member of object.member or pointer->member is, in *pcBound
// (CCPPBinder::BindMember, in the object's class), with nothing reported:
// false if it is nothing, or the object isn't one.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::BindMemberOf(CCPPSyntaxMemberExpression* pcMember, CCPPBoundName* pcBound)
{
	CCPPClass*	pcClass;
	bool		bBound;

	pcClass = MemberObjectClass(pcMember);
	mcDiagnostics.BeginQuiet();
	bBound = pcClass && mcBinder.BindMember(pcClass, pcMember->mpcName, pcMember, pcBound);
	mcDiagnostics.EndQuiet();
	return bBound;
}


//////////////////////////////////////////////////////////////////////////
//
// The enumerator that object.name or pointer->name names, a member of the
// object's class (s.E is S::E, once s is worked out), or NULL.
// [expr.ref] 4
//
//////////////////////////////////////////////////////////////////////////
CCPPSymbol* CCPPUnit::MemberEnumerator(CCPPSyntaxMemberExpression* pcMember)
{
	CCPPClass*	pcClass;
	CCPPClass*	pcDeclaring;

	pcClass = MemberObjectClass(pcMember);
	return (pcClass && CompleteClass(pcClass, pcMember)) ? pcClass->FindMemberSymbol(GetComponentName(pcMember->mpcName->GetLast()), CPPSK_Enumerator, &pcDeclaring) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The bit-field an expression names, or NULL: a field of this named alone,
// or one after . or ->, that is declared with a width, and c ? x : y that
// picks one (its first).
//
//////////////////////////////////////////////////////////////////////////
SCPPField* CCPPUnit::BitFieldOf(CCPPSyntax* pcExpression)
{
	CCPPSyntaxConditionalExpression*	pcConditional;
	CCPPBoundName						cBound;
	SCPPField*							psField;

	pcExpression = SkipParentheses(pcExpression);
	cBound.meWhat = CPPBN_Nothing;
	if (pcExpression->GetKind() == CPPS_ConditionalExpression)
	{
		pcConditional = (CCPPSyntaxConditionalExpression*)pcExpression;
		psField = BitFieldOf(pcConditional->mpcTrue);
		return psField ? psField : BitFieldOf(pcConditional->mpcFalse);
	}
	if (pcExpression->GetKind() == CPPS_IdExpression)
	{
		mcDiagnostics.BeginQuiet();
		mcBinder.BindName((CCPPSyntaxIdExpression*)pcExpression, &cBound);
		mcDiagnostics.EndQuiet();
	}
	else if ((pcExpression->GetKind() == CPPS_MemberExpression) && !BindMemberOf((CCPPSyntaxMemberExpression*)pcExpression, &cBound))
	{
		cBound.meWhat = CPPBN_Nothing;
	}
	return ((cBound.meWhat == CPPBN_Field) && cBound.mpsField->IsBitField()) ? cBound.mpsField : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type the value of pcExpression, a psType, has where integral
// promotion is applied to it (an operator's operand, an argument that has
// no parameter, a switch's condition): its own, but for a bit-field, which
// is promoted by its width and not by its type.  An unsigned field of
// three bits is an int there, so that field - 4 is less than 0.
// [conv.prom] 3
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::OperandType(SCPP65Type* psType, CCPPSyntax* pcExpression)
{
	SCPPField*	psField;

	psField = psType->IsInteger() ? BitFieldOf(pcExpression) : NULL;
	return psField ? mcTypes.PromoteBitField(psType, psField->iBitWidth) : psType;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of what a call is made through when it isn't a function by its
// name: a pointer to a function, which a function (what a reference to one
// refers to) is converted to.  NULL if the callee is neither.  [conv.func]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::CalleePointerType(CCPPSyntax* pcCallee)
{
	SCPP65Type*	psType;

	psType = TypeOf(pcCallee);
	if (psType && psType->IsFunction())
	{
		psType = mcTypes.Pointer(psType);
	}
	return (psType && psType->IsPointer() && psType->psBase->IsFunction()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type an argument with no parameter (one of a function's ...) is
// passed as, after the default argument promotions: a bool or an integer
// smaller than an int is promoted, a float is a double, an array or a
// function is a pointer to it and nullptr is a void*.  NULL for what isn't
// passed so (an object).  [expr.call] 7
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::VariadicArgumentType(SCPP65Type* psType)
{
	if (psType->IsFloat())
	{
		return &mcTypes.msDouble;
	}
	if (psType->IsInteger())
	{
		return mcTypes.Promote(psType);
	}
	if (psType->IsArray())
	{
		return mcTypes.Pointer(psType->psBase);
	}
	if (psType->IsFunction())
	{
		return mcTypes.Pointer(psType);
	}
	if (psType->IsNullPointer())
	{
		return mcTypes.Pointer(&mcTypes.msVoid);
	}
	return (psType->IsPointer() || psType->IsMemberPointer()) ? psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What the types two pointers point to, or two pointers to members point
// to as members, have in common: one type with the const and volatile of
// both, where they are the same type but for those.  At the first level
// (bFirstLevel: what the pointers themselves point to) void with an object
// type is void, and a class with one it is derived from is the base.
// Pointers that differ further in point to what those have in common, and
// are const as const was added there.  NULL if they have nothing in
// common.  [expr] 13, [conv.qual] 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::CompositePointee(SCPP65Type* psLeft, SCPP65Type* psRight, bool bFirstLevel)
{
	SCPP65Type*	psCommon;
	SCPP65Type*	psInner;
	CCPPClass*	pcBase;
	bool		bConst;

	bConst = psLeft->bConst || psRight->bConst;
	pcBase = bFirstLevel ? ConditionalClass(psLeft, psRight) : NULL;
	psCommon = NULL;
	if (psLeft->Unqualified()->Same(psRight->Unqualified()))
	{
		psCommon = psLeft;
	}
	else if (bFirstLevel && (psLeft->IsVoid() || psRight->IsVoid()) && !psLeft->IsFunction() && !psRight->IsFunction())
	{
		psCommon = &mcTypes.msVoid;
	}
	else if (pcBase)
	{
		psCommon = pcBase->mpsType;
	}
	else if (psLeft->IsPointer() && psRight->IsPointer())
	{
		psInner = CompositePointee(psLeft->psBase, psRight->psBase, false);
		psCommon = psInner ? mcTypes.Pointer(psInner) : NULL;
		bConst = true;
	}
	return psCommon ? mcTypes.Qualified(psCommon->Unqualified(), bConst, psLeft->bVolatile || psRight->bVolatile) : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The composite pointer type of two operands: the type both are converted
// to before two pointers, or two pointers to members, are compared, and
// the type of c ? x : y of them.  A null pointer constant (a std::nullptr_t,
// or an integer, which its user has seen is one) with a pointer is the
// pointer's type and with a std::nullptr_t that; two pointers are a
// pointer to what they point to in common (CompositePointee); two
// pointers to members are the derived class's, to what the members are in
// common.  NULL if there is no such type.  [expr] 13
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::CompositePointerType(SCPP65Type* psLeft, SCPP65Type* psRight)
{
	SCPP65Type*	psPointee;
	CCPPClass*	pcDerived;
	bool		bLeftNull;
	bool		bRightNull;

	bLeftNull = psLeft->IsNullPointer() || psLeft->IsInteger();
	bRightNull = psRight->IsNullPointer() || psRight->IsInteger();
	if (bRightNull && (psLeft->IsPointer() || psLeft->IsMemberPointer() || psLeft->IsNullPointer()))
	{
		return psLeft->Unqualified();
	}
	if (bLeftNull && (psRight->IsPointer() || psRight->IsMemberPointer() || psRight->IsNullPointer()))
	{
		return psRight->Unqualified();
	}
	if (psLeft->IsPointer() && psRight->IsPointer())
	{
		psPointee = CompositePointee(psLeft->psBase, psRight->psBase, true);
		return psPointee ? mcTypes.Pointer(psPointee) : NULL;
	}
	if (psLeft->IsMemberPointer() && psRight->IsMemberPointer())
	{
		//A pointer to a base's member is one to the derived class's.  [conv.mem] 2
		pcDerived = IsDerived(psLeft->pcClass, psRight->pcClass, NULL) ? psLeft->pcClass : (IsDerived(psRight->pcClass, psLeft->pcClass, NULL) ? psRight->pcClass : NULL);
		psPointee = CompositePointee(psLeft->psBase, psRight->psBase, false);
		return (pcDerived && psPointee) ? mcTypes.MemberPointer(pcDerived, psPointee) : NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of c ? x : y for x and y that aren't objects, NULL if they have
// none in common: void if either is; their own type where it is one type,
// which isn't promoted (two chars are a char, two bools a bool); the usual
// arithmetic conversions' for numbers and enums; otherwise their composite
// pointer type.  [expr.cond] 2, 6
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ConditionalType(SCPP65Type* psTrue, SCPP65Type* psFalse)
{
	if (psTrue->IsVoid() || psFalse->IsVoid())
	{
		return &mcTypes.msVoid;
	}
	if (psTrue->Unqualified()->Same(psFalse->Unqualified()))
	{
		return psTrue->Unqualified();
	}
	if (psTrue->IsFloat() || psFalse->IsFloat())
	{
		return ((psTrue->IsFloat() || psTrue->IsInteger()) && (psFalse->IsFloat() || psFalse->IsInteger())) ? mcTypes.FloatArithmetic(psTrue, psFalse) : NULL;
	}
	if (psTrue->IsInteger() && psFalse->IsInteger())
	{
		return mcTypes.Arithmetic(psTrue, psFalse);
	}
	return CompositePointerType(psTrue, psFalse);
}


//////////////////////////////////////////////////////////////////////////
//
// The class of c ? x : y with objects: x's and y's if they are the same,
// otherwise the one the other is derived from (the derived one is converted
// to its base).  NULL if they aren't both objects, or neither class is the
// other's base.  [expr.cond] 4
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::ConditionalClass(SCPP65Type* psTrue, SCPP65Type* psFalse)
{
	psTrue = psTrue->WithoutReference();
	psFalse = psFalse->WithoutReference();
	if (!psTrue->IsClass() || !psFalse->IsClass())
	{
		return NULL;
	}
	if (IsDerived(psTrue->pcClass, psFalse->pcClass, NULL))
	{
		return psFalse->pcClass;
	}
	return IsDerived(psFalse->pcClass, psTrue->pcClass, NULL) ? psTrue->pcClass : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a type is an aggregate: an array, or a class with no constructor
// of its own, no base and no virtual function, whose members a list in
// braces gives values to.  [dcl.init.aggr] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsAggregate(SCPP65Type* psType)
{
	CCPPClass*	pcClass;
	bool		bComplete;

	if (psType->IsArray())
	{
		return true;
	}
	if (!psType->IsClass())
	{
		return false;
	}
	pcClass = psType->pcClass;
	mcDiagnostics.BeginQuiet();
	bComplete = CompleteClass(pcClass, NULL);
	mcDiagnostics.EndQuiet();
	return bComplete && !pcClass->mbConstructors && !pcClass->mpsLambda && (pcClass->macBases.NumElements() == 0) && !pcClass->IsPolymorphic();
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the member of an aggregate that its list's value number
// uiIndex is for: an array's element, a class's field in the order they are
// declared (a union's first only), bit-fields with no name left out.  NULL
// past the last.  An array of unknown bound has an element for every value.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::AggregateMember(SCPP65Type* psType, size uiIndex)
{
	SCPPField*	psField;

	if (psType->IsArray())
	{
		return ((psType->iCount == 0) || (uiIndex < (size)psType->iCount)) ? psType->psBase : NULL;
	}
	psField = psType->pcClass->AggregateField(uiIndex);
	return psField ? psField->psType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the lvalue c ? x : y is: x and y are lvalues of one type (it
// has the const and volatile of both), or objects of a class and of one
// derived from it, the base no less const (the derived object is its base
// there).  NULL if c ? x : y is a value.  [expr.cond] 3, 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ConditionalLvalueType(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*	psTrue;
	SCPP65Type*	psFalse;
	SCPP65Type*	psBase;
	CCPPClass*	pcClass;

	psTrue = PlaceOf(pcConditional->mpcTrue);
	psFalse = PlaceOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse || IsRvalue(pcConditional->mpcTrue) || IsRvalue(pcConditional->mpcFalse))
	{
		return NULL;
	}
	if (psTrue->Unqualified()->Same(psFalse->Unqualified()))
	{
		return mcTypes.Qualified(psTrue->Unqualified(), psTrue->bConst || psFalse->bConst, psTrue->bVolatile || psFalse->bVolatile);
	}
	pcClass = ConditionalClass(psTrue, psFalse);
	if (!pcClass)
	{
		return NULL;
	}
	psBase = (psTrue->pcClass == pcClass) ? psTrue : psFalse;
	return psBase->HasQualifiersOf((psBase == psTrue) ? psFalse : psTrue) ? psBase : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of c ? x : y when x and y are both about to go and of one type,
// static_cast<T&&>(a) and a call that returns T&&: a T&& too.  NULL if they
// aren't.  [expr.cond] 4
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ConditionalXvalueType(CCPPSyntaxConditionalExpression* pcConditional)
{
	SCPP65Type*	psTrue;
	SCPP65Type*	psFalse;

	psTrue = DeclaredTypeOf(pcConditional->mpcTrue);
	psFalse = DeclaredTypeOf(pcConditional->mpcFalse);
	return (psTrue && psFalse && psTrue->IsRvalueReference() && psFalse->IsRvalueReference() && psTrue->psBase->Same(psFalse->psBase)) ? psTrue : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The type of c ? x : y when x or y is an object and the result is a value:
// the class of both; the base, when one's class is derived from the
// other's; or else the type of the one operand that the other converts to
// (an int for an object with a conversion function to it, the class for
// what its constructor takes).  NULL if neither converts, or each does
// (*pbAmbiguous).  [expr.cond] 3
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ConditionalObjectType(CCPPSyntaxConditionalExpression* pcConditional, bool* pbAmbiguous)
{
	SCPP65Type*	psTrue;
	SCPP65Type*	psFalse;
	CCPPClass*	pcClass;
	bool		bToTrue;
	bool		bToFalse;

	*pbAmbiguous = false;
	psTrue = ValueTypeOf(pcConditional->mpcTrue);
	psFalse = ValueTypeOf(pcConditional->mpcFalse);
	if (!psTrue || !psFalse || psTrue->IsVoid() || psFalse->IsVoid())
	{
		return NULL;
	}
	pcClass = ConditionalClass(psTrue, psFalse);
	if (pcClass)
	{
		return (psTrue->pcClass == pcClass) ? psTrue : psFalse;
	}
	bToTrue = mcTypeChecker.CanInitialise(psTrue, pcConditional->mpcFalse);
	bToFalse = mcTypeChecker.CanInitialise(psFalse, pcConditional->mpcTrue);
	*pbAmbiguous = bToTrue && bToFalse;
	if (bToTrue == bToFalse)
	{
		return NULL;
	}
	return (bToTrue ? psTrue : psFalse)->Unqualified();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an identifier (lliName) is szName.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsNamed(int64 lliName, const char* szName)
{
	char*	szIdentifier;
	int		iLength;

	if (lliName < 0)
	{
		return false;
	}
	szIdentifier = mpcTokeniser->GetIdentifier(lliName, &iLength);
	return ((size)iLength == strlen(szName)) && (memcmp(szIdentifier, szName, iLength) == 0);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a name declared in pcScope is std::szName: the library's, which
// the compiler knows (std::initializer_list, std::type_info).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsStdName(int64 lliName, CCPPScope* pcScope, const char* szName)
{
	return pcScope && pcScope->mpcOwner && IsNamed(lliName, szName) && IsNamed(pcScope->mpcOwner->mlliName, "std") && (!pcScope->mpcParent || !pcScope->mpcParent->mpcOwner);
}


//////////////////////////////////////////////////////////////////////////
//
// The class std::type_info, if the program has declared it (<typeinfo>), or
// NULL.  [type.info]
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::TypeInfoClass(void)
{
	CCPPClass*	pcClass;
	size		i;

	for (i = 0; !mpcTypeInfo && (i < mapcClasses.NumElements()); i++)
	{
		pcClass = mapcClasses.GetPtr(i);
		if (!pcClass->mpcTemplate && pcClass->mpcSymbol && IsStdName(pcClass->mlliName, pcClass->mpcSymbol->mpcScope, "type_info"))
		{
			mpcTypeInfo = pcClass;
		}
	}
	return mpcTypeInfo;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a function is a member of std::type_info that the program declares
// and nothing defines (<typeinfo>'s), which the IR front end works out where
// it is called (CCPP65IRFrontEnd::TypeInfoCall).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsTypeInfoMember(CCPPFunction* pcFunction)
{
	return pcFunction->mpcClass && (pcFunction->mpcClass == TypeInfoClass()) && !pcFunction->mpcDefinition && (pcFunction->meKind == CPPFK_Function);
}


//////////////////////////////////////////////////////////////////////////
//
// The type typeid gives the std::type_info of: a type's, or an expression's
// (its type without a reference), without const and volatile.  *pbDynamic
// when it is the object's own (an lvalue of a polymorphic class), found
// from its vtable.  [expr.typeid]
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::TypeidType(CCPPSyntaxTypeidExpression* pcTypeid, bool* pbDynamic)
{
	SCPP65Type*	psType;

	*pbDynamic = false;
	psType = OperandType(pcTypeid->mpcType, pcTypeid->mpcOperand);
	psType = psType ? psType->WithoutReference() : NULL;
	if (!pcTypeid->mpcType)
	{
		*pbDynamic = psType && psType->IsClass() && CompleteClass(psType->pcClass, pcTypeid) && psType->pcClass->IsPolymorphic() && IsObjectLvalue(pcTypeid->mpcOperand);
	}
	return psType ? psType->Unqualified() : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// The class template std::initializer_list, if the program has declared it
// (<initializer_list>), or NULL.  [support.initlist]
//
//////////////////////////////////////////////////////////////////////////
CCPPTemplate* CCPPUnit::InitializerListTemplate(void)
{
	CCPPTemplate*	pcTemplate;
	size			i;

	for (i = 0; !mpcInitializerList && (i < mapcTemplates.NumElements()); i++)
	{
		pcTemplate = mapcTemplates.GetPtr(i);
		if (pcTemplate->mbMember || (pcTemplate->mpcPrimary != pcTemplate) || !pcTemplate->mpcSymbol || (pcTemplate->mpcSymbol->mlliName < 0))
		{
			continue;
		}
		if (IsStdName(pcTemplate->mpcSymbol->mlliName, pcTemplate->mpcNamespace, "initializer_list"))
		{
			mpcInitializerList = pcTemplate;
		}
	}
	return mpcInitializerList;
}


//////////////////////////////////////////////////////////////////////////
//
// E if psType (or what it refers to) is std::initializer_list<E>, or NULL.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::InitializerListElement(SCPP65Type* psType)
{
	CCPPClass*	pcClass;

	psType = psType ? psType->WithoutReference() : NULL;
	pcClass = (psType && psType->IsClass()) ? psType->pcClass : NULL;
	if (!pcClass || !pcClass->mpcTemplate || !InitializerListTemplate() || (pcClass->mpcTemplate->mpcPrimary != mpcInitializerList) || !pcClass->mpcBindings || (pcClass->mpcBindings->macBindings.NumElements() != 1))
	{
		return NULL;
	}
	return pcClass->mpcBindings->macBindings.Get(0)->psType;
}


//////////////////////////////////////////////////////////////////////////
//
// std::initializer_list<psElement>, or NULL after an error if the program
// hasn't declared it.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::InitializerListOf(SCPP65Type* psElement, CCPPSyntax* pcNode)
{
	CCPPTemplateArguments*	pcBindings;
	CCPPClass*				pcClass;
	CArrayCPP65Types		apsDeduced;
	CChars					szArguments;

	if (!InitializerListTemplate())
	{
		mcDiagnostics.Error(pcNode, "A list in braces is a std::initializer_list, which needs <initializer_list>");
		return NULL;
	}
	apsDeduced.Init();
	apsDeduced.Add(psElement);
	szArguments.Init();
	pcBindings = mcInstantiator.BindArguments(mpcInitializerList->mpcDeclaration, NULL, &apsDeduced, &szArguments, pcNode);
	pcClass = pcBindings ? mcInstantiator.InstanceOfClass(mpcInitializerList, pcBindings, &szArguments, pcNode) : NULL;
	szArguments.Kill();
	apsDeduced.Kill();
	return pcClass ? pcClass->mpsType : NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// What auto is for a list in braces (auto l = {1, 2}, and a range-for's
// range): std::initializer_list of its elements' type, which they must all
// have (an array's a pointer, without its const).  [dcl.spec.auto] 7
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::BracedListType(CCPPSyntax* pcList, CCPPSyntax* pcNode)
{
	CArraySyntax*	papcElements;
	SCPP65Type*		psElement;
	SCPP65Type*		psType;
	size			i;

	papcElements = ListElements(pcList);
	if (papcElements->NumElements() == 0)
	{
		mcDiagnostics.Error(pcNode, "auto can't be worked out from {}");
		return NULL;
	}
	psElement = NULL;
	for (i = 0; i < papcElements->NumElements(); i++)
	{
		psType = ValueTypeOf(papcElements->GetPtr(i));
		if (!psType)
		{
			ReportTypeOf(papcElements->GetPtr(i));
			return NULL;
		}
		psType = (psType->IsArray() || psType->IsFunction()) ? mcTypes.Pointer(psType->ElementType()) : psType;
		psType = psType->Unqualified();
		if (psElement && !psElement->Same(psType))
		{
			mcDiagnostics.Error(papcElements->GetPtr(i), "The values in braces aren't all of the same type, which auto needs");
			return NULL;
		}
		psElement = psType;
	}
	return InitializerListOf(psElement, pcNode);
}


//////////////////////////////////////////////////////////////////////////
//
// Whether a std::initializer_list instance is laid out as the compiler
// fills it: a pointer to its elements, then their count (an integer) or
// the end (a pointer, *pbEnd).  <initializer_list>'s are.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::InitializerListLayout(CCPPClass* pcClass, bool* pbEnd)
{
	SCPPField*	psFirst;
	SCPPField*	psSecond;

	if (!CompleteClass(pcClass, NULL) || (pcClass->macFields.NumElements() != 2) || (pcClass->macBases.NumElements() != 0) || pcClass->IsPolymorphic())
	{
		return false;
	}
	psFirst = pcClass->macFields.Get(0);
	psSecond = pcClass->macFields.Get(1);
	*pbEnd = psSecond->psType->IsPointer();
	return psFirst->psType->IsPointer() && !psFirst->IsBitField() && !psSecond->IsBitField() && (*pbEnd || psSecond->psType->IsInteger());
}


//////////////////////////////////////////////////////////////////////////
//
// An expression of a class type that is an object where it is (named,
// pointed at, indexed, or a reference to one), not one to be made: an
// lvalue or an xvalue (CCPP65TypeChecker::Category).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsObjectLvalue(CCPPSyntax* pcExpression)
{
	ECPPValueCategory	eCategory;

	mpcStatistics->Count(CPPCC_IsObjectLvalue);
	eCategory = mcTypeChecker.Category(pcExpression);
	return (eCategory == CPPVC_Lvalue) || (eCategory == CPPVC_Xvalue);
}


//////////////////////////////////////////////////////////////////////////
//
// The type of the place a reference to the expression would refer to, or
// NULL if it is a value with no address (a literal, a sum, a call's int).
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::PlaceOf(CCPPSyntax* pcExpression)
{
	mpcStatistics->Count(CPPCC_PlaceOf);
	return mcTypeChecker.PlaceFact(pcExpression);
}


//////////////////////////////////////////////////////////////////////////
//
// What a reference is initialised from: a list in braces with one element
// the reference binds to as it is (its type the reference's, or a class
// derived from it) is that element, int& r{x}; any other list makes a
// temporary the reference is bound to (BindsToTemporary).
// [dcl.init.list] 3
//
//////////////////////////////////////////////////////////////////////////
CCPPSyntax* CCPPUnit::BoundInitialiser(SCPP65Type* psReference, CCPPSyntax* pcInitialiser)
{
	CArraySyntax*	papcElements;
	CCPPSyntax*		pcElement;
	SCPP65Type*		psElement;
	SCPP65Type*		psReferred;

	if (!pcInitialiser || (pcInitialiser->GetKind() != CPPS_InitializerList))
	{
		return pcInitialiser;
	}
	papcElements = ListElements(pcInitialiser);
	pcElement = (papcElements->NumElements() == 1) ? papcElements->GetPtr(0) : NULL;
	if (!pcElement || (pcElement->GetKind() == CPPS_InitializerList))
	{
		return pcInitialiser;
	}
	mcDiagnostics.BeginQuiet();
	psElement = PlaceOf(pcElement);
	psElement = psElement ? psElement : ValueTypeOf(pcElement);
	mcDiagnostics.EndQuiet();
	psElement = psElement ? psElement->WithoutReference()->Unqualified() : NULL;
	psReferred = psReference->psBase->Unqualified();
	if (psElement && (psElement->Equals(psReferred) || (psElement->IsClass() && psReferred->IsClass() && IsDerived(psElement->pcClass, psReferred->pcClass, NULL))))
	{
		return pcElement;
	}
	return pcInitialiser;
}


//////////////////////////////////////////////////////////////////////////
//
// True if a reference gets a temporary holding the value, not the value's
// address: a const reference or T&&, and a value that has no address or
// is of another type (const long& of a char, const char* const& of an
// array, const Class& of what a Class is made from, a list in braces).  An
// object of the class, or of one derived from it, is referred to itself.
// [dcl.init.ref]
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::BindsToTemporary(SCPP65Type* psReference, CCPPSyntax* pcArgument)
{
	SCPP65Type*			psPlace;
	SCPP65Type*			psValue;
	CCPPBoundConversion	cConversion;

	if (!(psReference->psBase->bConst || psReference->bRvalue))
	{
		return false;
	}
	if (pcArgument->GetKind() == CPPS_InitializerList)
	{
		//const S& s = {1, 2}: what the list makes.  [dcl.init.list] 3
		return true;
	}
	if (psReference->psBase->IsClass())
	{
		psValue = ValueTypeOf(pcArgument);
		return psValue && !(psValue->IsClass() && ((psValue->pcClass == psReference->psBase->pcClass) || IsDerived(psValue->pcClass, psReference->psBase->pcClass, NULL)));
	}
	psValue = ValueTypeOf(pcArgument);
	if (psValue && psValue->IsClass() && !psReference->bRvalue)
	{
		//const T& binds to what the object's operator T& returns, itself; a temporary only holds what another
		//conversion function's result converts to.  [dcl.init.ref] 5.1.2, [over.match.ref]
		mcBinder.BindObjectConversion(psValue->pcClass, psReference, false, NULL, &cConversion);
		return !cConversion.mpcFunction || !cConversion.mpcFunction->mpsReturn->IsLvalueReference() ||
			   !psReference->psBase->Unqualified()->Equals(cConversion.mpcFunction->mpsReturn->psBase->Unqualified()) || !psReference->psBase->HasQualifiersOf(cConversion.mpcFunction->mpsReturn->psBase);
	}
	if (IsUncapturedConstant(pcArgument))
	{
		//A constant a lambda doesn't capture has no object in it to refer to: binding a reference to it odr-uses it.
		//[basic.def.odr] 3
		mcDiagnostics.ReportOnce(pcArgument, "The lambda doesn't capture the variable");
		return true;
	}
	psPlace = PlaceOf(pcArgument);
	return !psPlace || !psReference->psBase->Equals(psPlace->WithoutReference());
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression names a constant of the function a lambda being
// written is in, which the lambda doesn't capture (CPPBN_EnclosingConstant):
// only its value can be used.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsUncapturedConstant(CCPPSyntax* pcExpression)
{
	CCPPBoundName	cBound;

	pcExpression = SkipParentheses(pcExpression);
	if (!pcExpression || (pcExpression->GetKind() != CPPS_IdExpression))
	{
		return false;
	}
	mcBinder.BindName((CCPPSyntaxIdExpression*)pcExpression, &cBound);
	return cBound.meWhat == CPPBN_EnclosingConstant;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether left, right is a call of an operator, because an operand is an
// object and one matches.  Other operators with an object need one; a comma
// without one is the built in comma.  [over.match.oper] 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsOverloadedComma(CCPPSyntaxBinaryExpression* pcBinary)
{
	CCPPBoundCall	cBound;
	bool			bOverloaded;

	if (!IsObject(pcBinary->mpcLeft) && !IsObject(pcBinary->mpcRight))
	{
		return false;
	}
	mcDiagnostics.BeginSilent();
	bOverloaded = mcBinder.BindOperator(CT_Comma, pcBinary->mpcLeft, pcBinary->mpcRight, pcBinary, &cBound, false);
	mcDiagnostics.EndSilent();
	return bOverloaded;
}


//////////////////////////////////////////////////////////////////////////
//
// True if the expression is an object (or a reference to one).
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsObject(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	mpcStatistics->Count(CPPCC_IsObject);
	psType = ValueTypeOf(pcExpression);
	return psType && psType->IsClass();
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an operator's operand may make it an operator function's call: an
// object, or an enumerator or enum value where an operator function of the
// operator's name that is no class's is declared (else it is only ever the
// built-in operator, and isn't looked for).  [over.match.oper] 1, 3
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsOperatorOperand(CCPPSyntax* pcExpression, ECppTokens eOperator)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	if (!psType)
	{
		return false;
	}
	if (psType->IsClass())
	{
		return true;
	}
	return psType->pcEnum && mcBinder.HasFreeOperator(GetOperatorName(eOperator, false));
}


//////////////////////////////////////////////////////////////////////////
//
// The class of an object expression (see IsObject).
//
//////////////////////////////////////////////////////////////////////////
CCPPClass* CCPPUnit::ObjectClass(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType->pcClass;
}


