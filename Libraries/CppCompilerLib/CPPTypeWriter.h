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


//Writes a type the compiler worked out as C++ source would, for the IDE's refactorings to declare what they make
//(CCPPSemanticPass::AppendType): "const char*", "NS::CFoo&", "CArray<int>*", and a declaration of a name with it,
//"int (*pf)(char)" and "char ac[4]".  Pointers and references are written against the type, as the house style has
//them.
//
//A class or an enum is written with the namespaces and classes it is in, and a class template's instance with its
//arguments.  The compiler's types have no typedefs in them, so none is written: uint32 is "unsigned int" and
//size is "unsigned long long" (the host's).  An integer the size of the target's int is "int", so a long of that
//size is too.  What has no name can't be written (a class or enum without one): false, with nothing appended.
class CCPPTypeWriter
{
protected:
	CCPPUnit*	mpcUnit;

public:
	void	Init(CCPPUnit* pcUnit);

	bool	AppendType(SCPP65Type* psType, CChars* pszDest);
	bool	AppendDeclaration(SCPP65Type* psType, const char* szName, CChars* pszDest);

protected:
	bool	Declaration(SCPP65Type* psType, CChars* pszOperators, CChars* pszCore, CChars* pszDest);
	void	Parenthesise(CChars* pszOperators, CChars* pszCore);
	bool	Named(SCPP65Type* psType, CChars* pszDest);
	bool	ClassName(CCPPClass* pcClass, CChars* pszDest);
	bool	QualifiedName(CCPPSymbol* pcSymbol, CChars* pszDest);
	void	Qualifiers(SCPP65Type* psType, CChars* pszDest);
};


#endif // __C_P_P_TYPE_WRITER_H__

