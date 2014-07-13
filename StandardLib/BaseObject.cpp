/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

This file is part of The Codaphela Project: Codaphela StandardLib

Codaphela StandardLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela StandardLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela StandardLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#include "BaseLib/Log.h"
#include "ObjectSerialiser.h"
#include "ObjectDeserialiser.h"
#include "Objects.h"
#include "DistCalculator.h"
#include "BaseObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject::CBaseObject()
{
	mpcObjectsThisIn = NULL;
	miDistToRoot = UNATTACHED_DIST_TO_ROOT;
	miDistToStack = MIN_STACK_DIST_TO_STACK;
	moi = INVALID_O_INDEX;
	miFlags = OBJECT_FLAGS_DIRTY | OBJECT_FLAGS_CALLED_CONSTRUCTOR;
	miNumEmbedded = 0;
	miPreInits = 0;
	miPostInits = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject::~CBaseObject()
{
	if (mpcUnknownsThisIn == NULL)
	{
		//This destructor code will only be called if the object was allocated on the stack.
		ValidateHasClass();
	}
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Allocate(CObjects* pcObjects)
{
	mpcObjectsThisIn = pcObjects;
	SetFlag(OBJECT_FLAGS_CALLED_ALLOCATE, TRUE);
	Class();
	SetDistToStack(UNKNOWN_DIST_TO_STACK);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Class(void)
{
	SetFlag(OBJECT_FLAGS_CALLED_CLASS, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreInit(void)
{
	CBaseObject*	pcContainer;

#ifdef DEBUG
	ValidateHasClass();
#endif

	pcContainer = GetEmbeddingContainer();
	pcContainer->ContainerPreInit();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PostInit(void)
{
	CBaseObject*	pcContainer;

	pcContainer = GetEmbeddingContainer();
	pcContainer->ContainerPostInit();
}

//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ContainerPreInit(void)
{
	ValidateNotEmbedded(__METHOD__);

	miPreInits++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ContainerPostInit(void)
{
	ValidateNotEmbedded(__METHOD__);

	miPostInits++;
	if (miPreInits == miPostInits)
	{
		Initialised();
	}		
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Initialised(void)
{
	SetFlag(OBJECT_FLAGS_CALLED_INIT, TRUE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Kill(void)
{
	BOOL	bHeapFromChanged;

	bHeapFromChanged = HasHeapFroms();
	Kill(bHeapFromChanged);

#ifdef DEBUG
	if (mpcObjectsThisIn)
	{
		mpcObjectsThisIn->ValidateObjectsConsistency();
	}
#endif
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Kill(BOOL bHeapFromChanged)
{
	ValidateNotEmbedded(__METHOD__);

	CDistCalculator			cDistCalculator;
	CArrayBlockObjectPtr*	papcKilled;

	//This method is for the user to forcibly kill an object.
	//It is not called internally.

	if (IsAllocatedInObjects())
	{
		RemoveAllStackFroms();
		RemoveAllHeapFroms();

		cDistCalculator.Init();
		papcKilled = cDistCalculator.Calculate(this, bHeapFromChanged);

		mpcObjectsThisIn->Remove(papcKilled);
		cDistCalculator.Kill();
	}
	else
	{
		RemoveAllPointerTos();

		RemoveAllStackFroms();
		RemoveAllHeapFroms();

		KillDontFree();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillDontFree(void)
{
	LOG_OBJECT_DESTRUCTION(this);

	KillData();
	KillInternalData();

	miFlags |= OBJECT_FLAGS_KILLED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillInternalData(void)
{
	CEmbeddedObject::KillInternalData();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillIdentifiers(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::CollectValidDistStartingObjectsAndSetClearedToRoot(CBaseObject* pcTo, CDistCalculatorParameters* pcParameters)
{
	//It is assumed at this point that all the tos and froms have been updated.
	//This method will only be called on an object that can find Root.

	ValidateNotEmbedded(__METHOD__);

	int								i;
	CArrayTemplateEmbeddedBaseObjectPtr		apcFroms;
	CBaseObject*					pcFrom;
	CBaseObject*					pcContainer;

	if (IsRoot())
	{
		return;
	}

	if (!IsDistToRootValid())
	{
		pcParameters->AddTouched(this);
		SetDistToRoot(CLEARED_DIST_TO_ROOT);
		SetFlag(OBJECT_FLAGS_CLEARED_DIST_TO_ROOT, TRUE);

		apcFroms.Init();
		GetHeapFroms(&apcFroms);

		if (apcFroms.NumElements() > 0)
		{
			for (i = 0; i < apcFroms.NumElements(); i ++)
			{
				pcFrom = *apcFroms.Get(i);
				pcContainer = pcFrom->GetEmbeddingContainer();
				if (!(pcContainer->miFlags & OBJECT_FLAGS_CLEARED_DIST_TO_ROOT))
				{
					pcContainer->CollectValidDistStartingObjectsAndSetClearedToRoot(this, pcParameters);
				}
			}
		}

		apcFroms.Kill();
	}
	else
	{
		if (pcTo != NULL)
		{
			pcParameters->AddExpectedDist(pcTo, miDistToRoot+1);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::CollectAndClearInvalidDistToRootObjects(CDistCalculatorParameters* pcParameters)
{
	ValidateNotEmbedded(__METHOD__);

	if (IsRoot())
	{
		return;
	}

	if (!(miFlags & OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED))
	{
		if (!CanFindRootThroughValidPath())
		{
			pcParameters->AddTouched(this);
			SetFlag(OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED, TRUE);
			SetDistToRoot(CLEARED_DIST_TO_ROOT);

			CollectAndClearPointerTosInvalidDistToRootObjects(pcParameters);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::CollectDetachedAndSetDistToStackZero(CDistCalculatorParameters* pcParameters)
{
	ValidateNotEmbedded(__METHOD__);

	int	iNumWithStackPointers;

	iNumWithStackPointers = 0;

	if (miDistToRoot != UNATTACHED_DIST_TO_ROOT)
	{
		return 0;
	}

	if (miFlags & OBJECT_FLAGS_DIST_FROM_WALKED)
	{
		return 0;
	}

	pcParameters->AddDetachedFromRoot(this);

	if (HasStackPointers())
	{
		if (IsAllocatedInObjects())
		{
			SetDistToStack(MIN_HEAP_DIST_TO_STACK);
		}
		iNumWithStackPointers++;
	}

	if (!(miFlags & OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED))
	{
		SetFlag(OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED, TRUE);
		pcParameters->AddTouched(this);
	}

	SetFlag(OBJECT_FLAGS_DIST_FROM_WALKED, TRUE);

	iNumWithStackPointers += CollectDetachedFroms(pcParameters);
	return iNumWithStackPointers;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::CollectDetachedFroms(CDistCalculatorParameters* pcParameters)
{
	int					i;
	int					iNumFroms;
	CEmbeddedObject*	pcEmbedded;
	CBaseObject*		pcBaseObject;
	int					iNumWithStackPointers;

	iNumWithStackPointers = 0;
	iNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < iNumFroms; i++)
	{
		pcEmbedded = *mapHeapFroms.Get(i);
		pcBaseObject = pcEmbedded->GetEmbeddingContainer();

		iNumWithStackPointers += pcBaseObject->CollectDetachedAndSetDistToStackZero(pcParameters);
	}

	return iNumWithStackPointers;

}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsDistToRootValid(void)
{
	int				i;
	CBaseObject*	pcBaseObject;
	int				iExpectedDistToRoot;

	if (miDistToRoot < ROOT_DIST_TO_ROOT)
	{
		return FALSE;
	}

	iExpectedDistToRoot = miDistToRoot - 1;
	for (i = 0; i < mapHeapFroms.NumElements(); i++)
	{
		pcBaseObject = *mapHeapFroms.Get(i);
		if (pcBaseObject->GetDistToRoot() == iExpectedDistToRoot)
		{
			return TRUE;
		}
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::TryKill(BOOL bKillIfNoRoot, BOOL bHeapFromChanged)
{
	ValidateNotEmbedded(__METHOD__);

	BOOL					bHasStackPointers;
	BOOL					bHasHeapPointers;
	BOOL					bMustKill;
	CDistCalculator			cDistCalculator;
	CArrayBlockObjectPtr*	papcKilled;
	
	if (IsRoot())
	{
		return;
	}

	if (bKillIfNoRoot)
	{
		cDistCalculator.Init();
		papcKilled = cDistCalculator.Calculate(this, bHeapFromChanged);

		mpcObjectsThisIn->Remove(papcKilled);
		cDistCalculator.Kill();
	}
	else
	{
		bHasHeapPointers = HasHeapFroms();
		bHasStackPointers = HasStackPointers();

		//If we removed a stack pointer and have no more stack pointers and have no heap pointers (regardless of whether or not they can find the root)
		bMustKill = !bHasHeapPointers && !bHasStackPointers && (miDistToStack != 0);
		if (bMustKill)
		{
			cDistCalculator.Init();
			papcKilled = cDistCalculator.Calculate(this, bHeapFromChanged);
			mpcObjectsThisIn->Remove(papcKilled);
			cDistCalculator.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetFlag(int iFlag, int iFlagValue)
{
	::SetFlag(&miFlags, iFlag, iFlagValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::GetFlags(void)
{
	return miFlags;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::CanFindRoot(void)
{
	ValidateNotEmbedded(__METHOD__);

	CEmbeddedObject*				pcPointedFrom;
	BOOL							bResult;

	if (IsRoot())
	{
		return TRUE;
	}

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, TRUE);

	pcPointedFrom = GetClosestFromForCanFindRoot();
	if (pcPointedFrom == NULL)
	{
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return FALSE;
	}

	bResult = pcPointedFrom->GetEmbeddingContainer()->CanFindRoot();

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::CanFindRootThroughValidPath(void)
{
	ValidateNotEmbedded(__METHOD__);

	CEmbeddedObject*	pcPointedFrom;
	BOOL				bResult;
	int					iFromDistToRoot;

	if (IsRoot())
	{
		return TRUE;
	}
	if (miDistToRoot < ROOT_DIST_TO_ROOT)
	{
		return FALSE;
	}

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, TRUE);

	pcPointedFrom = GetClosestFromForCanFindRoot();
	if (pcPointedFrom == NULL)
	{
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return FALSE;
	}

	iFromDistToRoot = pcPointedFrom->GetDistToRoot();
	if (iFromDistToRoot >= miDistToRoot)
	{
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
		return FALSE;
	}

	bResult = pcPointedFrom->GetEmbeddingContainer()->CanFindRootThroughValidPath();

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, FALSE);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetExpectedDistToRoot(int iExpectedDistToRoot)
{
	ValidateNotEmbedded(__METHOD__);

	int	iBestDistToRoot;

	iBestDistToRoot = CalculateDistToRootFromPointedFroms(iExpectedDistToRoot);
	if (SetDistToRoot(iBestDistToRoot))
	{
		SetPointerTosExpectedDistToRoot(iBestDistToRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetCalculatedDistToRoot(void)
{
	ValidateNotEmbedded(__METHOD__);
	ValidateInitialised(__METHOD__);

	int	iBestDistToRoot;

	iBestDistToRoot = CalculateDistToRootFromPointedFroms();
	if (SetDistToRoot(iBestDistToRoot))
	{
		SetPointerTosExpectedDistToRoot(iBestDistToRoot);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::SetDistToRoot(int iDistToRoot)
{
	if (miDistToRoot != iDistToRoot)
	{
		miDistToRoot = iDistToRoot;
		return TRUE;
	}
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetDistToStack(int iDistToStack)
{
	miDistToStack = iDistToStack;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::CalculateDistToRootFromPointedFroms(void)
{
	int iDistToRoot;

	iDistToRoot = CalculateDistToRootFromPointedFroms(MAX_DIST_TO_ROOT);
	if (iDistToRoot != MAX_DIST_TO_ROOT)
	{
		return iDistToRoot;
	}
	else
	{
		return UNATTACHED_DIST_TO_ROOT;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::CalculateDistToRootFromPointedFroms(int iDistToRoot)
{
	int				iNumFroms;
	int				i;
	CBaseObject*	pcFrom;
	int				iBestDistToRoot;

	iBestDistToRoot = iDistToRoot;
	iNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < iNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		if (pcFrom)
		{
			if (pcFrom->miDistToRoot < iBestDistToRoot)
			{
				if (pcFrom->GetEmbeddingContainer()->CanFindRoot())
				{
					if (pcFrom->miDistToRoot >= ROOT_DIST_TO_ROOT)
					{
						iBestDistToRoot = pcFrom->miDistToRoot+1;
					}
				}
			}
		}
	}
	return iBestDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::UpdateAttachedTosDistToRoot(CDistCalculatorParameters* pcParameters)
{
	ValidateNotEmbedded(__METHOD__);

	CEmbeddedObject*	pcClosestToRoot;
	int					iClosestToRoot;

	pcClosestToRoot = GetClosestFromToRoot();
	iClosestToRoot = pcClosestToRoot->GetDistToRoot()+1;

	pcParameters->AddTouched(this);
	SetDistToRoot(iClosestToRoot);
	SetFlag(OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT, TRUE);

	UpdateAttachedEmbeddedObjectPointerTosDistToRoot(pcParameters, iClosestToRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::AddExpectedDistToRoot(CEmbeddedObject* pcPointedTo, int iExpectedDist, CDistCalculatorParameters* pcParameters)
{
	CBaseObject*		pcPointedToContainer;

	if (pcPointedTo)
	{
		pcPointedToContainer = pcPointedTo->GetEmbeddingContainer();
		if (!pcPointedToContainer->IsUpdateAttachedPointerTosDistToRoot())
		{
			pcPointedToContainer->SetDistToRoot(CLEARED_DIST_TO_ROOT);
			pcParameters->AddExpectedDist(pcPointedToContainer, iExpectedDist);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearDistTouchedFlags(void)
{
	ValidateNotEmbedded(__METHOD__);

	SetFlag(OBJECT_FLAGS_CLEARED_DIST_TO_ROOT | OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT | OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED | OBJECT_FLAGS_DIST_FROM_WALKED, FALSE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::HasDistTouchedFlag(void)
{
	return miFlags & (OBJECT_FLAGS_CLEARED_DIST_TO_ROOT | OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::RemoveToFrom(CEmbeddedObject* pcPointedTo)
{
	CBaseObject*	pcBaseObject;

	if (pcPointedTo)
	{
		if (pcPointedTo->IsBaseObject())
		{
			pcBaseObject = (CBaseObject*)pcPointedTo;
			pcBaseObject->PrivateRemoveHeapFrom(this);  //If the object pointed to us is also being killed then we needed remove our from from it.
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::SerialisedSize(void)
{
	CObjectSerialiser	cSerialiser;
	int					iLength;

	cSerialiser.Init(NULL, this);
	cSerialiser.Save();
	iLength = cSerialiser.GetLength();
	cSerialiser.Kill();
	return iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsRoot(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsNamed(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CBaseObject::GetName(void)
{
	return "";
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsSubRoot(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsInvalidated(void)
{
	return miFlags & OBJECT_FLAGS_INVALIDATED;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsDirty(void)
{
	return miFlags & OBJECT_FLAGS_DIRTY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::TestedForRoot(void)
{
	return miFlags & OBJECT_FLAGS_TESTED_FOR_ROOT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::TestedForSanity(void)
{
	return miFlags & OBJECT_FLAGS_TESTED_FOR_SANITY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetDirty(void)
{
	miFlags |= OBJECT_FLAGS_DIRTY;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::GetDistToRoot(void)
{
	return miDistToRoot;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::GetDistToStack(void)
{
	return miDistToStack;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetObjectID(OIndex oi)
{
	moi = oi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearIndex(void)
{
	SetObjectID(INVALID_O_INDEX);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsUnknown(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsHollow(void)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CBaseObject::TestGetPointerTo(int iToIndex)
{
	CEmbeddedObject**			ppTo;
	CEmbeddedObject*			pTo;
	CArrayTemplateEmbeddedObjectPtr		apcTos;

	apcTos.Init(32);
	GetPointerTos(&apcTos);
	ppTo = apcTos.SafeGet(iToIndex);
	if (ppTo)
	{
		pTo = *ppTo;
	}
	else
	{
		pTo = NULL;
	}
	apcTos.Kill();
	return pTo;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::ContainsPointerTo(CEmbeddedObject* pcEmbedded)
{
	return FALSE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CBaseObject::GetOI(void)
{
	return moi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetName(char* szName)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsBaseObject(void)
{
	return TRUE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short int CBaseObject::GetNumEmbedded(void)
{
	SetFlagNumEmbedded(1);
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
unsigned short int CBaseObject::GetNumEmbeddedFromFlags(void)
{
	return miNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetFlagNumEmbedded(int iNumEmbedded)
{
	miNumEmbedded = (unsigned short int)iNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearFlagNumEmbedded(void)
{
	miNumEmbedded = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
int CBaseObject::TestGetNumEmbeddedFromFlags(void)
{
	return GetNumEmbeddedFromFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjects* CBaseObject::GetObjects(void)
{
	CBaseObject*	pcBaseObject;

	pcBaseObject = GetEmbeddingContainer();
	return pcBaseObject->mpcObjectsThisIn;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CStackPointers* CBaseObject::GetStackPointers(void)
{
	return &gcStackPointers;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsMarkedUnreachable(void)
{
	return miFlags & OBJECT_FLAGS_UNREACHABLE;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsUpdateAttachedPointerTosDistToRoot(void)
{
	return miFlags & OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::HasClass(void)
{
	return miFlags & OBJECT_FLAGS_CALLED_CLASS;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
BOOL CBaseObject::IsInitialised(void)
{
	return miFlags & OBJECT_FLAGS_CALLED_INIT;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ReplaceOneWithX(char* szDest, char* szMask)
{
	int i;

	for (i = 0; ; i++)
	{
		if ((szMask[i] == '\0') || (szDest == '\0'))
		{
			return;
		}

		if (szMask[i] == '1')
		{
			szDest[i] ='X';
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpFroms(void)
{
	CChars				sz;
	int					i;
	int					iNumEmbedded;
	CEmbeddedObject*	pcEmbedded;
	int					j;
	int					iNumHeapFroms;
	CBaseObject*		pcFromObject;
	int					iLength;
	CChars				szLine;

	sz.Init();

	sz.Append("-- ");
	PrintObject(&sz);
	sz.Append(" --\n");
	iLength = sz.Length()-1;

	szLine.Init('-', iLength);
	szLine.AppendNewLine();

	sz.Insert(0, &szLine);
	sz.Append("Total Heap Froms [");
	sz.Append(NumHeapFroms());
	sz.Append("], ");

	sz.Append("Stack Froms [");
	sz.Append(NumStackFroms());
	sz.Append("]\n");

	iNumEmbedded = GetNumEmbedded();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		iNumHeapFroms = pcEmbedded->CEmbeddedObject::NumHeapFroms();
		sz.Append("Embedded ");
		sz.Append(i);
		sz.Append(" Heap Froms [");
		sz.Append(iNumHeapFroms);
		sz.Append("], ");
		sz.Append("Stack Froms [");
		sz.Append(pcEmbedded->CEmbeddedObject::NumStackFroms());
		sz.Append("]\n");

		for (j = 0; j < iNumHeapFroms; j++)
		{
			pcFromObject = pcEmbedded->GetHeapFrom(j);
			sz.Append(" ");
			pcFromObject->PrintObject(&sz);
			sz.AppendNewLine();
		}
	}

	sz.Append(&szLine);
	szLine.Kill();

	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpPointerTos(void)
{
	CChars						sz;
	int							i;
	int							iNumEmbedded;
	CEmbeddedObject*			pcEmbedded;
	int							j;
	int							iNumTos;
	CEmbeddedObject**			ppcToObject;
	int							iLength;
	CChars						szLine;
	CArrayTemplateEmbeddedObjectPtr		acTos;
	CBaseObject*				pcToObject;
	int							iTotalTos;

	sz.Init();

	sz.Append("-- ");
	PrintObject(&sz);
	sz.Append(" --\n");
	iLength = sz.Length()-1;

	szLine.Init('-', iLength);
	szLine.AppendNewLine();

	iTotalTos = 0;
	iNumEmbedded = GetNumEmbedded();
	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		iTotalTos += pcEmbedded->NumPointerTos();
	}

	sz.Insert(0, &szLine);
	sz.Append("Total Tos [");
	sz.Append(iTotalTos);
	sz.Append("]\n");

	for (i = 0; i < iNumEmbedded; i++)
	{
		pcEmbedded = GetEmbeddedObject(i);
		acTos.Init(1);
		pcEmbedded->BaseGetPointerTos(&acTos);
		iNumTos = acTos.NumElements();
		sz.Append("Embedded ");
		sz.Append(i);
		sz.Append(" Tos [");
		sz.Append(iNumTos);
		sz.Append("]\n");

		for (j = 0; j < iNumTos; j++)
		{
			ppcToObject = acTos.Get(j);  //A pointed to never comes back NULL.
			sz.Append(" ");
				
			if ((*ppcToObject)->IsBaseObject())
			{
				pcToObject = (CBaseObject*)*ppcToObject;
				pcToObject->PrintObject(&sz);
			}
			else
			{
				sz.Append("HollwEmbeddedObject");
			}
			sz.AppendNewLine();
		}

		acTos.Kill();
	}

	sz.Append(&szLine);
	szLine.Kill();

	sz.Dump();
	sz.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Dump(void)
{
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateFlagNotSet(int iFlag, char* szFlag)
{
	CChars	sz;

	if (miFlags & iFlag)
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have flag [", szFlag,"] set.", NULL);
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateFlagSet(int iFlag, char* szFlag)
{
	CChars	sz;

	if (!(miFlags & iFlag))
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should have flag [", szFlag,"] set.", NULL);
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateCanFindRoot(void)
{
	ValidateNotEmbedded(__METHOD__);

	CChars			sz;
	BOOL			bCanFindRoot;

	if (miDistToRoot > ROOT_DIST_TO_ROOT)
	{
		bCanFindRoot = CanFindRoot();

		if (!bCanFindRoot)
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} has a positive dist to root and should be able to find the Root object.", NULL);
			sz.Kill();
		}
	}
	else if (miDistToRoot == ROOT_DIST_TO_ROOT)
	{
		if (!IsRoot())
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} has a dist to root [0] but is not the Root object.", NULL);
			sz.Kill();
		}
	}
	else if (miDistToRoot == UNATTACHED_DIST_TO_ROOT)
	{
		bCanFindRoot = CanFindRoot();

		if (bCanFindRoot)
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} has an [UNATTACHED_DIST_TO_ROOT] dist to root should not be able to find the Root object.", NULL);
			sz.Kill();
		}
	}
	else if (miDistToRoot == CLEARED_DIST_TO_ROOT)
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have dist to root [CLEARED_DIST_TO_ROOT].", NULL);
		sz.Kill();
	}
	else
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have dist to root [", IntToString(miDistToRoot), "].", NULL);
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateContainerFlag(void)
{
	CChars	sz;
	int		iEmbeddedFlags;
	int		iThisFlags;
	int		iIgnoredFlags;
	char*	szEmbeddedFlags;
	char*	szFlags;
	char*	szIgnoredFlags;

	if (IsEmbedded())
	{
		iIgnoredFlags = OBJECT_FLAGS_CALLED_CLASS;
		iEmbeddedFlags = mpcEmbedded->miFlags & ~iIgnoredFlags;
		iThisFlags = miFlags & ~iIgnoredFlags;
		if ((iEmbeddedFlags) != (iThisFlags))
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			szFlags = ShortToFlags(miFlags);
			szEmbeddedFlags = ShortToFlags(mpcEmbedded->miFlags);
			szIgnoredFlags = ShortToFlags(iIgnoredFlags);
			ReplaceOneWithX(szFlags, szIgnoredFlags);
			ReplaceOneWithX(szEmbeddedFlags, szIgnoredFlags);

			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should have the same flags [", szFlags ,"] as it's embedding container's flags [", szEmbeddedFlags, "].", NULL);
			sz.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateFlags(void)
{
	ValidateFlagNotSet(OBJECT_FLAGS_TESTED_FOR_ROOT, "OBJECT_FLAGS_TESTED_FOR_ROOT");
	ValidateFlagNotSet(OBJECT_FLAGS_KILLED, "OBJECT_FLAGS_KILLED");
	ValidateFlagNotSet(OBJECT_FLAGS_DUMPED, "OBJECT_FLAGS_DUMPED");
	ValidateFlagNotSet(OBJECT_FLAGS_UNREACHABLE, "OBJECT_FLAGS_UNREACHABLE");
	ValidateFlagNotSet(OBJECT_FLAGS_CLEARED_DIST_TO_ROOT, "OBJECT_FLAGS_CLEARED_DIST_TO_ROOT");
	ValidateFlagNotSet(OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT, "OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT");
	ValidateFlagNotSet(OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED, "OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED");
	ValidateFlagSet(OBJECT_FLAGS_CALLED_CONSTRUCTOR, "OBJECT_FLAGS_CALLED_CONSTRUCTOR");
	ValidateFlagSet(OBJECT_FLAGS_CALLED_ALLOCATE, "OBJECT_FLAGS_CALLED_ALLOCATE");

	ValidateContainerFlag();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateAllocation(void)
{
	BOOL	bDistToStackZero;
	BOOL	bAllocateCalled;
	BOOL	bInObjects;
	BOOL	bAllSame;
	CChars	sz;

	bDistToStackZero = GetDistToStack() == 0;
	bAllocateCalled = miFlags & OBJECT_FLAGS_CALLED_ALLOCATE;
	bAllocateCalled = FixBool(bAllocateCalled);
	bInObjects = IsAllocatedInObjects();
	bAllSame = !bDistToStackZero == bAllocateCalled == bInObjects;

	if (!bAllSame)
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have a dist to stack of [", IntToString(GetDistToStack()), "] and flag OBJECT_FLAGS_CALLED_ALLOCATE [", IntToString(bAllocateCalled), "] and be allocated in Objects [0x", PointerToString(GetObjects()), "].", NULL);
		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateDistToRoot(void)
{
	CChars			sz;
	CBaseObject*	pcContainer;

	if (!((miDistToRoot >= ROOT_DIST_TO_ROOT) || (miDistToRoot == UNATTACHED_DIST_TO_ROOT)))
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have a dist to root of [", IntToString(miDistToRoot), "].", NULL);
		sz.Kill();
	}

	if (IsEmbedded())
	{
		pcContainer = GetEmbeddingContainer();
		if (pcContainer->GetDistToRoot() != GetDistToRoot())
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should have a dist to root [", IntToString(miDistToRoot), "] the same as it's embedding object [", IntToString(pcContainer->GetDistToRoot()),"].", NULL);
			sz.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateIndex(void)
{
	CChars			sz;

	if (IsEmbedded())
	{
		if (moi != INVALID_O_INDEX)
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should have an Index [", IndexToString(moi), "] of INVALID_O_INDEX [", IndexToString(INVALID_O_INDEX),"].", NULL);
			sz.Kill();
		}
	}
	else
	{
		if (moi == INVALID_O_INDEX)
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have an Index of INVALID_O_INDEX [", IndexToString(INVALID_O_INDEX),"].", NULL);
			sz.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateObjectsThisIn(void)
{
	CChars			sz;

	if (IsEmbedded())
	{
		if (mpcObjectsThisIn != NULL)
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have ObjectsThisIn [", PointerToString(mpcObjectsThisIn), "] set.", NULL);
			sz.Kill();
		}
	}
	else
	{
		if (mpcObjectsThisIn == NULL)
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should have ObjectsThisIn [NULL] set.", NULL);
			sz.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateBaseObjectDetail(void)
{
	ValidateFlags();
	ValidateAllocation();
	ValidateDistToRoot();
	ValidateIndex();
	ValidateObjectsThisIn();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateEmbeddedConsistency(void)
{
	ValidateBaseObjectDetail();
	ValidateFroms();
	ValidatePointerTos();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateObjectIdentifiers(void)
{
	CChars			szThis;
	CChars			szOther;
	CChars			szContainer;
	CBaseObject*	pcContainer;
	char*			szName;
	CBaseObject*	pcThis;

	if (IsNamed())
	{
		if (IsEmbedded())
		{
			pcContainer = GetEmbeddingContainer();

			szContainer.Init();
			pcContainer->PrintObject(&szContainer, FALSE);

			szThis.Init();
			PrintObject(&szThis, IsEmbedded());

			gcLogger.Error2(__METHOD__, " Object {", szThis.Text(), "} should have a name as it's embedded in object {", szContainer.Text(), "}.", NULL);

			szThis.Kill();
			szContainer.Kill();
		}
		else
		{
			szName = GetName();
			if (!StrEmpty(szName))
			{
				pcThis = mpcObjectsThisIn->GetFromMemory(szName);
				if (pcThis != this)
				{
					szThis.Init();
					PrintObject(&szThis, IsEmbedded());
					szOther.Init();
					if (pcThis != NULL)
					{
						pcThis->PrintObject(&szOther, IsEmbedded());
					}
					else
					{
						szOther.Append("NULL");
					}

					gcLogger.Error2(__METHOD__, " 'this' Object {", szThis.Text(), "} does not match the Named Object {", szOther.Text(), "} in Objects.", NULL);

					szOther.Kill();
					szThis.Kill();
				}
			}
		}
	}

	if (!IsEmbedded())
	{
		pcThis = mpcObjectsThisIn->GetFromMemory(GetOI());
		if (pcThis != this)
		{
			szThis.Init();
			PrintObject(&szThis, IsEmbedded());

			gcLogger.Error2(__METHOD__, " Object {", szThis.Text(), "} does not match the Object in Objects.", NULL);

			szThis.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateHasClass(void)
{
	CChars sz;

	if (!HasClass())
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());

		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} does not have Class initialised.", NULL);

		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateInitCalled(void)
{
	CChars	szObject;

	if (miPreInits != miPostInits)
	{
		szObject.Init();
		PrintObject(&szObject, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", szObject.Text(), "} has pre-inits [", IntToString(miPreInits), "] not equal to post inits [", IntToString(miPostInits), "].", NULL);
		szObject.Kill();
	}
	else if (miPreInits == 0)
	{
		szObject.Init();
		PrintObject(&szObject, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", szObject.Text(), "} has a pre-init of zero.", NULL);
		szObject.Kill();
	}
}

