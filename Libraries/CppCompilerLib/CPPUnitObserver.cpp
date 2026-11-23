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
#include "CPPUnitObserver.h"


//////////////////////////////////////////////////////////////////////////
//
// The unit typed an expression (CCPPCheck::Typed).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitObserver::Typed(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The unit found the type of the place an expression is.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitObserver::PlaceTyped(CCPPSyntax* pcExpression, SCPP65Type* psType)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The place an expression is can be changed there (CPPPC_).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitObserver::Changed(CCPPSyntax* pcExpression, uint8 uiChanges)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The name whose last component is the token iToken resolved to
// pcSymbol (declared by pcDeclarator, a function's).
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitObserver::Resolved(int iToken, CCPPSymbol* pcSymbol, CCPPSyntaxDeclarator* pcDeclarator)
{
}


//////////////////////////////////////////////////////////////////////////
//
// The unit made an enum's type.
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnitObserver::EnumMade(CCPPUnit* pcUnit, SCPP65Type* psType)
{
}


//////////////////////////////////////////////////////////////////////////
//
// Whether psType is the enum a refactoring asks about.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnitObserver::IsRecordedEnum(SCPP65Type* psType)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// A cast the asked about enum would need, recorded instead of refused.
// False: none is recorded.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnitObserver::RecordEnumCast(CCPPSyntax* pcNode, SCPP65Type* psFrom, SCPP65Type* psTo, uint8 uiKind)
{
	return false;
}

