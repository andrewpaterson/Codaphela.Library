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
#ifndef __C_P_P_ATTRIBUTES_H__
#define __C_P_P_ATTRIBUTES_H__
#include "BaseLib/PrimitiveTypes.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"


class CCPPUnit;
class CCPPDiagnostics;


//The attributes a declaration is written with (CCPPUnit::GetAttributes), [[...]] and __attribute__((...)): whether it
//has one of a name and where, and what [[deprecated]] says where a deprecated name is used.  [dcl.attr]
class CCPPAttributes
{
protected:
	CCPPUnit*			mpcUnit;
	CCPPDiagnostics*	mpcDiagnostics;
	int					miSaysDeprecated;	//Whether the source says deprecated anywhere (SaysDeprecated): 1, 0, or -1 until it is asked.

public:
	void	Init(CCPPUnit* pcUnit);
	void	Kill(void);

	void					WarnDeprecated(CCPPSyntax* pcNode, const char* szWhat, int32 iAttribute);
	void					WarnDeprecatedSymbol(CCPPSymbol* pcSymbol, CCPPSyntax* pcNode);
	bool					HasUsedAttribute(CCPPSyntaxDeclarator* pcDeclarator);
	bool					HasAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName);
	int32					FindAttribute(CCPPSyntaxDeclarator* pcDeclarator, const char* szName);
	int32					FindAttribute(CArraySyntax* papcAttributes, const char* szName);
	int32					DeprecatedAttribute(CCPPSymbol* pcSymbol);
	bool					SaysDeprecated(void);
};


#endif // __C_P_P_ATTRIBUTES_H__

