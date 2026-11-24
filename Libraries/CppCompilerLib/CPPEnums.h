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
#ifndef __C_P_P_ENUMS_H__
#define __C_P_P_ENUMS_H__
#include "BaseLib/HashMapPrimitiveTemplate.h"
#include "CPPSyntaxes.h"
#include "CPPSymbol.h"


class CCPPUnit;
class CCPPDiagnostics;
class CCPP65Types;
struct SCPP65Type;


//The unit's enums (CCPPUnit::GetEnums): an enum's type, from its underlying type or from its enumerators' values, and
//each enumerator's type and value, worked out once, and that a value fits the type it is given.  [dcl.enum]
class CCPPEnums
{
protected:
	CCPPUnit*			mpcUnit;
	CCPPDiagnostics*	mpcDiagnostics;
	CCPP65Types*		mpcTypes;
	CHashMapPrimitiveTemplate<CCPPSyntaxEnumerator*, int64>	mcEnumeratorValues;	//The values enumerators are given, folded once (GivenEnumeratorValue).
	CHashMapPrimitiveTemplate<CCPPSyntaxEnumerator*, SCPP65Type*>	mcEnumeratorBodyTypes;	//While EnumType works out an enum's type, each of its enumerators' so far: its value's.  [dcl.enum] 5

public:
	void	Init(CCPPUnit* pcUnit);
	void	Kill(void);

	bool					CheckEnumerators(CCPPSyntax* pcType);
	void					CheckEnumeratorFits(CCPPSyntaxEnumSpecifier* pcEnum, CCPPSyntaxEnumerator* pcEnumerator, int64 lliValue);
	bool					GivenEnumeratorValue(CCPPSyntaxEnumSpecifier* pcEnum, size uiEnumerator, int64* plliValue);
	bool					FoldEnumeratorValue(CCPPSyntaxEnumSpecifier* pcEnum, CCPPSyntaxEnumerator* pcEnumerator, int64* plliValue);
	static bool				IsOutsideTemplates(CCPPSyntax* pcNode);
	void					SetEnumeratorBodyType(CCPPSyntaxEnumSpecifier* pcSpecifier, size uiEnumerator, SCPP65Type* psFixed);
	void					ForgetEnumeratorBodyTypes(CCPPSyntaxEnumSpecifier* pcSpecifier);
	bool					EnumeratorValue(CCPPSymbol* pcSymbol, int64* plliValue);
	SCPP65Type*				EnumType(CCPPSymbol* pcEnum, CCPPSyntaxEnumSpecifier* pcSpecifier);
	SCPP65Type*				EnumeratorType(CCPPSymbol* pcSymbol);
};


#endif // __C_P_P_ENUMS_H__

