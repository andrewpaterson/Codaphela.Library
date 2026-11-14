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
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"
#include "CPPFunction.h"


//A template parameter's value in an instantiation: a type, or a constant for a non-type parameter.
struct SCPPBinding
{
	CCPPSymbol*						pcParameter;
	SCPP65Type*						psType;
	int64							lliValue;
	CArrayTemplatePtr<SCPP65Type>*	papsPack;	//A parameter pack's types.
};


typedef CArrayTemplate<SCPPBinding>	CArrayCPPBindings;


//The template arguments of an instantiation.  mpcOuter is the class instance a member template is in.  mpcClass
//is the instance the bindings belong to, for the class's own name used without arguments inside it.
class CCPPTemplateArguments
{
public:
	CArrayCPPBindings		macBindings;
	CCPPTemplateArguments*	mpcOuter;
	CCPPClass*				mpcClass;

	void			Init(void);
	void			Kill(void);

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
	CCPPSyntax*						mpcDefinition;	//A class specifier or a function definition.
	bool							mbMember;
	bool							mbMemberTemplate;
	CCPPClass*						mpcClass;
	int64							mlliName;
	CCPPSyntaxTemplateDeclaration*	mpcClassDeclaration;
	CCPPSyntaxDeclarator*			mpcDeclarator;	//A member template's declarator in its class.
	bool							mbStatic;
	CCPPTemplate*					mpcPrimary;
	CCPPSyntaxNameComponent*		mpcPattern;
	CCPPTemplate*					mpcOwner;
	bool							mbArguments;	//mszArguments is worked out (a function specialisation's).
	CCPPScope*						mpcNamespace;	//The namespace it was declared in, NULL for the global one: of an instance with no symbol (an operator).
	CChars							mszArguments;
	CArrayCPPFunctions				mapcInstances;	//A function template's instances.

	void	Init(CCPPSymbol* pcSymbol, CCPPSyntaxTemplateDeclaration* pcDeclaration, CCPPSyntax* pcDefinition, bool bMember, CCPPScope* pcNamespace);
	void	Kill(void);
};


typedef CArrayTemplatePtr<CCPPTemplate>	CArrayCPPTemplates;


#endif // __C_P_P_TEMPLATE_H__
