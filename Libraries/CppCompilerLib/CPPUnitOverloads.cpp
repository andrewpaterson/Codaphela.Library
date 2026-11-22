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


//What this is in a member function, and a class's bases: whether one class is derived from another, how many times, at what
//offset and through which virtual base.  (Choosing a function is CCPPOverloads'.)


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
// The function whose this the function being generated has: itself, or in
// a lambda's body the member function the lambda is in.
//
//////////////////////////////////////////////////////////////////////////
CCPPFunction* CCPPUnit::ThisFunction(void)
{
	CCPPFunction*	pcFunction;

	pcFunction = mcContext.GetFunction();
	while (BodyLambda(pcFunction))
	{
		pcFunction = BodyLambda(pcFunction)->pcEnclosing;
	}
	return pcFunction;
}


//////////////////////////////////////////////////////////////////////////
//
// Whether the function being generated is a const, or a volatile, member
// function: its this points to an object that is.  [class.this] 1
//
//////////////////////////////////////////////////////////////////////////
bool CCPPUnit::IsConstThis(void) { return !mcContext.GetObject() && ThisFunction() && ThisFunction()->mbConstThis; }
bool CCPPUnit::IsVolatileThis(void) { return !mcContext.GetObject() && ThisFunction() && ThisFunction()->mbVolatileThis; }


//////////////////////////////////////////////////////////////////////////
//
// The type of this in the function being generated, for an object of
// pcClass: a pointer to it, const or volatile as the function is.
//
//////////////////////////////////////////////////////////////////////////
SCPP65Type* CCPPUnit::ThisType(CCPPClass* pcClass)
{
	return mcTypes.Pointer(mcTypes.Qualified(pcClass->mpsType, IsConstThis(), IsVolatileThis()));
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

	mpcStatistics->Count(CPPCC_BaseWalks);
	if ((pcClass != pcBase) && !pcClass->IsComplete())
	{
		//The bases are enough to know; their offsets need the class laid out.
		mcDiagnostics.BeginQuiet();
		ResolveBases(pcClass, NULL);
		mcDiagnostics.EndQuiet();
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
// How many pcBase subobjects an object of pcClass has: itself, if it is the
// class, and one for each way to the base that passes no virtual base, from
// the class and from each of its virtual bases (of which an object has one,
// however many ways reach it).  More than one, and the base is ambiguous: a
// pcClass doesn't become a pcBase.  [class.mi] 4, [conv.ptr] 3
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnit::CountBases(CCPPClass* pcClass, CCPPClass* pcBase)
{
	CArrayCPPClasses	apcVirtual;
	size				uiCount;
	size				i;

	mpcStatistics->Count(CPPCC_BaseWalks);
	if (!IsDerived(pcClass, pcBase, NULL))
	{
		return 0;
	}
	apcVirtual.Init();
	CollectVirtualBases(pcClass, &apcVirtual);
	uiCount = CountFixedBases(pcClass, pcBase);
	for (i = 0; i < apcVirtual.NumElements(); i++)
	{
		uiCount += CountFixedBases(apcVirtual.GetPtr(i), pcBase);
	}
	apcVirtual.Kill();
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// The virtual bases of pcClass, direct or not, each once.  (A class that is
// laid out has them in macVirtualBases; this asks only for its bases.)
//
//////////////////////////////////////////////////////////////////////////
void CCPPUnit::CollectVirtualBases(CCPPClass* pcClass, CArrayCPPClasses* papcVirtual)
{
	SCPPBase*	psBase;
	size		i;

	mpcStatistics->Count(CPPCC_BaseWalks);
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (psBase->bVirtual && (papcVirtual->Find(psBase->pcClass) == ARRAY_ELEMENT_NOT_FOUND))
		{
			papcVirtual->Add(psBase->pcClass);
		}
		CollectVirtualBases(psBase->pcClass, papcVirtual);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many pcBase subobjects are at a fixed place in pcClass's own part:
// itself, or those of its bases that aren't virtual.
//
//////////////////////////////////////////////////////////////////////////
size CCPPUnit::CountFixedBases(CCPPClass* pcClass, CCPPClass* pcBase)
{
	SCPPBase*	psBase;
	size		uiCount;
	size		i;

	mpcStatistics->Count(CPPCC_BaseWalks);
	if (pcClass == pcBase)
	{
		return 1;
	}
	uiCount = 0;
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		psBase = pcClass->macBases.Get(i);
		if (!psBase->bVirtual)
		{
			uiCount += CountFixedBases(psBase->pcClass, pcBase);
		}
	}
	return uiCount;
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

	mpcStatistics->Count(CPPCC_BaseWalks);
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

	mpcStatistics->Count(CPPCC_BaseWalks);
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
