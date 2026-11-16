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
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"


//The names of functions that aren't identifiers, below -1 (which is no name): an operator is named by its token
//(operator() is the first token's), new[] and delete[] apart from new and delete, and every conversion function
//(operator T) has one name; they differ by their result.
#define CPP_OPERATOR_NAME(eToken)		(-2 - (int64)(eToken))
#define CPP_ARRAY_OPERATOR_NAME(eToken)	(CPP_OPERATOR_NAME(eToken) - (int64)NUM_CPP_TOKENS)
#define CPP_CONVERSION_NAME				(CPP_OPERATOR_NAME(0) - 2 * (int64)NUM_CPP_TOKENS)


//The object a member function is called on, which chooses between a const function and one that isn't, and
//between one declared & and one declared &&.
enum ECPPCalledObject
{
	CPPCO_None,
	CPPCO_Mutable,
	CPPCO_Const,
	CPPCO_MutableRvalue,	//An object with no name, or one about to go (CCPPUnit::IsRvalue).
	CPPCO_ConstRvalue,
};


//The object called on, from whether it is const and whether it is an rvalue.
inline ECPPCalledObject CPPCalledObject(bool bConst, bool bRvalue = false)
{
	return bRvalue ? (bConst ? CPPCO_ConstRvalue : CPPCO_MutableRvalue) : (bConst ? CPPCO_Const : CPPCO_Mutable);
}


inline bool CPPCalledConst(ECPPCalledObject eObject)
{
	return (eObject == CPPCO_Const) || (eObject == CPPCO_ConstRvalue);
}


inline bool CPPCalledRvalue(ECPPCalledObject eObject)
{
	return (eObject == CPPCO_MutableRvalue) || (eObject == CPPCO_ConstRvalue);
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


class CCPPClass;
class CCPPTemplate;
class CCPPTemplateArguments;


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
	bool							mbConstThis;	//A const member function: an overload of the same function without const.
	bool							mbConstexpr;	//Declared constexpr: its value may be worked out when the program is compiled.
	bool							mbNoexcept;		//Doesn't throw: declared noexcept or throw(), or a destructor or operator delete declared without either.
	CCPPFunction*					mpcAddressOf;	//What a closure's conversion to a pointer to a function gives the address of (its invoker), or NULL.
	ECPPRefQualifier				meRefQualifier;	//Declared & or &&: an overload of the same function declared the other.
	bool							mbImplicit;		//A constructor or destructor the compiler wrote.
	bool							mbDeleted;		//= delete: it is an overload like any other, and an error if it is the one chosen.
	CCPPFunction*					mpcInherited;	//The base's constructor an inherited constructor calls (using Base::Base).
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

	bool	IsConversion(void) { return mlliName == CPP_CONVERSION_NAME; }	//operator T.
	bool	IsOperator(void) { return mlliName < 0; }						//An operator or a conversion: its name is no identifier.

	size	RequiredParameters(void);
	bool	SameParameters(CCPPFunction* pcOther);
	bool	Overrides(CCPPFunction* pcVirtual);
	bool	TakesObject(ECPPCalledObject eObject);
	bool	IsUnitFunction(void);
	void	SetDefinition(CCPPSyntaxFunctionDefinition* pcDefinition);
};


typedef CArrayTemplatePtr<CCPPFunction>	CArrayCPPFunctions;


//The functions of a name, in the order they were added: pcFirst then each mpcNextOfName.
struct SCPPFunctionsOfName
{
	CCPPFunction*	pcFirst;
	CCPPFunction*	pcLast;
};


typedef CMapLongTemplate<SCPPFunctionsOfName>	CMapCPPFunctionsOfNames;


#endif // __C_P_P_FUNCTION_H__
