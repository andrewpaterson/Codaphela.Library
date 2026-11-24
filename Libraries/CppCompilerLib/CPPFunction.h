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
#ifndef __C_P_P_FUNCTION_H__
#define __C_P_P_FUNCTION_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPPExceptionSpec.h"
#include "CPP65Type.h"


//The names of functions that aren't identifiers, below -1 (which is no name): an operator is named by its token
//(operator() is the first token's), new[] and delete[] apart from new and delete, and every conversion function
//(operator T) has one name; they differ by their result.  A class's deleting destructor, which no program names, has
//one too (CCPPClass::mpcDeleting).
#define CPP_OPERATOR_NAME(eToken)		(-2 - (int64)(eToken))
#define CPP_ARRAY_OPERATOR_NAME(eToken)	(CPP_OPERATOR_NAME(eToken) - (int64)NUM_CPP_TOKENS)
#define CPP_CONVERSION_NAME				(CPP_OPERATOR_NAME(0) - 2 * (int64)NUM_CPP_TOKENS)
#define CPP_DELETING_NAME				(CPP_CONVERSION_NAME - 1)
#define CPP_LITERAL_NAME(iSuffix)		(CPP_DELETING_NAME - 1 - (int64)(iSuffix))	//operator "" _suffix, by the suffix's identifier.
#define CPP_IS_LITERAL_NAME(lliName)	((lliName) < CPP_DELETING_NAME)
#define CPP_LITERAL_SUFFIX(lliName)		(CPP_DELETING_NAME - 1 - (lliName))


//operator new and new[], the allocation functions; operator delete and delete[], the deallocation functions.
//[basic.stc.dynamic] 2
inline bool CPPIsNewName(int64 lliName) { return (lliName == CPP_OPERATOR_NAME(CT_new)) || (lliName == CPP_ARRAY_OPERATOR_NAME(CT_new)); }
inline bool CPPIsDeleteName(int64 lliName) { return (lliName == CPP_OPERATOR_NAME(CT_delete)) || (lliName == CPP_ARRAY_OPERATOR_NAME(CT_delete)); }
inline bool CPPIsAllocationName(int64 lliName) { return CPPIsNewName(lliName) || CPPIsDeleteName(lliName); }


//The object a member function is called on, which chooses between a const or volatile function and one that isn't,
//and between one declared & and one declared &&: CPPCO_None when there is none to choose by, else CPPCO_Object with
//what the object is.
enum ECPPCalledObject
{
	CPPCO_None = 0,
	CPPCO_Object = 1,
	CPPCO_Const = 2,
	CPPCO_Volatile = 4,
	CPPCO_Rvalue = 8,	//An object with no name, or one about to go (CCPPUnit::IsRvalue).
};


//The object called on, from whether it is const, volatile and an rvalue.
inline ECPPCalledObject CPPCalledObject(bool bConst, bool bVolatile = false, bool bRvalue = false)
{
	return (ECPPCalledObject)(CPPCO_Object | (bConst ? CPPCO_Const : 0) | (bVolatile ? CPPCO_Volatile : 0) | (bRvalue ? CPPCO_Rvalue : 0));
}


inline bool CPPCalledConst(ECPPCalledObject eObject) { return (eObject & CPPCO_Const) != 0; }
inline bool CPPCalledVolatile(ECPPCalledObject eObject) { return (eObject & CPPCO_Volatile) != 0; }
inline bool CPPCalledRvalue(ECPPCalledObject eObject) { return (eObject & CPPCO_Rvalue) != 0; }


//The types a place converts an object to by its conversion function, when it wants one of them and the class has
//one such function.  [conv] 5, [over.built]
enum ECPPContextualTypes
{
	CPPXT_Integer = 1,			//An integer or an enum that isn't scoped: a switch's condition, a new array's bound, ~object.
	CPPXT_Float = 2,
	CPPXT_ObjectPointer = 4,	//delete's operand.
	CPPXT_FunctionPointer = 8,
	CPPXT_Arithmetic = CPPXT_Integer | CPPXT_Float,												//-object
	CPPXT_ArithmeticOrPointer = CPPXT_Arithmetic | CPPXT_ObjectPointer | CPPXT_FunctionPointer,	//+object
};


//What the built-in +, - or ~ takes.  [expr.unary.op] 7, 8, 10
inline ECPPContextualTypes CPPUnaryOperandTypes(ECppTokens eOperator)
{
	return (eOperator == CT_Add) ? CPPXT_ArithmeticOrPointer : ((eOperator == CT_Subtract) ? CPPXT_Arithmetic : CPPXT_Integer);
}


//What a built-in binary operator takes: + and - and the comparisons numbers and pointers, * and / numbers, the rest
//integers.  [expr.mul] to [expr.or]
inline ECPPContextualTypes CPPBinaryOperandTypes(ECppTokens eOperator)
{
	switch (eOperator)
	{
	case CT_Multiply:
	case CT_Divide:
		return CPPXT_Arithmetic;
	case CT_Modulus:
	case CT_BitwiseAnd:
	case CT_BitwiseOr:
	case CT_BitwiseXor:
	case CT_LeftShift:
	case CT_RightShift:
		return CPPXT_Integer;
	default:
		return CPPXT_ArithmeticOrPointer;
	}
}


//The object without its const and volatile: what a function that takes the arguments but not the object would take.
inline ECPPCalledObject CPPCalledUnqualified(ECPPCalledObject eObject)
{
	return (ECPPCalledObject)(eObject & ~(CPPCO_Const | CPPCO_Volatile));
}


//void f() & and void f() &&: the kind of object a member function takes.  [dcl.fct] 6
enum ECPPRefQualifier
{
	CPPRQ_None,
	CPPRQ_Lvalue,
	CPPRQ_Rvalue,
};


enum ECPPFunctionKind
{
	CPPFK_Function,
	CPPFK_Constructor,
	CPPFK_Destructor,
};


//What is done to an object of a class with a function the class declares or, when it declares none, part by part.
enum ECPPSpecialMember
{
	CPPSM_DefaultConstruct,
	CPPSM_Copy,
	CPPSM_Move,
	CPPSM_Assign,
	CPPSM_MoveAssign,
	CPPSM_Destroy,
};


class CCPPClass;
class CCPPTemplate;
class CCPPTemplateArguments;
struct SCPPSignature;


//A function's signature, from its definition or a declaration, and its definition.  Inline, in class and
//template functions are generated when they are first called; others when their definition is reached.
class CCPPFunction
{
public:
	int64							mlliName;		//An identifier, or an operator (see CCPPUnit::GetOperatorName).
	SCPP65Type*						mpsReturn;
	CArrayCPP65Types				mapsParameters;
	CArraySyntax					mapcDefaults;	//Each parameter's default argument, or NULL.
	bool							mbVariadic;
	bool							mbDefined;
	CCPPClass*						mpcClass;		//NULL for a function that is not a member.
	CCPPScope*						mpcNamespace;	//The namespace of a function that is not a member, NULL for the global one.
	ECPPFunctionKind				meKind;
	bool							mbStatic;
	bool							mbVirtual;
	bool							mbPure;
	int								miSlot;			//A virtual function's place in the vtable.
	bool							mbCLinkage;		//Declared in extern "C": its label is only its name.
	bool							mbExplicit;		//An explicit constructor or conversion function: not used for implicit conversions.
	int32							miDeprecated;	//The token of the [[deprecated]] a declaration of it is marked with, or -1: a call of it is warned of.  [dcl.attr.deprecated]
	bool							mbConstThis;	//A const member function: an overload of the same function without const.
	bool							mbVolatileThis;	//A volatile member function: another overload again.
	bool							mbConstexpr;	//Declared constexpr: its value may be worked out when the program is compiled.
	CCPPExceptionSpec				mcExceptions;	//What it may throw.  Ask CCPPUnit::ExceptionsOf: one that wasn't written is worked out when it is asked for.
	bool							mbExceptionsWritten;	//mcExceptions is what a declaration says (or what an operator delete declared without one has).
	bool							mbExceptionsKnown;		//mcExceptions has been worked out (a destructor's, or a function's the compiler writes).
	bool							mbOverride;		//Declared override: it must override a base class's virtual function.
	bool							mbFinal;		//Declared final: no derived class's function may override it.
	CCPPFunction*					mpcAddressOf;	//What a closure's conversion to a pointer to a function gives the address of (its invoker), or NULL.
	ECPPRefQualifier				meRefQualifier;	//Declared & or &&: an overload of the same function declared the other.
	bool							mbImplicit;		//A constructor or destructor the compiler wrote.
	bool							mbDeleting;		//Its class's deleting destructor (CCPPClass::mpcDeleting): the compiler writes its body.
	bool							mbDeleted;		//= delete: it is an overload like any other, and an error if it is the one chosen.
	CCPPFunction*					mpcInherited;	//The base's constructor an inherited constructor calls (using Base::Base).
	CCPPFunction*					mpcCovariantOf;	//A covariant thunk (CCPPVtable::EntryFor): the overrider it calls, whose result it moves...
	int								miCovariantAdjust;	//...by this, to the overridden function's result.
	CCPPSyntaxFunctionDefinition*	mpcDefinition;
	CCPPSyntaxDeclarator*			mpcDeclarator;	//The first declaration's declarator (a template's, for an instance).
	CCPPTemplateArguments*			mpcBindings;	//The template arguments the definition is generated with.
	bool							mbInline;		//Generated when used.
	bool							mbQueued;
	bool							mbGenerated;
	bool							mbCalled;
	CCPPTemplate*					mpcTemplate;	//The function template this is an instance of.
	CChars							mszArguments;	//The instance's template arguments, mangled for its label.
	bool							mbExternInstance;	//A function template's instance extern template names: another unit's.
	CCPPFunction*					mpcNextOfName;	//The next function added with the same mlliName (see CCPPUnit::FirstFunctionOfName).

	void	Init(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn);
	void	Kill(void);

	bool	IsConversion(void) const;	//operator T.
	bool	IsOperator(void) const;		//An operator or a conversion: its name is no identifier.

	size	RequiredParameters(void);
	bool	TakesArgumentCount(size uiArguments);
	bool	SameParameters(CCPPFunction* pcOther);
	bool	TakesParameters(CArrayCPP65Types* papsParameters);
	bool	IsDeclaredBy(SCPPSignature* psSignature);
	void	Adopt(SCPPSignature* psSignature);
	void	Redeclare(SCPPSignature* psSignature);
	bool	Overrides(CCPPFunction* pcVirtual);
	bool	TakesObject(ECPPCalledObject eObject);
	bool	IsCopy(void);
	int		QualifiersAdded(ECPPCalledObject eObject);
	bool	IsUnitFunction(void) const;
	void	SetDefinition(CCPPSyntaxFunctionDefinition* pcDefinition);
};


typedef CArrayTemplatePtr<CCPPFunction>	CArrayCPPFunctions;


//The functions of a name, in the order they were added: pcFirst then each mpcNextOfName.
struct SCPPFunctionsOfName
{
	CCPPFunction*	pcFirst;
	CCPPFunction*	pcLast;
};


typedef CHashMapLongTemplate<SCPPFunctionsOfName>	CMapCPPFunctionsOfNames;


#endif // __C_P_P_FUNCTION_H__
