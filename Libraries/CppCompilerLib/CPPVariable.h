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
#ifndef __C_P_P_VARIABLE_H__
#define __C_P_P_VARIABLE_H__
#include "BaseLib/PrimitiveTypes.h"
#include "BaseLib/ArrayTemplatePtr.h"
#include "CPPSymbol.h"
#include "CPP65Type.h"


enum ECPPStorage
{
	CPPVS_Local,
	CPPVS_Parameter,
	CPPVS_Global,
	CPPVS_Static,		//A static local: data, numbered in the unit by miOffset.
};


class CCPPClass;
class CCPPSyntax;


//A variable that has been given storage: a global, a static data member, a static local, a parameter or a local.
class CCPPVariable
{
public:
	CCPPSymbol*		mpcSymbol;
	SCPP65Type*		mpsType;
	ECPPStorage		meStorage;
	int				miOffset;				//Parameters: the index in the function's parameters.  Static locals: the number (the label's S<n>_).  Members of an anonymous union or struct (mpcAnonymous): the offset in it.
	CCPPClass*		mpcClass;				//The class of a static data member.
	CCPPSyntax*		mpcInClassInitialiser;	//static const int k = 5; in the class: what its definition has if it has none.
	bool			mbInternal;				//A static global, or one in a namespace without a name: the unit's own.
	bool			mbCLinkage;				//Declared in extern "C": its label is only its name.
	bool			mbDefined;				//A global's data has been written.
	bool			mbUsed;
	bool			mbRestrict;				//A global or parameter pointer declared __restrict.
	CCPPClass*		mpcAnonymous;			//The anonymous union or struct declared in a function that this local is a member of.

	void	Init(CCPPSymbol* pcSymbol, SCPP65Type* psType, ECPPStorage eStorage, int iOffset, bool bCLinkage);
};


typedef CArrayTemplatePtr<CCPPVariable>	CArrayCPPVariables;


#endif // __C_P_P_VARIABLE_H__
