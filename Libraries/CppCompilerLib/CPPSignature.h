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
#ifndef __C_P_P_SIGNATURE_H__
#define __C_P_P_SIGNATURE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/Chars.h"
#include "CPPSyntaxes.h"
#include "CPP65Type.h"
#include "CPPFunction.h"


class CCPPClass;
class CCPPTemplate;
class CCPPScope;


//How a declaration declares its function (CCPPFunctionDeclarer::Declare).
enum ECPPDeclaredAs
{
	CPPDA_Declaration,
	CPPDA_Definition,
	CPPDA_Pure,			//A declaration of a pure virtual function: = 0.
};


//A function's signature as one declaration writes it: made once from its specifiers and declarator
//(CCPPFunctionDeclarer::ReadSignature).  A declaration of a function declared before is one it is declared by
//(CCPPFunction::IsDeclaredBy), and adds to it (CCPPFunction::Redeclare); a function declared first adopts it
//(CCPPFunction::Adopt).  [dcl.fct], [over.load]
struct SCPPSignature
{
	//What declares it.
	CCPPSyntaxDeclSpecifiers*	pcSpecifiers;		//Its specifiers, NULL for none (a constructor's).
	CCPPSyntaxDeclarator*		pcWritten;			//Its declarator, as written.
	CCPPClass*					pcClass;			//The class it is a member of, NULL for none.
	CCPPTemplate*				pcTemplate;			//The function template it is an instance of, NULL for none...
	CChars*						pszArguments;		//...and its template arguments, mangled.
	ECPPDeclaredAs				eAs;

	//What the declaration says.  A function declared by its type's name (typedef int Binary(int, int); Binary Add;)
	//has its parameters in the typedef's declarator and its return in the typedef's specifiers.
	CCPPSyntaxDeclSpecifiers*	pcTypeSpecifiers;	//The specifiers its return type is read from.
	CCPPSyntaxDeclarator*		pcDeclarator;		//The declarator with its parameters: pcWritten, or the typedef's.
	CCPPSyntaxFunctionSuffix*	pcSuffix;
	CCPPSyntaxName*				pcName;
	CCPPSyntaxNameComponent*	pcComponent;		//The last of its name's components.
	int64						lliName;
	ECPPFunctionKind			eKind;
	SCPP65Type*					psReturn;			//msUndeduced for auto, until its body says.
	CArrayCPP65Types			apsParameters;		//As its function type has them (CCPPFunctionDeclarer::ParameterType).
	CArraySyntax				apcDefaults;		//Each parameter's default argument, or NULL.
	bool						bVariadic;
	bool						bConstThis;
	bool						bVolatileThis;
	ECPPRefQualifier			eRefQualifier;
	CCPPScope*					pcNamespace;		//A function's that isn't a member, NULL for the global one.
	bool						bCLinkage;			//Declared in extern "C", and not a member.
	uint32						uiFlags;			//The specifiers' CPPDS_ flags.

	void	Init(CCPPSyntaxDeclSpecifiers* pcSpecifiers, CCPPSyntaxDeclarator* pcDeclarator, CCPPClass* pcClass, ECPPDeclaredAs eAs);
	void	Kill(void);

	bool	Says(uint32 uiFlag) const;
	bool	IsDefinition(void) const;
	bool	IsPure(void) const;
};


#endif // __C_P_P_SIGNATURE_H__

