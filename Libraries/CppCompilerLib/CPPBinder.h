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
#ifndef __C_P_P_BINDER_H__
#define __C_P_P_BINDER_H__
#include "BaseLib/MapLongTemplate.h"
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPPFunction.h"
#include "CPPClass.h"
#include "CPPVariable.h"
#include "CPPNames.h"
#include "CPPTemplate.h"
#include "CPPOverloads.h"
#include "CPPAccess.h"
#include "CPPLambda.h"


class CCPPUnit;
class CCPPDiagnostics;


//How a call is made.
enum ECPPBoundCall
{
	CPPBC_Failed,			//It can't be: an error says why.
	CPPBC_Member,			//A member function, on the object (this's if there is none): object.f(), pointer->f(), Class::f(), f(); and
	//object(), which is its operator().
	CPPBC_Destructor,		//object.~X(), pointer->~X() or ~X(): the class's destructor, or nothing if it has none.
	CPPBC_PseudoDestructor,	//object.~T() and pointer->~T() of a type that isn't a class: only the object's expression is worked out.
	CPPBC_MemberPointer,	//(object.*pointer)() and (pointer->*pointer)().
	CPPBC_Function,			//The function, by its label.
	CPPBC_Pointer,			//Through a pointer to a function, which the callee is.
	CPPBC_Builtin,			//A __builtin_ function, written where it is called.
	CPPBC_InlineMemcpy,		//memcpy of a few bytes, copied where it is called.
	CPPBC_Unchecked,		//An intrinsic of the host's compiler that no header declares (only when checking).
	CPPBC_Surrogate,		//object() through the pointer to a function its conversion function (mpcFunction) gives.  [over.call.object] 2
};


//What a call expression means, worked out once (CCPPBinder::Bind): how it is made, and for a function called
//by its label, which.  The type checker gives its type from this and the IR front end writes it from this.
class CCPPBoundCall
{
public:
	ECPPBoundCall		meHow;
	CCPPSyntax*			mpcCallee;		//What is called, without its parentheses.
	CCPPSyntaxName*		mpcName;		//The callee's name, or NULL if it isn't one.
	int64				mlliName;		//The function's name (-1 if the callee has none).
	CCPPFunction*		mpcFunction;	//The function chosen (a template's instance is made); NULL for a destructor there isn't.
	CArraySyntax*		mpapcArguments;	//A call expression's arguments: the node's own, or with args... expanded (the binder's).

	//A member's call and a destructor's.
	CCPPSyntax*			mpcObject;		//What it is called on: the left of . or ->, or NULL for this.
	CCPPClass*			mpcClass;		//The object's class (this's, or the one that qualifies the name, without an object).
	CCPPClass*			mpcQualifier;	//The base of the object's class (or of this's) that the name's qualifier names (p->C::f(), C::f()): the function's object is in it.  NULL without one.
	bool				mbArrow;		//The object is the left of ->.
	bool				mbVirtual;		//A virtual function is called through the vtable: not for a named object or a qualified name.

	//An operator's call: what was asked, so that it is the same question that is remembered.
	CCPPSyntax*			mpcLeft;		//The left operand, or the only one; the object for an operator of its own.
	CCPPSyntax*			mpcRight;		//The right operand or the argument, or NULL.
	bool				mbOwnOnly;		//Only the object's own operator was looked for (BindOwnOperator).

	//Where it was bound: the same call in another instance of a template is another call.
	CCPPTemplateArguments*	mpcBindings;
	CCPPFunction*			mpcIn;

	void	Init(void);
};


//What a name used as an expression is.
enum ECPPBoundName
{
	CPPBN_Nothing,			//Nothing an expression can be.
	CPPBN_FunctionName,		//__func__ and its like: the name of the function it is in, a string.
	CPPBN_Constant,			//An enumerator, or a template's parameter that is a value.
	CPPBN_Function,			//A function: its address.
	CPPBN_FunctionTemplate,	//A function template: the address of an instance.
	CPPBN_Variable,			//A local, a parameter, a global or a static member.
	CPPBN_Field,			//A field of this.
	CPPBN_Capture,			//A variable a lambda being written captures: its closure's field.
	CPPBN_EnclosingConstant,	//A constant of the function a lambda being written is in, which it doesn't capture: its value.
	CPPBN_TemplateArgument,	//A template's pointer or reference parameter: the expression its argument is (mpcArgument).
};


//What an id-expression means (CCPPBinder::BindName), or the name after . or -> for an object of a class
//(BindMember): which of the things a name can be, and the symbol, the variable or the field it is.  The type checker gives its type from this and the IR front end its value and place.
class CCPPBoundName
{
public:
	ECPPBoundName	meWhat;
	CCPPSymbol*		mpcSymbol;		//The name's symbol; a qualified enumerator's (Enum::Value) is found here.
	CCPPVariable*	mpcVariable;	//CPPBN_Variable.
	CCPPClass*		mpcClass;		//CPPBN_Field: the object's class (this's for a name alone)...
	SCPPField*		mpsField;		//...its field (or a base's)...
	int				miOffset;		//...and where it is in the object...
	CCPPClass*		mpcQualifier;	//...looked up in this base of the object's class that the name's qualifier names (d.B::a), or NULL...
	CCPPClass*		mpcVirtualBase;	//...in this virtual base of that class, which the object says where it is (or NULL).
	SCPPLambdaCapture*	mpsCapture;	//CPPBN_Capture: the capture (mpcClass is the closure's, mpsField its field).
	CCPPSyntax*		mpcArgument;	//CPPBN_TemplateArgument.
};


//How an object becomes a value of a type that isn't a class.
enum ECPPBoundConversion
{
	CPPBV_AsItIs,		//Not by a function of its class: it is used as it is (and converted, if it can be, as a value is).
	CPPBV_Function,		//By the class's conversion function (operator T, its own or a base's), whose result is then converted.
};


//What converts an object (CCPPBinder::BindConversion): the conversion function chosen, and the object's class.
class CCPPBoundConversion
{
public:
	ECPPBoundConversion	meHow;
	CCPPFunction*		mpcFunction;
	CCPPClass*			mpcClass;
};


//How an object of a class is made from arguments.
enum ECPPBoundConstruction
{
	CPPBO_Failed,		//It can't be: an error says why.
	CPPBO_Constructor,	//By the constructor chosen for the arguments.
	CPPBO_Nothing,		//The class has no constructor and there are no arguments: nothing runs.
	CPPBO_Aggregate,	//The class has no constructor: its fields take the arguments in order, the rest are zero.
};


//What makes an object (CCPPBinder::BindConstruction).
class CCPPBoundConstruction
{
public:
	ECPPBoundConstruction	meHow;
	CCPPFunction*			mpcConstructor;
};


//How many operator->s object-> goes through at most: an operator-> may return an object whose operator-> is then used.
#define CPP_MAX_ARROWS	16


typedef CHashMapLongTemplate<CCPPBoundCall>	CMapCPPBoundCalls;


//Says what the calls, names and members of a unit's expressions are, in one place, for the type checker and the
//IR front end to read.  It is the one way in to choosing a function: it owns the overloads.  [expr.call]
//Bind has the one cascade that says what a call is: a member's, a template's instance, a builtin, a function found
//by its name or by its arguments' namespaces, or a pointer's.  A call bound once is remembered for the template
//arguments and the function it is in; one that failed is bound again when it is asked for with its errors
//wanted, so they are reported again.  An operator with an object for an operand is a call as well (BindOperator:
//the left operand's own operator function if one fits, or else one that is no class's; BindOwnOperator: the
//object's own, for [], -> and what must be a member).  BindName says what a name alone is, BindMember which
//member of a class, BindConversion what converts an object to a value of another type, and BindConstruction what makes one.
//What it says a name or a call is must also be something that may be named there (CCPPAccess): a private member
//used outside its class is an error here.  Access is asked after the choosing, and never changes what is chosen.
class CCPPBinder
{
protected:
	CCPPUnit*			mpcUnit;
	CCPPDiagnostics*	mpcDiagnostics;	//Where its errors go: the unit's.
	CCPPOverloads		mcOverloads;	//Chooses between the functions of a name; only the binder asks it.
	CCPPAccess			mcAccess;		//Says whether what was chosen may be named where it is.
	CCPPNames			mcNames;		//What a name means as a class or a member.
	CMapCPPBoundCalls	mcBound;
	CMapCPPBoundCalls	mcOperators;	//By left operand.

public:
	void	Init(CCPPUnit* pcUnit);
	void	Kill(void);
	CCPPNames*	GetNames(void);

	bool	Bind(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound, bool bReport);
	bool	BindOperator(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode, CCPPBoundCall* pcBound, bool bReport);
	bool	BindOwnOperator(ECppTokens eOperator, CCPPSyntax* pcObject, CCPPSyntax* pcArgument, CCPPSyntax* pcNode, CCPPBoundCall* pcBound, bool bReport);

	void			BindName(CCPPSyntaxIdExpression* pcId, CCPPBoundName* pcBound);
	bool			BindMember(CCPPClass* pcClass, CCPPSyntaxName* pcName, CCPPSyntax* pcNode, CCPPBoundName* pcBound);
	bool			HasOneBase(CCPPClass* pcClass, CCPPClass* pcBase, CCPPSyntax* pcNode);
	bool			HasOneMember(CCPPClass* pcClass, CCPPClass* pcLookup, CCPPClass* pcDeclaring, CCPPSyntax* pcNode);
	bool			HasOneMeaning(CCPPClass* pcClass, int64 lliName, CCPPSyntax* pcNode);
	bool			ReachesBase(CCPPClass* pcClass, CCPPClass* pcBase, CCPPSyntax* pcNode);
	bool			ReachesDestructor(CCPPClass* pcClass, CCPPSyntax* pcNode);
	void			CheckTypeName(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
	bool			IsConvertible(SCPP65Type* psTo, SCPP65Type* psFrom);
	bool			IsConstructible(SCPP65Type* psType, CArrayCPP65Types* papsArguments);
	bool			IsAssignable(SCPP65Type* psTo, SCPP65Type* psFrom);
	CCPPFunction*	BindRangeFunction(int64 lliName, CCPPClass* pcClass, CCPPSyntax* pcRange, CCPPSyntax* pcNode);
	bool			BindConstruction(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode, CCPPBoundConstruction* pcBound, bool bReport);
	bool			IsCopyInitialiser(CCPPSyntax* pcValue);
	bool			ConstructsWithoutBraces(CCPPClass* pcClass, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool			HasMemberMadeByCode(CCPPClass* pcClass);
	bool			IsCopyDeleted(CCPPClass* pcClass);
	bool			IsMoveDeleted(CCPPClass* pcClass);
	bool			ReachesMember(CCPPSymbol* pcSymbol, CCPPClass* pcNaming, CCPPSyntax* pcNode);
	bool			UsingReaches(CCPPSymbol* pcSymbol, CCPPSyntax* pcUsing);
	bool			CopyInitialises(CCPPClass* pcClass, CCPPSyntaxInitDeclarator* pcInit);
	void			BindConversion(CCPPSyntax* pcExpression, SCPP65Type* psTo, bool bExplicit, CCPPBoundConversion* pcBound);
	void			BindObjectConversion(CCPPClass* pcClass, SCPP65Type* psTo, bool bExplicit, CCPPSyntax* pcNode, CCPPBoundConversion* pcBound);

	//What isn't a call expression's: these choose a function and remember nothing.
	CCPPFunction*	FindAddressedFunction(CCPPSyntaxName* pcName, int64 lliName, SCPP65Type* psTarget, CCPPSyntax* pcNode, bool* pbOverloaded);
	CCPPFunction*	FindMemberFunction(CCPPClass* pcClass, int64 lliName, SCPP65Type* psTarget, CCPPClass** ppcClass, bool* pbOverloaded);
	CCPPFunction*	FindLiteralOperator(int64 lliName, CArraySyntax* papcExplicit, CArraySyntax* papcArguments, CCPPSyntax* pcNode);
	bool			HasFreeOperator(int64 lliName);
	bool			ListConverts(SCPP65Type* psElement, CCPPSyntax* pcList);
	bool			InDependentBase(CCPPClass* pcClass, CCPPClass* pcDeclaring);
	CCPPFunction*	InitializerListConstructor(CCPPClass* pcClass, CCPPSyntax* pcList);
	CCPPFunction*	FindArrowOperator(CCPPClass* pcClass, CCPPSyntax* pcObject, CCPPSyntax* pcNode);
	CCPPFunction*	ConversionToClass(CCPPClass* pcClass, CCPPSyntax* pcSource, CCPPSyntax* pcNode);
	bool			ConvertsByConstructor(CCPPClass* pcClass, CCPPSyntax* pcSource);
	CCPPFunction*	FindAllocation(CCPPSyntaxNewExpression* pcNew, SCPP65Type* psType, CArraySyntax* papcArguments, bool* pbFailed);
	bool			IsBuiltinPlacement(CArraySyntax* papcArguments);
	bool			IsReservedPlacement(CCPPFunction* pcFunction);
	CCPPFunction*	FindDeallocation(CCPPClass* pcClass, bool bArray, bool bSized = false);
	bool			CopiesByConstructor(CCPPClass* pcClass, CCPPSyntax* pcSource);
	bool			KeepsArrayCount(SCPP65Type* psElement);
	bool			IsUsualDeallocation(CCPPFunction* pcFunction);
	bool			IsGlobalSizedDeallocation(CCPPFunction* pcFunction);
	bool			IsNearerDeallocation(CCPPFunction* pcFunction, CCPPFunction* pcFound);
	bool			HasMemberTemplate(CCPPClass* pcClass, int64 lliName, size uiArguments);
	bool			HasOwnMemberTemplate(CCPPClass* pcClass, int64 lliName, size uiArguments);
	bool			KeepsQualifiers(SCPP65Type* psTo, SCPP65Type* psFrom);
	SCPP65Type*		ContextualType(CCPPSyntax* pcExpression, ECPPContextualTypes eWanted);
	CCPPFunction*	FindIntegerConversion(CCPPClass* pcClass, CCPPSyntax* pcNode);

	CCPPClass*				MemberObjectClass(CCPPSyntaxMemberExpression* pcMember);
	bool					BindMemberOf(CCPPSyntaxMemberExpression* pcMember, CCPPBoundName* pcBound);
	CCPPSymbol*				MemberEnumerator(CCPPSyntaxMemberExpression* pcMember);
	SCPPField*				BitFieldOf(CCPPSyntax* pcExpression);
	CCPPSyntax*				BoundInitialiser(SCPP65Type* psReference, CCPPSyntax* pcInitialiser);
	bool					BindsToTemporary(SCPP65Type* psReference, CCPPSyntax* pcArgument);
	bool					IsUncapturedConstant(CCPPSyntax* pcExpression);
	bool					IsOverloadedComma(CCPPSyntaxBinaryExpression* pcBinary);
	bool					IsOperatorOperand(CCPPSyntax* pcExpression, ECppTokens eOperator);
	bool					IsNamed(int64 lliName, const char* szName);
	bool					IsStdName(int64 lliName, CCPPScope* pcScope, const char* szName);

	CCPPSyntaxIdExpression*	FunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo);
	CCPPSyntaxIdExpression*	MemberFunctionNameFor(CCPPSyntax* pcExpression, SCPP65Type* psTo);
	CCPPSyntaxIdExpression*	FunctionNameOf(CCPPSyntax* pcExpression);
	CCPPSyntaxIdExpression*	MemberPointerName(CCPPSyntaxUnaryExpression* pcAddress);
protected:
	ECPPCalledObject	CalledObject(CCPPSyntax* pcObject, bool bArrow);
	bool	IsNamedObject(CCPPSyntax* pcObject);
	void	Resolve(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound);
	void	ResolveName(CCPPSyntaxIdExpression* pcId, CCPPBoundName* pcBound);
	void	CheckAccess(CCPPBoundCall* pcBound, CCPPSyntax* pcNode);
	void	ResolveMember(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound);
	CCPPFunction*	FindConversion(CCPPClass* pcClass, CCPPSyntaxNameComponent* pcComponent, CCPPSyntax* pcNode);
	CCPPFunction*	FindConversionTo(CCPPClass* pcClass, SCPP65Type* psTarget);
	void	ResolvePseudoDestructor(CCPPSyntaxMemberExpression* pcMember, SCPP65Type* psType, CCPPBoundCall* pcBound);
	bool	ResolveCallOperator(CCPPSyntaxCallExpression* pcCall, CCPPBoundCall* pcBound);
	bool	BindAnOperator(ECppTokens eOperator, CCPPSyntax* pcLeft, CCPPSyntax* pcRight, CCPPSyntax* pcNode, CCPPBoundCall* pcBound, bool bReport, bool bOwnOnly);
	void	ResolveOperator(CCPPSyntax* pcNode, CCPPBoundCall* pcBound);
	void	ResolveOwnOperator(CCPPSyntax* pcNode, CCPPBoundCall* pcBound);
	bool	TakesAsTheyAre(CCPPFunction* pcFunction, CArraySyntax* papcArguments);
};


#endif // __C_P_P_BINDER_H__
