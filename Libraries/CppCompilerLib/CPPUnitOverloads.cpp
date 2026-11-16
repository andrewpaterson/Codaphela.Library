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
#include "CPPUnit.h"


//The generator's overload resolution: finding the functions a call may mean, argument dependent lookup and ranking conversions.


//////////////////////////////////////////////////////////////////////////
//
// Whether an expression is an object that is const.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsConstObject(CCPPSyntax* pcExpression)
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
bool CCPPUnit::IsConstPointer(CCPPSyntax* pcExpression)
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
bool CCPPUnit::IsConstThis(void)
{
	CCPPFunction*	pcFunction;

	//In a lambda's body, this is the enclosing member function's.
	pcFunction = mcContext.GetFunction();
	while (BodyLambda(pcFunction))
	{
		pcFunction = BodyLambda(pcFunction)->pcEnclosing;
	}
	return pcFunction && pcFunction->mbConstThis;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether pcClass is psBase or derives from it.  *piOffset (if not NULL) is set to
// where psBase is in pcClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsDerived(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset)
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
				//Through a virtual base there is none that is fixed: where the base is depends on the complete object.
				*piOffset = (psDirect->bVirtual || (iOffset == CPP_VIRTUAL_PATH)) ? CPP_VIRTUAL_PATH : (psDirect->iOffset + iOffset);
			}
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// The virtual base of pcClass (direct or not) that pcBase is, or that pcBase
// is a non-virtual base of, with where pcBase is in it (*piBelow); NULL if
// no virtual base of pcClass leads to pcBase.
//
//////////////////////////////////////////////////////////////////////////
SCPPVirtualBase* CCPPUnit::VirtualBaseOf(CCPPClass* pcClass, CCPPClass* pcBase, int* piBelow)
{
	SCPPVirtualBase*	psVirtual;
	size				i;
	int					iOffset;

	for (i = 0; i < pcClass->macVirtualBases.NumElements(); i++)
	{
		psVirtual = pcClass->macVirtualBases.Get(i);
		if (IsDerived(psVirtual->pcClass, pcBase, &iOffset) && (iOffset != CPP_VIRTUAL_PATH))
		{
			*piBelow = iOffset;
			return psVirtual;
		}
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// Where pcBase is in a complete object of pcClass (which is laid out): its
// fixed offset, or through a virtual base where pcClass puts that.  False if
// pcBase isn't a base of pcClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::CompleteOffset(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset)
{
	SCPPVirtualBase*	psVirtual;
	int					iBelow;

	if (!IsDerived(pcClass, pcBase, piOffset))
	{
		return false;
	}
	if (*piOffset != CPP_VIRTUAL_PATH)
	{
		return true;
	}
	psVirtual = VirtualBaseOf(pcClass, pcBase, &iBelow);
	*piOffset = psVirtual ? (psVirtual->iOffset + iBelow) : 0;
	return psVirtual != NULL;
}
