/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2025 Andrew Paterson

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
#include "BaseLib/Logger.h"
#include "BaseLib/Logger.h"
#include "BaseLib/Float2.h"
#include "BaseLib/Float3.h"
#include "BaseLib/Float4.h"
#include "BaseLib/Float4x4.h"
#include "ObjectWriter.h"
#include "ObjectReader.h"
#include "Objects.h"
#include "DistCalculator.h"
#include "Classes.h"
#include "Primitive.h"
#include "BaseObject.h"


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject::CBaseObject()
{
	// The Constructor code is only called if an Object is allocated on the stack.
	mpcObjectsThisIn = NULL;
	mpcClass = NULL;
	miDistToRoot = UNATTACHED_DIST_TO_ROOT;
	miDistToStack = MIN_STACK_DIST_TO_STACK;
	moi = INVALID_O_INDEX;
	mon._Init();
	muiFlags = OBJECT_FLAGS_CALLED_CONSTRUCTOR;
	muiNumEmbedded = 0;
	muiPreInits = 0;
	muiPostInits = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CBaseObject::~CBaseObject()
{
	//This Destructor code will only be called if the object was allocated on the stack.
	if (mpcUnknownsThisIn == NULL)
	{
		if (!HasClass())
		{
			ValidateHasClassFlag(__METHOD__);
		}
	}
	FreePointers();

	ValidateInitCalled();
	ValidateKillCalled();

	muiFlags = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Allocate(CObjects* pcObjects)
{
	//Allocate is only called if an object is allocated in Objects.
	mpcObjectsThisIn = pcObjects;
	SetFlag(OBJECT_FLAGS_CALLED_ALLOCATE, true);
	PreClass();
	SetDistToStack(UNKNOWN_DIST_TO_STACK);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreClass(void)
{
	CClasses*		pcClasses;

	if (muiFlags & OBJECT_FLAGS_FREED)
	{
		LogCantInitFreedObject(__METHOD__);
	}

	if (!HasClass())
	{
		SetFlag(OBJECT_FLAGS_CALLED_CLASS, true);

		pcClasses = GetClasses();
		CompleteClass(pcClasses);
		EmbedFields();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClass* CBaseObject::CompleteClass(CClasses* pcClasses)
{
	CClass*		pcClass;
	const char* szClassName;

	szClassName = ClassName();
	pcClass = pcClasses->Get(szClassName);
	if (!pcClass)
	{
		pcClass = pcClasses->Add(szClassName, ClassSize());
		if (!pcClass)
		{
			return NULL;
		}
	}
	SetClass(pcClass);
	if (!pcClass->IsComplete())
	{
		Class();
		pcClass->Complete();
	}
	return pcClass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetClass(CClass* pcClass)
{
	if (!mpcClass)
	{
		mpcClass = pcClass;
	}
	else
	{
		gcLogger.Error2(__METHOD__, " Class is already set.", NULL);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::PreInit(void)
{
	CBaseObject*	pcContainer;

	PreClass();

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

	muiPreInits++;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ContainerPostInit(void)
{
	CChars	sz;

	ValidateNotEmbedded(__METHOD__);

	if (IsInitialised())
	{
		sz.Init();
		gcLogger.Error2(__METHOD__, " Init has already been called on object[", GetIdentifier(&sz), "].", NULL);
		sz.Kill();
		return;
	}

	muiPostInits++;
	if (muiPreInits == muiPostInits)
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
	SetFlag(OBJECT_FLAGS_CALLED_KILL, false);
	SetFlag(OBJECT_FLAGS_CALLED_INIT, true);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Kill(void)
{
	//This method is for the user to forcibly kill an object.
	//It is not called internally.
	//It is the equivalent of setting the last pointer to the object to NULL.

	bool		bHeapFromChanged;
	CObjects*	pcObjectsThisIn;

	if (IsKilled())
	{
		return;
	}

	pcObjectsThisIn = GetObjectsThisIn();

	SetFlag(OBJECT_FLAGS_CALLED_KILL, true);
	SetFlag(OBJECT_FLAGS_CALLED_INIT, false);

	bHeapFromChanged = HasHeapFroms();
	KillInternal(bHeapFromChanged);

#ifdef _DEBUG
	if (pcObjectsThisIn)
	{
		pcObjectsThisIn->ValidateObjectsConsistency();
	}
#endif // _DEBUG
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::KillInternal(bool bHeapFromChanged)
{
	ValidateNotEmbedded(__METHOD__);

	CDistCalculator			cDistCalculator;
	CArrayBlockObjectPtr*	papcKilled;

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
		RemoveAllPointerTos();  //Handles embedded objects.

		RemoveAllStackFroms();  //Handles embedded objects (in Object.RemoveAllStackFroms()).
		RemoveAllHeapFroms();   //Handles embedded objects (in Object.RemoveAllHeapFroms()).

		FreeInternal(false);    //Handles embedded objects (in Object.FreeInternal()).
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsKilled(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_CALLED_KILL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::Flush(void)
{
	bool	bResult;
	bool	bCanFindRoot;
	bool	bDirty;

	ValidateNotEmbedded(__METHOD__);

	bCanFindRoot = GetDistToRoot() != UNATTACHED_DIST_TO_ROOT;
	bDirty = IsDirty();

	if (bDirty)
	{
		if (bCanFindRoot)
		{
			bResult = GetObjectsThisIn()->ForceSave(this);
			SetDirty(false);
			return bResult;
		}
		else
		{
			return false;
		}
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::FreeInternal(bool bAllocatedInObjects)
{
	LOG_OBJECT_DESTRUCTION(this);
	if (mpcObjectsThisIn)
	{
		mpcObjectsThisIn->mDestructionCallback(this);
	}

	//Frees user data.
	Free();

	if (bAllocatedInObjects)
	{
		SetFlag(OBJECT_FLAGS_FREED, true);
	}

	//Note that FreeIdentifiers cannot be called here because the object is not yet RemovedFromMemory.
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::FreePointers(void)
{
	CEmbeddedObject::FreePointers();
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

	size									i;
	CArrayTemplateEmbeddedBaseObjectPtr		apcFroms;
	CBaseObject*							pcFrom;
	CBaseObject*							pcContainer;
	size									uiNumFroms;

	if (IsRoot())
	{
		return;
	}

	if (!IsDistToRootValid())
	{
		pcParameters->AddTouched(this);
		SetDistToRoot(CLEARED_DIST_TO_ROOT);
		SetFlag(OBJECT_FLAGS_CLEARED_DIST_TO_ROOT, true);

		apcFroms.Init();
		GetHeapFroms(&apcFroms);

		uiNumFroms = apcFroms.NumElements();
		if (uiNumFroms != 0)
		{
			for (i = 0; i < uiNumFroms; i ++)
			{
				pcFrom = *apcFroms.Get(i);
				pcContainer = pcFrom->GetEmbeddingContainer();
				if (!(pcContainer->muiFlags & OBJECT_FLAGS_CLEARED_DIST_TO_ROOT))
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
			pcParameters->AddExpectedDist(pcTo, miDistToRoot + 1);
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

	if (!(muiFlags & OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED))
	{
		if (!CanFindRootThroughValidPath())
		{
			pcParameters->AddTouched(this);
			SetFlag(OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED, true);
			SetDistToRoot(CLEARED_DIST_TO_ROOT);

			CollectAndClearPointerTosInvalidDistToRootObjects(pcParameters);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBaseObject::CollectDetachedAndSetDistToStackZero(CDistCalculatorParameters* pcParameters)
{
	ValidateNotEmbedded(__METHOD__);

	size	iNumWithStackPointers;

	iNumWithStackPointers = 0;

	if (miDistToRoot != UNATTACHED_DIST_TO_ROOT)
	{
		return 0;
	}

	if (muiFlags & OBJECT_FLAGS_DIST_FROM_WALKED)
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

	if (!(muiFlags & OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED))
	{
		SetFlag(OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED, true);
		pcParameters->AddTouched(this);
	}

	SetFlag(OBJECT_FLAGS_DIST_FROM_WALKED, true);

	iNumWithStackPointers += CollectDetachedFroms(pcParameters);
	return iNumWithStackPointers;
}



//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBaseObject::CollectDetachedFroms(CDistCalculatorParameters* pcParameters)
{
	size				i;
	size				uiNumFroms;
	CEmbeddedObject*	pcEmbedded;
	CBaseObject*		pcBaseObject;
	size				iNumWithStackPointers;

	iNumWithStackPointers = 0;
	uiNumFroms = mapHeapFroms.NumElements();
	for (i = 0; i < uiNumFroms; i++)
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
bool CBaseObject::IsDistToRootValid(void)
{
	size				i;
	CEmbeddedObject*	pcBaseObject;
	int					iExpectedDistToRoot;
	size				uiNumFroms;

	if (miDistToRoot < ROOT_DIST_TO_ROOT)
	{
		return false;
	}

	uiNumFroms = mapHeapFroms.NumElements();
	iExpectedDistToRoot = miDistToRoot - 1;
	for (i = 0; i < uiNumFroms; i++)
	{
		pcBaseObject = *mapHeapFroms.Get(i);
		if (pcBaseObject->GetDistToRoot() == iExpectedDistToRoot)
		{
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::TryFree(bool bKillIfNoRoot, bool bHeapFromChanged)
{
	ValidateNotEmbedded(__METHOD__);

	bool					bHasStackPointers;
	bool					bHasHeapPointers;
	bool					bMustKill;
	CDistCalculator			cDistCalculator;
	CArrayBlockObjectPtr*	papcKilled;
	CBaseObject*			pcKilled;
	bool					bOnlyThisKilled;
	
	if (IsRoot())
	{
		return;
	}

	if (bKillIfNoRoot)
	{
		cDistCalculator.Init();
		papcKilled = cDistCalculator.Calculate(this, bHeapFromChanged);

		if (!mpcObjectsThisIn)
		{
			if (papcKilled->NumElements() > 0)
			{
				bOnlyThisKilled = false;
				if (papcKilled->NumElements() == 1)
				{
					pcKilled = *papcKilled->Get(0);
					if (pcKilled == this)
					{
						bOnlyThisKilled = true;
					}
				}

				if (!bOnlyThisKilled)
				{
					gcLogger.Error2(__METHOD__, " Uh, probably if not in an objects then the only pointer to this object should be itself.  Maybe some stack kak?", NULL);
				}
			}
		}
		else
		{
			mpcObjectsThisIn->Remove(papcKilled);
		}
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
void CBaseObject::SetFlag(uint16 iFlag, uint16 iFlagValue)
{
	::SetFlagShort(&muiFlags, iFlag, iFlagValue);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CBaseObject::GetFlags(void)
{
	return muiFlags;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::CanFindRoot(void)
{
	ValidateNotEmbedded(__METHOD__);

	CEmbeddedObject*	pcPointedFrom;
	bool				bResult;

	if (IsRoot())
	{
		return true;
	}

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, true);

	pcPointedFrom = GetClosestFromForCanFindRoot();
	if (pcPointedFrom == NULL)
	{
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, false);
		return false;
	}

	bResult = pcPointedFrom->GetEmbeddingContainer()->CanFindRoot();

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, false);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::CanFindRootThroughValidPath(void)
{
	ValidateNotEmbedded(__METHOD__);

	CEmbeddedObject*	pcPointedFrom;
	bool				bResult;
	int					iFromDistToRoot;

	if (IsRoot())
	{
		return true;
	}
	if (miDistToRoot < ROOT_DIST_TO_ROOT)
	{
		return false;
	}

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, true);

	pcPointedFrom = GetClosestFromForCanFindRoot();
	if (pcPointedFrom == NULL)
	{
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, false);
		return false;
	}

	iFromDistToRoot = pcPointedFrom->GetDistToRoot();
	if (iFromDistToRoot >= miDistToRoot)
	{
		SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, false);
		return false;
	}

	bResult = pcPointedFrom->GetEmbeddingContainer()->CanFindRootThroughValidPath();

	SetFlag(OBJECT_FLAGS_TESTED_FOR_ROOT, false);
	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetExpectedDistToRoot(int iExpectedDistToRoot)
{
	ValidateNotEmbedded(__METHOD__);

	int		iBestDistToRoot;

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

	int		iBestDistToRoot;

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
bool CBaseObject::SetDistToRoot(int iDistToRoot)
{
	if (miDistToRoot != iDistToRoot)
	{
		miDistToRoot = iDistToRoot;
		return true;
	}
	return false;
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
	int		iDistToRoot;

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
	size				uiNumFroms;
	size				i;
	CEmbeddedObject*	pcFrom;
	int					iBestDistToRoot;

	iBestDistToRoot = iDistToRoot;
	uiNumFroms = CEmbeddedObject::NumHeapFroms();
	for (i = 0; i < uiNumFroms; i++)
	{
		pcFrom = CEmbeddedObject::GetHeapFrom(i);
		if (pcFrom)
		{
			iDistToRoot = pcFrom->GetDistToRoot();
			if (iDistToRoot < iBestDistToRoot)
			{
				if (pcFrom->GetEmbeddingContainer()->CanFindRoot())
				{
					if (iDistToRoot >= ROOT_DIST_TO_ROOT)
					{
						iBestDistToRoot = iDistToRoot  + 1;
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
	iClosestToRoot = pcClosestToRoot->GetDistToRoot() + 1;

	pcParameters->AddTouched(this);
	SetDistToRoot(iClosestToRoot);
	SetFlag(OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT, true);

	UpdateAttachedEmbeddedObjectPointerTosDistToRoot(pcParameters, iClosestToRoot);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::AddExpectedDistToRoot(CEmbeddedObject* pcPointedTo, int iExpectedDist, CDistCalculatorParameters* pcParameters)
{
	CBaseObject*	pcPointedToContainer;

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

	SetFlag(OBJECT_FLAGS_CLEARED_DIST_TO_ROOT | OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT | OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED | OBJECT_FLAGS_DIST_FROM_WALKED, false);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::HasDistTouchedFlag(void)
{
	return FixBool(muiFlags & (OBJECT_FLAGS_CLEARED_DIST_TO_ROOT | OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::RemoveToFrom(CEmbeddedObject* pcPointedTo)
{
	CBaseObject*	pcBaseObject;

	if (pcPointedTo)
	{
		if (pcPointedTo->IsBaseObject())
		{
			pcBaseObject = (CBaseObject*)pcPointedTo;
			pcBaseObject->PrivateRemoveHeapFrom(this);  //If the object pointed to us is also being killed then we needed remove our from from it.
		}
		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBaseObject::SerialisedSize(void)
{
	//This method should only be used for testing.  It should be a helper somewhere else.
	CObjectWriter	cWriter;
	size			iLength;
	CMemoryFile		cMemory;

	cMemory.Init();
	cMemory.Open(EFM_ReadWrite_Create);
	cWriter.Init(&cMemory, NULL);
	cWriter.Write(this);
	cWriter.WriteHeapFroms(this);
	iLength = cMemory.GetBufferSize();
	cMemory.Close();
	cWriter.Kill();
	cMemory.Kill();
	return iLength;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsRoot(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsNamed(void)
{
	return !mon.Empty();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CBaseObject::GetIdentifier(CChars* psz)
{
	char*	szName;

	psz->Append(ClassName());
	psz->Append(":");
	psz->Append(GetIndex());
	szName = GetName();
	if (!StrEmpty(szName))
	{
		psz->Append(":\"");
		psz->Append(szName);
		psz->Append("\"");
	}
	return psz->Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsSubRoot(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsInvalidated(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_INVALIDATED);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsDirty(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_DIRTY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::TestedForRoot(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_TESTED_FOR_ROOT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::TestedForSanity(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_TESTED_FOR_SANITY);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetDirty(bool bDirty)
{
	SetFlag(OBJECT_FLAGS_DIRTY, bDirty);
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
bool CBaseObject::InitIdentifiers(const char* szName, OIndex oi)
{
	moi = oi;
	mon.Init(szName);
	return ClipName();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::FreeIdentifiers(void)
{
	moi = INVALID_O_INDEX;
	mon.Kill();
	mon.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsHollow(void)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CEmbeddedObject* CBaseObject::TestGetPointerTo(size iToIndex)
{
	CEmbeddedObject**					ppTo;
	CEmbeddedObject*					pTo;
	CArrayTemplateEmbeddedObjectPtr		apcTos;

	apcTos.Init();
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
bool CBaseObject::ContainsPointerTo(CEmbeddedObject* pcEmbedded)
{
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::Save(CObjectWriter* pcFile)
{
	return true;
} 


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SaveManaged(CObjectWriter* pcFile)
{
	bool	bResult;

	bResult = SaveEmbeddedObjectsManaged(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = SavePointers(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = SavePrimitives(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = SaveUnmanaged(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = Save(pcFile);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SaveHeapFroms(CObjectWriter* pcFile)
{
	bool			bResult;
	size			i;
	size			uiNumElements;
	CBaseObject*	pcHeapFrom;
	int				iDistToRoot;

	iDistToRoot = GetDistToRoot();
	bResult = pcFile->WriteSInt(iDistToRoot);
	ReturnOnFalse(bResult);

	bResult = SaveEmbeddedObjectsHeapFroms(pcFile);
	ReturnOnFalse(bResult);

	uiNumElements = mapHeapFroms.NumElements();
	bResult = pcFile->WriteSize(uiNumElements);

	ReturnOnFalse(bResult);

	for (i = 0; i < uiNumElements; i++)
	{
		pcHeapFrom = (CBaseObject*)mapHeapFroms.GetPtr(i);
		bResult = pcFile->WriteDependent(pcHeapFrom);
		iDistToRoot = pcHeapFrom->GetDistToRoot();
		bResult &= pcFile->WriteSInt(iDistToRoot);
		ReturnOnFalse(bResult);
	}
	
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SaveEmbeddedObjectsManaged(CObjectWriter* pcFile)
{
	size					uiNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	size					i;
	CBaseObject*			pcEmbeddedObject;
	bool					bResult;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		bResult = pcEmbeddedObject->SaveManaged(pcFile);
		if (!bResult)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SaveEmbeddedObjectsHeapFroms(CObjectWriter* pcFile)
{
	size					uiNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	size					i;
	CBaseObject*			pcEmbeddedObject;
	bool					bResult;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		bResult = pcEmbeddedObject->SaveHeapFroms(pcFile);
		if (!bResult)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SavePointers(CObjectWriter* pcFile)
{
	size					uiNumFields;
	CPointerField**			ppacPointerFields;
	CArrayVoidPtr*			papv;
	size					i;
	CPointer*				pcPointer;
	bool					bResult;

	papv = mpcClass->GetPointerFields();
	ppacPointerFields = (CPointerField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcPointer = ppacPointerFields[i]->GetPointer(this);
		bResult = pcFile->WritePointer(pcPointer);
		if (!bResult)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SavePrimitives(CObjectWriter* pcFile)
{
	size				uiNumFields;
	CPrimitiveField**	ppacPrimitiveFields;
	CArrayVoidPtr*		papv;
	size				i;
	CPrimitiveObject*	pcPrimitive;
	bool				bResult;
	CPrimitiveField*	pcPrimitiveField;
	SDataIO*			psIO;
	void*				pvPrimitive;

	papv = mpcClass->GetPrimitiveFields();
	ppacPrimitiveFields = (CPrimitiveField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcPrimitiveField = ppacPrimitiveFields[i];
		psIO = pcPrimitiveField->GetDataIO();
		pcPrimitive = pcPrimitiveField->GetPrimitiveObject(this);
		pvPrimitive = pcPrimitiveField->GetValue(this);
		bResult = (((SDataTypeIO*)pvPrimitive)->*(psIO->fWriter))(pcFile);
		ReturnOnFalse(bResult);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::SaveUnmanaged(CObjectWriter* pcFile)
{
	size				uiNumFields;
	CArrayVoidPtr*		papv;
	size				i;
	CUnmanagedField*	pcUnmanagedField;
	bool				bResult;
	size				uiCount;
	uint32				uiSize;
	void*				pvUnmanaged;

	papv = mpcClass->GetUnmanagedFields();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcUnmanagedField = (CUnmanagedField*)papv->GetPtr(i);
		pvUnmanaged = pcUnmanagedField->GetData(this);
		uiCount = pcUnmanagedField->GetLength();
		uiSize = pcUnmanagedField->GetSizeOf();
		bResult = pcFile->WriteData(pvUnmanaged, uiCount * uiSize);

		if (!bResult)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::Load(CObjectReader* pcFile)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadManaged(CObjectReader* pcFile)
{
	bool	bResult;

	bResult = LoadEmbeddedObjectsManaged(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = LoadPointers(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = LoadPrimitives(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = LoadUnmanaged(pcFile);
	if (!bResult)
	{
		return false;
	}
	bResult = Load(pcFile);
	if (!bResult)
	{
		return false;
	}
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadEmbeddedObjectsManaged(CObjectReader* pcFile)
{
	size					uiNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	size					i;
	CBaseObject*			pcEmbeddedObject;
	bool					bResult;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		bResult = pcEmbeddedObject->LoadManaged(pcFile);
		if (!bResult)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadPointers(CObjectReader* pcFile)
{
	size				uiNumFields;
	CPointerField*		pcPointerField;
	CArrayVoidPtr*		papv;
	size				i;
	CPointer*			pcPointer;
	bool				bResult;

	papv = mpcClass->GetPointerFields();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcPointerField = (CPointerField*)papv->GetPtr(i);
		pcPointer = pcPointerField->GetPointer(this);
		bResult = pcFile->ReadPointer(pcPointer);
		if (!bResult)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadPrimitives(CObjectReader* pcFile)
{
	size				uiNumFields;
	CPrimitiveField**	ppacPrimitiveFields;
	CArrayVoidPtr*		papv;
	size				i;
	CPrimitiveObject*	pcPrimitive;
	bool				bResult;
	CPrimitiveField*	pcPrimitiveField;
	SDataIO*			psIO;
	void*				pvPrimitive;

	papv = mpcClass->GetPrimitiveFields();
	ppacPrimitiveFields = (CPrimitiveField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcPrimitiveField = ppacPrimitiveFields[i];
		psIO = pcPrimitiveField->GetDataIO();
		pcPrimitive = pcPrimitiveField->GetPrimitiveObject(this);
		pvPrimitive = pcPrimitiveField->GetValue(this);
		bResult = (((SDataTypeIO*)pvPrimitive)->*(psIO->fReader))(pcFile);

		if (!bResult)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadUnmanaged(CObjectReader* pcFile)
{
	size					uiNumFields;
	CArrayVoidPtr*			papv;
	size					i;
	CUnmanagedField*		pcUnmanagedField;
	bool					bResult;
	size					uiCount;
	uint32					uiSize;
	void*					pvUnmanaged;

	papv = mpcClass->GetUnmanagedFields();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcUnmanagedField = (CUnmanagedField*)papv->GetPtr(i);
		pvUnmanaged = pcUnmanagedField->GetData(this);
		uiCount = pcUnmanagedField->GetLength();
		uiSize = pcUnmanagedField->GetSizeOf();
		bResult = pcFile->ReadData(pvUnmanaged, uiCount * uiSize);

		if (!bResult)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadHeapFroms(CObjectReader* pcFile)
{
	bool				bResult;
	size				i;
	size				uiNumElements;
	CEmbeddedObject*	pcHeapFrom;
	int					iDistToRoot;
	
	bResult = pcFile->ReadSInt(&iDistToRoot);
	ReturnOnFalse(bResult);

	miDistToRoot = iDistToRoot;

	bResult = LoadEmbeddedObjectsHeapFroms(pcFile);
	ReturnOnFalse(bResult);

	bResult = pcFile->ReadSize(&uiNumElements);
	ReturnOnFalse(bResult);

	for (i = 0; i < uiNumElements; i++)
	{
		bResult = pcFile->ReadReverseDependent(&pcHeapFrom, this);
		ReturnOnFalse(bResult);
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::LoadEmbeddedObjectsHeapFroms(CObjectReader* pcFile)
{
	size					uiNumFields;
	CEmbeddedObjectField**	ppacEmbeddedObjectFields;
	CArrayVoidPtr*			papv;
	size					i;
	CBaseObject*			pcEmbeddedObject;
	bool					bResult;

	papv = mpcClass->GetEmbeddedObjectFields();
	ppacEmbeddedObjectFields = (CEmbeddedObjectField**)papv->GetData();
	uiNumFields = papv->NumElements();
	for (i = 0; i < uiNumFields; i++)
	{
		pcEmbeddedObject = ppacEmbeddedObjectFields[i]->GetEmbeddedObject(this);
		bResult = pcEmbeddedObject->LoadHeapFroms(pcFile);
		if (!bResult)
		{
			return false;
		}
	}

	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
OIndex CBaseObject::GetIndex(void)
{
	return moi;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsBaseObject(void)
{
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBaseObject::NumEmbedded(void)
{
	SetFlagNumEmbedded(1);
	return 1;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
uint16 CBaseObject::GetNumEmbeddedFromFlags(void)
{
	return muiNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::SetFlagNumEmbedded(size iNumEmbedded)
{
	muiNumEmbedded = (uint16)iNumEmbedded;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ClearFlagNumEmbedded(void)
{
	muiNumEmbedded = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CBaseObject::TestGetNumEmbeddedFromFlags(void)
{
	return GetNumEmbeddedFromFlags();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CObjects* CBaseObject::GetObjectsThisIn(void)
{
	CBaseObject*	pcBaseObject;
	CObjects*		pcObjects;

	pcBaseObject = GetEmbeddingContainer();
	pcObjects = pcBaseObject->mpcObjectsThisIn;
	return pcObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
CClasses* CBaseObject::GetClasses(void)
{
	CObjects*	pcObjects;

	pcObjects = GetObjects();
	if (pcObjects)
	{
		return pcObjects->GetClasses();
	}
	else
	{
		return NULL;
	}
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
bool CBaseObject::IsMarkedUnreachable(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_UNREACHABLE);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsUpdateAttachedPointerTosDistToRoot(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::HasClass(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_CALLED_CLASS);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsInitialised(void)
{
	return FixBool(muiFlags & OBJECT_FLAGS_CALLED_INIT);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ReplaceOneWithX(char* szDest, char* szMask)
{
	size i;

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
CClass* CBaseObject::GetClass(void)
{
	return mpcClass;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpFroms(void)
{
	CChars				sz;
	size				i;
	size				iNumEmbedded;
	CEmbeddedObject*	pcEmbedded;
	size				j;
	size				iNumHeapFroms;
	CEmbeddedObject*	pcFromObject;
	size				iLength;
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

	iNumEmbedded = NumEmbedded();
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
	CChars								sz;
	size								i;
	size								iNumEmbedded;
	CEmbeddedObject*					pcEmbedded;
	size								j;
	size								iNumTos;
	CEmbeddedObject**					ppcToObject;
	size								iLength;
	CChars								szLine;
	CArrayTemplateEmbeddedObjectPtr		acTos;
	CBaseObject*						pcToObject;
	size								iTotalTos;

	sz.Init();

	sz.Append("-- ");
	PrintObject(&sz);
	sz.Append(" --\n");
	iLength = sz.Length()-1;

	szLine.Init('-', iLength);
	szLine.AppendNewLine();

	iTotalTos = 0;
	iNumEmbedded = NumEmbedded();
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
		acTos.Init();
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
void CBaseObject::PrintFlags(CChars* psz)
{
	if (muiFlags & OBJECT_FLAGS_TESTED_FOR_ROOT)
	{
		psz->Append("TESTED_FOR_ROOT, ");
	}
	if (muiFlags & OBJECT_FLAGS_INVALIDATED)
	{
		psz->Append("INVALIDATED, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_CONSTRUCTOR)
	{
		psz->Append("CALLED_CONSTRUCTOR, ");
	}
	if (muiFlags & OBJECT_FLAGS_DIRTY)
	{
		psz->Append("DIRTY, ");
	}
	if (muiFlags & OBJECT_FLAGS_FREED)
	{
		psz->Append("FREED, ");
	}
	if (muiFlags & OBJECT_FLAGS_DUMPED)
	{
		psz->Append("DUMPED, ");
	}
	if (muiFlags & OBJECT_FLAGS_UNREACHABLE)
	{
		psz->Append("UNREACHABLE, ");
	}
	if (muiFlags & OBJECT_FLAGS_TESTED_FOR_SANITY)
	{
		psz->Append("TESTED_FOR_SANITY, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_ALLOCATE)
	{
		psz->Append("CALLED_ALLOCATE, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_INIT)
	{
		psz->Append("CALLED_INIT, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_KILL)
	{
		psz->Append("CALLED_KILL, ");
	}
	if (muiFlags & OBJECT_FLAGS_CALLED_CLASS)
	{
		psz->Append("CALLED_CLASS, ");
	}
	if (muiFlags & OBJECT_FLAGS_CLEARED_DIST_TO_ROOT)
	{
		psz->Append("CLEARED_DIST_TO_ROOT, ");
	}
	if (muiFlags & OBJECT_FLAGS_UPDATED_TOS_DIST_TO_ROOT)
	{
		psz->Append("UPDATED_TOS_DIST_TO_ROOT, ");
	}
	if (muiFlags & OBJECT_FLAGS_DIST_CALCULATOR_TOUCHED)
	{
		psz->Append("DIST_CALCULATOR_TOUCHED, ");
	}
	if (muiFlags & OBJECT_FLAGS_DIST_FROM_WALKED)
	{
		psz->Append("DIST_FROM_WALKED, ");
	}

	if (psz->EndsWith(", "))
	{
		psz->RemoveFromEnd(2);
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::DumpFlags(void)
{
	CChars	sz;

	sz.Init();
	PrintFlags(&sz);
	sz.AppendNewLine();
	sz.DumpKill();
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
void CBaseObject::ValidateFlagNotSet(uint16 iFlag, char* szFlag)
{
	CChars	sz;

	if (muiFlags & iFlag)
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
void CBaseObject::ValidateFlagSet(uint16 iFlag, char* szFlag)
{
	CChars	sz;

	if (!(muiFlags & iFlag))
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

	CChars	sz;
	bool	bCanFindRoot;

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
	uint16	iEmbeddedFlags;
	uint16	iThisFlags;
	uint16	iIgnoredFlags;
	char*	szEmbeddedFlags;
	char*	szFlags;
	char*	szIgnoredFlags;

	if (IsEmbedded())
	{
		iIgnoredFlags = OBJECT_FLAGS_CALLED_CLASS;
		iEmbeddedFlags = mpcEmbedded->muiFlags & ~iIgnoredFlags;
		iThisFlags = muiFlags & ~iIgnoredFlags;
		if ((iEmbeddedFlags) != (iThisFlags))
		{
			sz.Init();
			PrintObject(&sz, IsEmbedded());
			szFlags = ShortToFlags(muiFlags);
			szEmbeddedFlags = ShortToFlags(mpcEmbedded->muiFlags);
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
	ValidateFlagNotSet(OBJECT_FLAGS_FREED, "OBJECT_FLAGS_FREED");
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
	bool	bDistToStackZero;
	bool	bAllocateCalled;
	bool	bInObjects;
	bool	bAllSame;
	CChars	sz;

	bDistToStackZero = GetDistToStack() == 0;
	bAllocateCalled = FixBool(muiFlags & OBJECT_FLAGS_CALLED_ALLOCATE);
	bInObjects = IsAllocatedInObjects();
	bAllSame = !bDistToStackZero == bAllocateCalled == bInObjects;

	if (!bAllSame)
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", sz.Text(), "} should not have a dist to stack of [", IntToString(GetDistToStack()), "] and flag OBJECT_FLAGS_CALLED_ALLOCATE [", IntToString(bAllocateCalled), "] and be allocated in Objects [0x", PointerToString(GetObjectsThisIn()), "].", NULL);
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
			pcContainer->PrintObject(&szContainer, false);

			szThis.Init();
			PrintObject(&szThis, IsEmbedded());

			gcLogger.Error2(__METHOD__, " Object {", szThis.Text(), "} should not have a name as it's embedded in object {", szContainer.Text(), "}.", NULL);

			szThis.Kill();
			szContainer.Kill();
		}
		else
		{
			szName = GetName();
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

	if (!IsEmbedded())
	{
		pcThis = mpcObjectsThisIn->GetFromMemory(GetIndex());
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
void CBaseObject::ValidateHasClassFlag(char* szMethod)
{
	CChars sz;

	if (!HasClass())
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());

		gcLogger.Error2(szMethod, " Object {", sz.Text(), "} does not have Class initialised.  Call PreInit() first in Init().", NULL);

		sz.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateHasClassField(char* szMethod)
{
	CChars sz;

	if (mpcClass == NULL)
	{
		sz.Init();
		PrintObject(&sz, IsEmbedded());

		gcLogger.Error2(szMethod, " Object {", sz.Text(), "} does not have Class field set.", NULL);

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

	if (muiPreInits != muiPostInits)
	{
		szObject.Init();
		PrintObject(&szObject, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", szObject.Text(), "} has pre-inits [", IntToString(muiPreInits), "] not equal to post inits [", IntToString(muiPostInits), "].  Call PreInit() first in Init().", NULL);
		szObject.Kill();
	}
	else if (muiPreInits == 0)
	{
		if (!IsEmbedded())
		{
			szObject.Init();
			PrintObject(&szObject, IsEmbedded());
			gcLogger.Error2(__METHOD__, " Object {", szObject.Text(), "} has a pre-init of zero.  Call PreInit() first in Init().", NULL);
			szObject.Kill();
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::ValidateKillCalled(void)
{
	CChars	szObject;

	if (!(muiFlags & OBJECT_FLAGS_CALLED_KILL))
	{
		szObject.Init();
		PrintObject(&szObject, IsEmbedded());
		gcLogger.Error2(__METHOD__, " Object {", szObject.Text(), "} has not beel killed.  Ensure sub-classes are declared DESTRUCTABLE().", NULL);
		szObject.Kill();
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
char* CBaseObject::GetName(void)
{
	return mon.Text();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::IsNamed(const char* szName)
{
	if (StrEmpty(szName))
	{
		return mon.Equals(szName);
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CBaseObject::ClipName(void)
{
	bool	bResult;

	bResult = true;
	if (mon.Contains("\\"))
	{
		mon.Replace("\\", "/");
		bResult = false;
	}
	if (mon.Length() >= MAX_NAMED_OBJECT_NAME_LENGTH)
	{
		mon.SetLength(MAX_NAMED_OBJECT_NAME_LENGTH - 1);
		bResult = false;
	}

	return bResult;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Pointer(CPointer* pcPointer, char* szFieldName)
{
	if (mpcClass == NULL)
	{
		ValidateHasClassField(__METHOD__);
	}
	mpcClass->Pointer(this, pcPointer, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Embedded(CBaseObject* pcObject, char* szFieldName)
{
	if (mpcClass == NULL)
	{
		ValidateHasClassField(__METHOD__);
	}
	mpcClass->Embedded(this, pcObject, szFieldName);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::Primitive(Int8* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(UInt8* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(Int16* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(UInt16* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(Int32* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(UInt32* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(Int64* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(UInt64* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(Bool* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(Float32* pcPrimitive, char* szFieldName)	{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(Float64* pcPrimitive, char* szFieldName)	{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(Char8* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }
void CBaseObject::Primitive(Char16* pcPrimitive, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Primitive(this, pcPrimitive, szFieldName); }


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CBaseObject::UnmanagedInt8(int8* pc, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedInt8(this, pc, szFieldName); };
void CBaseObject::UnmanagedInt8(int8* pac, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedInt8(this, pac, uiLength, szFieldName); };
void CBaseObject::UnmanagedInt16(int16* pi, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedInt16(this, pi, szFieldName); };
void CBaseObject::UnmanagedInt16(int16* pai, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedInt16(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedInt32(int32* pi, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedInt32(this, pi, szFieldName); };
void CBaseObject::UnmanagedInt32(int32* pai, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedInt32(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedInt64(int64* pi, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedInt64(this, pi, szFieldName); };
void CBaseObject::UnmanagedInt64(int64* pai, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedInt64(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedSInt(int* pi, char* szFieldName)							{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedSInt(this, pi, szFieldName); };
void CBaseObject::UnmanagedSInt(int* pai, size uiLength, char* szFieldName)			{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedSInt(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedUInt8(uint8* pc, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedUInt8(this, pc, szFieldName); };
void CBaseObject::UnmanagedUInt8(uint8* pac, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedUInt8(this, pac, uiLength, szFieldName); };
void CBaseObject::UnmanagedUInt16(uint16* pi, char* szFieldName)					{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedUInt16(this, pi, szFieldName); };
void CBaseObject::UnmanagedUInt16(uint16* pai, size uiLength, char* szFieldName)	{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedUInt16(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedUInt32(uint32* pi, char* szFieldName)					{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedUInt32(this, pi, szFieldName); };
void CBaseObject::UnmanagedUInt32(uint32* pai, size uiLength, char* szFieldName)	{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedUInt32(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedUInt64(uint64* pi, char* szFieldName)					{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedUInt64(this, pi, szFieldName); };
void CBaseObject::UnmanagedUInt64(uint64* pai, size uiLength, char* szFieldName)	{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedUInt64(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedSUInt(uint* pi, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedSUInt(this, pi, szFieldName); };
void CBaseObject::UnmanagedSUInt(uint* pai, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedSUInt(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedSize(size* pi, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedSize(this, pi, szFieldName); };
void CBaseObject::UnmanagedSize(size* pai, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedSize(this, pai, uiLength, szFieldName); };
void CBaseObject::UnmanagedFloat(float32* pf, char* szFieldName)					{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CBaseObject::UnmanagedFloat(float32* paf, size uiLength, char* szFieldName)	{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedFloat(this, paf, uiLength, szFieldName); }
void CBaseObject::UnmanagedFloat(float64* pf, char* szFieldName)					{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CBaseObject::UnmanagedFloat(float64* paf, size uiLength, char* szFieldName)	{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedFloat(this, paf, uiLength, szFieldName); }
void CBaseObject::UnmanagedFloat(float96* pf, char* szFieldName)					{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedFloat(this, pf, szFieldName); }
void CBaseObject::UnmanagedFloat(float96* paf, size uiLength, char* szFieldName)	{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedFloat(this, paf, uiLength, szFieldName); }
void CBaseObject::UnmanagedChar(char8* pc, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedChar(this, pc, szFieldName); }
void CBaseObject::UnmanagedChar(char8* pac, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedChar(this, pac, uiLength, szFieldName); }
void CBaseObject::UnmanagedChar(char16* pc, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedChar(this, pc, szFieldName); }
void CBaseObject::UnmanagedChar(char16* pac, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedChar(this, pac, uiLength, szFieldName); }
void CBaseObject::UnmanagedBool(bool* pb, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedBool(this, pb, szFieldName); }
void CBaseObject::UnmanagedBool(bool* pab, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedBool(this, pab, uiLength, szFieldName); }

void CBaseObject::UnmanagedEnum(void* pe, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedEnum(this, pe, szFieldName); }

void CBaseObject::UnmanagedString(CChars* pcChars, char* szFieldName)				{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL);  mpcClass->UnmanagedString(this, pcChars, szFieldName); };
void CBaseObject::UnmanagedString(CCharsImmutable* pcChars, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL);  mpcClass->UnmanagedString(this, pcChars, szFieldName); };
void CBaseObject::UnmanagedNumber(CNumber* pcNumber, char* szFieldName)				{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL);  mpcClass->UnmanagedNumber(this, pcNumber, szFieldName); };
void CBaseObject::UnmanagedDate(CDate* pcDate, char* szFieldName)					{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL);  mpcClass->UnmanagedDate(this, pcDate, szFieldName); };

void CBaseObject::UnmanagedData(void* pv, size uiSizeof, char* szFieldName)			{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedData(this, pv, uiSizeof, szFieldName); };
void CBaseObject::UnmanagedUnknown(void* pv, size uiSizeof, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedUnknown(this, pv, uiSizeof, szFieldName); };
void CBaseObject::UnmanagedPointer(void** pv, char* szFieldName)					{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->UnmanagedPointer(this, pv, szFieldName); };

void CBaseObject::Unmanaged2Float(SFloat2* pf, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Unmanaged2Float(this, pf, szFieldName); }
void CBaseObject::Unmanaged2Float(SFloat2* pf, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Unmanaged2Float(this, pf, uiLength, szFieldName); }
void CBaseObject::Unmanaged3Float(SFloat3* pf, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Unmanaged3Float(this, pf, szFieldName); }
void CBaseObject::Unmanaged3Float(SFloat3* pf, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Unmanaged3Float(this, pf, uiLength, szFieldName); }
void CBaseObject::Unmanaged4Float(SFloat4* pf, char* szFieldName)						{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Unmanaged4Float(this, pf, szFieldName); }
void CBaseObject::Unmanaged4Float(SFloat4* pf, size uiLength, char* szFieldName)		{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Unmanaged4Float(this, pf, uiLength, szFieldName); }
void CBaseObject::Unmanaged4x4Float(SFloat4x4* pf, char* szFieldName)					{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Unmanaged4x4Float(this, pf, szFieldName); }
void CBaseObject::Unmanaged4x4Float(SFloat4x4* pf, size uiLength, char* szFieldName)	{ ValidateHasClassField(mpcClass == NULL ? __METHOD__ : NULL); mpcClass->Unmanaged4x4Float(this, pf, uiLength, szFieldName); }

