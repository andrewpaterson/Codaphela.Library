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
#ifndef __C_P_P_MANGLER_H__
#define __C_P_P_MANGLER_H__
#include "BaseLib/Chars.h"
#include "CppTokeniser.h"
#include "CPPScope.h"
#include "CPP65Type.h"
#include "CPPVariable.h"
#include "CPPFunction.h"
#include "CPPClass.h"


//Makes the labels of a unit's functions, classes and variables: their names in the assembly, which every
//unit that names the same thing must make the same (hand-written assembly calls them too).  What it needs of
//the unit is its tokeniser (the identifiers), its classes (a class without a name is numbered by its place)
//and its functions by name (an overloaded member's label has its parameters).
class CCPPMangler
{
protected:
	CCPPTokeniser*				mpcTokeniser;
	CArrayCPPClasses*			mpapcClasses;
	CMapCPPFunctionsOfNames*	mpcFunctionsOfNames;
	CArrayScopes				mapcUnnamedNamespaces;	//Their places are the numbers in the labels of what they declare.

public:
	void				Init(CCPPTokeniser* pcTokeniser, CArrayCPPClasses* papcClasses, CMapCPPFunctionsOfNames* pcFunctionsOfNames);
	void				Kill(void);

	void				AppendFunctionLabel(CChars* psz, CCPPFunction* pcFunction);
	void				AppendGlobalLabel(CChars* psz, CCPPVariable* pcVariable);
	void				AppendClassLabel(CChars* psz, CCPPClass* pcClass, const char* szPart);
	void				AppendVtableLabel(CChars* psz, CCPPClass* pcClass);
	void				AppendTypeInfoLabel(CChars* psz, CCPPClass* pcClass);
	void				AppendVtableEntryLabel(CChars* psz, CCPPFunction* pcFunction, int iAdjust);
	void				AppendClassName(CChars* psz, CCPPClass* pcClass);
	void				AppendScopeNames(CChars* psz, CCPPScope* pcScope);
	void				AppendFunctionName(CChars* psz, int64 lliName);
	void				AppendType(CChars* psz, SCPP65Type* psType);

protected:
	bool				IsOverloaded(CCPPFunction* pcFunction);
	CCPPFunction*		FirstOfName(int64 lliName);
	static const char*	GetOperatorLabel(int64 lliName);
};


#endif // __C_P_P_MANGLER_H__
