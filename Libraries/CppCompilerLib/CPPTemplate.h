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
#ifndef __C_P_P_TEMPLATE_H__
#define __C_P_P_TEMPLATE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "BaseLib/ArrayInt64.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "CPPFunction.h"


//A template parameter's value in an instantiation: a type, or a constant for a non-type parameter.
class CCPPTemplate;


struct SCPPBinding
{
	CCPPSymbol*						pcParameter;
	SCPP65Type*						psType;
	int64							lliValue;
	CArrayTemplatePtr<SCPP65Type>*	papsPack;	//A type parameter pack's types.
	CArrayInt64*					palliPack;	//A non-type parameter pack's values.
	CCPPTemplate*					pcTemplate;	//A template template parameter's class template.
	CCPPSymbol*						pcAlias;	//Or its alias template.  [temp.arg.template] 1
	CCPPSyntax*						pcArgument;	//A pointer or reference parameter's argument (&g, f or g), which its name
												//stands for (CPPBN_TemplateArgument).
	bool							bSelf;		//The parameter stands for itself (CCPPUnit::BindAsItself): its type is a
												//placeholder that is written as its name, and its value isn't known.

	bool	IsPack(void) const;
	size	PackSize(void) const;
};


typedef CArrayTemplate<SCPPBinding>	CArrayCPPBindings;


//A parameter pack deduced whole from the template arguments of an instance a parameter's type names (Ints<I...>
//from Ints<1, 2>): its types, or its values for a non-type pack.  [temp.deduct.type] 9
struct SCPPDeducedPack
{
	size								uiParameter;
	CArrayTemplatePtr<SCPP65Type>*		papsTypes;
	CArrayInt64*						palliValues;
};


typedef CArrayTemplate<SCPPDeducedPack>	CArrayCPPDeducedPacks;
typedef CArrayTemplatePtr<SCPPBinding>	CArrayCPPBindingPtrs;


//A template argument once its pack expansions are expanded (CCPPUnit::ExpandTemplateArguments): an argument as
//written (pcArgument), or one element of a pack expansion, which is its type or, for a non-type pack, its value.
struct SCPPExpandedArgument
{
	CCPPSyntax*		pcArgument;	//As written, or the pack expansion the element is of.
	SCPP65Type*		psType;		//An element's type, NULL for a value.
	int64			lliValue;	//An element's value.
	bool			bExpanded;	//An element of a pack expansion.
};


typedef CArrayTemplate<SCPPExpandedArgument>	CArrayCPPExpandedArguments;


//The template arguments of an instantiation.  mpcOuter is the class instance a member template is in.  mpcClass
//is the instance the bindings belong to, for the class's own name used without arguments inside it.
//
//With mbSelf each parameter stands for itself: the template as it is written, with no arguments yet, which is what
//a kept check types a template's body as (CCPPUnit::CheckTemplatesAsThemselves).
class CCPPTemplateArguments
{
public:
	CArrayCPPBindings		macBindings;
	CCPPTemplateArguments*	mpcOuter;
	CCPPClass*				mpcClass;
	bool					mbSelf;

	void			Init(void);
	void			Kill(void);
	bool			IsSelf(void);

	SCPPBinding*	Find(CCPPSymbol* pcParameter);
	SCPPBinding*	FindOwn(CCPPSymbol* pcParameter);
};


typedef CArrayTemplatePtr<CCPPTemplateArguments>	CArrayCPPTemplateArguments;


//A class or function template, a member function of a class template defined outside the class, or a member
//template.  A member template is kept for each class (or class template instance) it is in (mpcClass).  One
//defined outside its class is kept once, with mpcClass NULL, and found by the class's name (mpcSymbol) and its
//own name (mlliName); mpcClassDeclaration is the class template's parameters if the class is a template.  An
//explicit or partial specialisation has the template it specialises as mpcPrimary (a template is its own
//primary otherwise) and its template arguments in mpcPattern.  An outside member function's class template is
//mpcOwner, the primary or a partial specialisation.
class CCPPTemplate
{
public:
	CCPPSymbol*						mpcSymbol;		//The template's name; for an outside member, the class's.
	CCPPSyntaxTemplateDeclaration*	mpcDeclaration;
	CCPPSyntax*						mpcDefinition;	//A class specifier or a function definition; a class template only declared
													//so far, its elaborated type specifier (template<class T> struct S;), and a
													//function template only declared, NULL (mpcDeclarator is its declarator).
	bool							mbMember;
	bool							mbMemberTemplate;
	CCPPClass*						mpcClass;
	int64							mlliName;
	CCPPSyntaxTemplateDeclaration*	mpcClassDeclaration;
	CCPPSyntaxDeclarator*			mpcDeclarator;	//A member template's declarator in its class, or a function template's declared before it is defined...
	CCPPSyntaxDeclSpecifiers*		mpcSpecifiers;	//...and its specifiers there.
	bool							mbStatic;
	CCPPTemplate*					mpcPrimary;
	CCPPSyntaxNameComponent*		mpcPattern;
	CCPPTemplate*					mpcOwner;
	bool							mbArguments;	//mszArguments is worked out (a function specialisation's).
	CCPPScope*						mpcNamespace;	//The namespace it was declared in, NULL for the global one: of an instance with no symbol (an operator).
	CChars							mszArguments;
	CArrayCPPFunctions				mapcInstances;	//A function template's instances.
	bool							mbAsItself;		//It has been instantiated as itself (CheckTemplatesAsThemselves), or couldn't be.

	void	Init(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember, CCPPScope* pcNamespace);
	void	Kill(void);

	ECPPSyntax	DefinitionKind(void);
	bool		IsDeclaredClass(void) const;
	bool		IsFunction(void) const;

	CCPPSyntaxDeclarator*		FunctionDeclarator(void);
	CCPPSyntaxDeclSpecifiers*	FunctionSpecifiers(void);
};


typedef CArrayTemplatePtr<CCPPTemplate>	CArrayCPPTemplates;


#endif // __C_P_P_TEMPLATE_H__
