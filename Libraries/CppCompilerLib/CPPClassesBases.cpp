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
#include "CPPClasses.h"


//A class's bases (CCPPClasses): whether one class is derived from another, how many times, at what offset and through
//which virtual base.


//////////////////////////////////////////////////////////////////////////
//
// Whether pcClass is psBase or derives from it.  *piOffset (if not NULL) is set to
// where psBase is in pcClass.
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::IsDerived(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset)
{
	CArrayCPPBasePaths*	pasPaths;
	SCPPBasePath*		psPath;
	size				i;

	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
	if (pcClass == pcBase)
	{
		if (piOffset)
		{
			*piOffset = 0;
		}
		return true;
	}
	pasPaths = BasePaths(pcClass);
	for (i = 1; i < pasPaths->NumElements(); i++)
	{
		psPath = pasPaths->Get(i);
		if (psPath->pcClass == pcBase)
		{
			if (piOffset)
			{
				//Through a virtual base there is none that is fixed: where the base is depends on the complete object.
				*piOffset = psPath->pcVirtual ? CPP_VIRTUAL_PATH : psPath->iOffset;
			}
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// pcClass's base paths (CCPPClass::BasePaths), its bases resolved first,
// and theirs, where the class isn't complete: knowing the bases is enough,
// their offsets need the class laid out.
//
//////////////////////////////////////////////////////////////////////////
CArrayCPPBasePaths* CCPPClasses::BasePaths(CCPPClass* pcClass)
{
	if (!pcClass->IsComplete())
	{
		mpcDiagnostics->BeginQuiet();
		ResolveAllBases(pcClass);
		mpcDiagnostics->EndQuiet();
	}
	return pcClass->BasePaths();
}


//////////////////////////////////////////////////////////////////////////
//
// Resolves the bases of pcClass, and of each of them, that are not known.
//
//////////////////////////////////////////////////////////////////////////
void CCPPClasses::ResolveAllBases(CCPPClass* pcClass)
{
	size	i;

	if (pcClass->IsComplete())
	{
		return;
	}
	ResolveBases(pcClass, NULL);
	for (i = 0; i < pcClass->macBases.NumElements(); i++)
	{
		ResolveAllBases(pcClass->macBases.Get(i)->pcClass);
	}
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcBase is a base of pcClass in one way only, which passes no
// virtual base and is public at every step: a pcBase is then the base of at
// most one pcClass object, *piOffset bytes into it.  (pcClass itself isn't.)
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::IsOnePublicBase(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset)
{
	CArrayCPPBasePaths*	pasPaths;
	SCPPBasePath*		psPath;
	SCPPBasePath*		psOne;
	size				i;

	*piOffset = 0;
	if (pcClass == pcBase)
	{
		return false;
	}
	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
	pasPaths = pcClass->BasePaths();
	psOne = NULL;
	for (i = 1; i < pasPaths->NumElements(); i++)
	{
		psPath = pasPaths->Get(i);
		if (psPath->pcClass == pcBase)
		{
			if (psOne || psPath->pcVirtual)
			{
				return false;
			}
			psOne = psPath;
		}
	}
	if (psOne)
	{
		*piOffset = psOne->iOffset;
	}
	return psOne && psOne->bPublic;
}


//////////////////////////////////////////////////////////////////////////
//
// True if pcBase is reached from pcClass through public bases only, by one
// path at least (a virtual base has many).  [class.access.base] 4
//
//////////////////////////////////////////////////////////////////////////
bool CCPPClasses::IsPublicBase(CCPPClass* pcClass, CCPPClass* pcBase)
{
	CArrayCPPBasePaths*	pasPaths;
	size				i;

	pasPaths = pcClass->BasePaths();
	for (i = 0; i < pasPaths->NumElements(); i++)
	{
		if ((pasPaths->Get(i)->pcClass == pcBase) && pasPaths->Get(i)->bPublic)
		{
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
size CCPPClasses::CountBases(CCPPClass* pcClass, CCPPClass* pcBase)
{
	CArrayCPPClasses	apcVirtual;
	size				uiCount;
	size				i;

	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
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
void CCPPClasses::CollectVirtualBases(CCPPClass* pcClass, CArrayCPPClasses* papcVirtual)
{
	CArrayCPPBasePaths*	pasPaths;
	SCPPBasePath*		psPath;
	size				i;

	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
	pasPaths = pcClass->BasePaths();
	for (i = 1; i < pasPaths->NumElements(); i++)
	{
		psPath = pasPaths->Get(i);
		if (psPath->bVirtualEdge && (papcVirtual->Find(psPath->pcClass) == ARRAY_ELEMENT_NOT_FOUND))
		{
			papcVirtual->Add(psPath->pcClass);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// How many pcBase subobjects are at a fixed place in pcClass's own part:
// itself, or those of its bases that aren't virtual.
//
//////////////////////////////////////////////////////////////////////////
size CCPPClasses::CountFixedBases(CCPPClass* pcClass, CCPPClass* pcBase)
{
	CArrayCPPBasePaths*	pasPaths;
	size				uiCount;
	size				i;

	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
	pasPaths = pcClass->BasePaths();
	uiCount = 0;
	for (i = 0; i < pasPaths->NumElements(); i++)
	{
		uiCount += ((pasPaths->Get(i)->pcClass == pcBase) && !pasPaths->Get(i)->pcVirtual) ? 1 : 0;
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
SCPPVirtualBase* CCPPClasses::VirtualBaseOf(CCPPClass* pcClass, CCPPClass* pcBase, int* piBelow)
{
	SCPPVirtualBase*	psVirtual;
	size				i;
	int					iOffset;

	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
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
bool CCPPClasses::CompleteOffset(CCPPClass* pcClass, CCPPClass* pcBase, int* piOffset)
{
	SCPPVirtualBase*	psVirtual;
	int					iBelow;

	mpcUnit->GetStatistics()->Count(CPPCC_BaseWalks);
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
