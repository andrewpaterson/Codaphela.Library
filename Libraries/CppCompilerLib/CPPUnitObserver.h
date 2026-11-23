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
#ifndef __C_P_P_UNIT_OBSERVER_H__
#define __C_P_P_UNIT_OBSERVER_H__
#include "BaseLib/PrimitiveTypes.h"


class CCPPUnit;
class CCPPSyntax;
class CCPPSymbol;
class CCPPSyntaxDeclarator;
struct SCPP65Type;


//What the unit tells whoever checks it as written (CCPPSemanticPass): an expression's type and what changes it, what a
//name resolved to, and the enums a refactoring asks about.  This one hears nothing, and is the unit's when code is
//made (CCPPUnit::GetObserver).
class CCPPUnitObserver
{
public:
	virtual void	Typed(CCPPSyntax* pcExpression, SCPP65Type* psType);
	virtual void	PlaceTyped(CCPPSyntax* pcExpression, SCPP65Type* psType);
	virtual void	Changed(CCPPSyntax* pcExpression, uint8 uiChanges);
	virtual void	Resolved(int iToken, CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator);
	virtual void	EnumMade(CCPPUnit* pcUnit, SCPP65Type* psType);
	virtual bool	IsRecordedEnum(SCPP65Type* psType);
	virtual bool	RecordEnumCast(CCPPSyntax* pcNode, SCPP65Type* psFrom, SCPP65Type* psTo, uint8 uiKind);
};


#endif // __C_P_P_UNIT_OBSERVER_H__

