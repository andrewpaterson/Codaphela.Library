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
	int64							mlliName;		//An identifier, or an operator (see CCPP65Generator::GetOperatorName).
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
	bool							mbImplicit;		//A constructor or destructor the compiler wrote.
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
	CCPPFunction*					mpcNextOfName;	//The next function added with the same mlliName (see CCPP65Generator::FirstFunctionOfName).

	void	Init(CCPPClass* pcClass, int64 lliName, ECPPFunctionKind eKind, SCPP65Type* psReturn);
	void	Kill(void);

	size	RequiredParameters(void);
	bool	SameParameters(CCPPFunction* pcOther);
	bool	Overrides(CCPPFunction* pcVirtual);
	bool	TakesObject(int iConstObject);
	bool	IsUnitFunction(void);
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
