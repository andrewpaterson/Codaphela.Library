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
#include "CPP65Generator.h"


//The generator's overload resolution: finding the functions a call may mean, argument dependent lookup and ranking conversions.


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is an object that is const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstObject(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType && psType->bConst;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is a pointer to const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstPointer(CCPPSyntax* pcExpression)
{
	SCPP65Type*	psType;

	psType = ValueTypeOf(pcExpression);
	return psType && psType->IsPointer() && psType->psBase->bConst;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function being generated is a const member function.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsConstThis(void)
{
	return mcContext.GetFunction() && mcContext.GetFunction()->mbConstThis;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcClass is psBase or derives from it.  *piOffset (if not NULL) is set to
// where psBase is in pcClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPP65Generator::IsDerived(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset)
{
	SCPPBase*	psDirect;
	size		i;
	int			iOffset;

	if ((pcClass != pcBase) && !pcClass->IsComplete())
	{
		//The bases are enough to know; their offsets need the class laid out.
		BeginQuiet();
		ResolveBases(pcClass, NULL);
		EndQuiet();
	}
	if (pcClass == pcBase)
	{
		if (piOffset)
		{
			*piOffset = 0;
		}
		return true;
	}
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psDirect = pcClass->macBases.Get(i);
		if (IsDerived(psDirect->pcClass, pcBase, &iOffset))
		{
			if (piOffset)
			{
				*piOffset = psDirect->iOffset + iOffset;
			}
			return true;
		}
	}
	return false;
}
