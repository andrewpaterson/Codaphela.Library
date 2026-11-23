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
#ifndef __C_P_P_TYPE_WRITER_H__
#define __C_P_P_TYPE_WRITER_H__
#include "BaseLib/Chars.h"
#include "CPP65Type.h"


class CCPPUnit;
class CCPPSymbol;
class CCPPClass;
class CCPPScope;
class CCPPFunction;
class CCPPVariable;


//What a type is written for (CCPPTypeWriter::Init).
enum ECPPTypeWriting
{
	CPPTW_Source,	//To declare it in the program, for the IDE's refactorings.  What can't be written makes the writing false.
	CPPTW_Shown,	//To show a reader: type_info::name, what an error says of a type, a class's layout.  Never false.
};


//The compiler's one writer of a type as C++ source writes it: "const char*", "NS::CFoo&", "CArray<int>*",
//"int (*)[3]", "void (*)(int)", "int A::*", and a declaration of a name with it, "int (*pf)(char)" and "char ac[4]".
//Pointers and references are written against the type, as the house style has them.
//
//A class or an enum is written with the namespaces and classes it is in, a class template's instance with its
//arguments, and a member class of an instance after that instance (P<char>::Q<long>).  The compiler's types have no
//typedefs in them, so none is written: uint32 is "unsigned int" and size is "unsigned long long" (the host's).  An
//integer is named by its size as the compiler has its types, so a long the size of an int is "int" and wchar_t is
//"unsigned short" (CppParserTodo.txt question 13).
//
//In a template checked as itself (CCPPUnit::CheckTemplatesAsThemselves) a type parameter's placeholder is written as
//the parameter's name, and the template's own instance with its parameters' names: "M*", "CArray<M, N>&".
//
//What it is written for (ECPPTypeWriting) decides only these:
//  - What has no name: in the source it can't be written (false, with nothing appended); shown, a closure is
//    <lambda>, a class without a name <unnamed>, an enum without one (unnamed enum) and what isn't a type ?.
//  - A namespace without a name: in the source what is in it is written as if in the one around it; shown, it is
//    (anonymous namespace)::.
//  - A function without parameters: (void) in the source, () shown.
//  - An array of unknown bound: [] in the source; shown, with the count the compiler has for it.
//  - A pointer or reference template argument (&g): in the source as written; shown, as its value.
class CCPPTypeWriter
{
protected:
	CCPPUnit*		mpcUnit;
	ECPPTypeWriting	meWriting;

public:
	void	Init(CCPPUnit* pcUnit, ECPPTypeWriting eWriting = CPPTW_Source);

	bool	AppendType(SCPP65Type* psType, CChars* pszDest);
	bool	AppendDeclaration(SCPP65Type* psType, const char* szName, CChars* pszDest);
	bool	AppendClass(CCPPClass* pcClass, CChars* pszDest);
	bool	AppendFunction(CCPPFunction* pcFunction, CChars* pszDest);
	bool	AppendVariable(CCPPVariable* pcVariable, CChars* pszDest);
	bool	AppendIdentifier(int64 lliName, CChars* pszDest);

protected:
	bool	Declaration(SCPP65Type* psType, CChars* pszOperators, CChars* pszCore, CChars* pszDest);
	void	Parenthesise(CChars* pszOperators, CChars* pszCore);
	void	Qualifiers(SCPP65Type* psType, CChars* pszDest);
	bool	Named(SCPP65Type* psType, CChars* pszDest);
	bool	ClassName(CCPPClass* pcClass, CChars* pszDest);
	bool	Arguments(CCPPClass* pcClass, CChars* pszDest);
	bool	ScopeName(CCPPScope* pcScope, CChars* pszDest);
	bool	ParameterName(CCPPSymbol* pcSymbol, CChars* pszDest);
	bool	Unnamed(const char* szShown, CChars* pszDest);
	bool	StartsWithName(CChars* psz);
};


#endif // __C_P_P_TYPE_WRITER_H__

