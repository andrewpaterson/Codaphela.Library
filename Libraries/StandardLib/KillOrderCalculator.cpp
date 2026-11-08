/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

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
#include "BaseObject.h"
#include "Objects.h"
#include "KillOrderCalculator.h"


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CKillOrderCalculator::Init(CArrayBaseObjectPtr* papcKilled)
{
	mpapcKilled = papcKilled;
	mapcKilledNesObjects.Init();
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
void CKillOrderCalculator::Kill(void)
{
	mapcKilledNesObjects.Kill();
	mpapcKilled = NULL;
}


//////////////////////////////////////////////////////////////////////////
//																		//
//																		//
//////////////////////////////////////////////////////////////////////////
CArrayBaseObjectPtr* CKillOrderCalculator::Calculate(void)
{
	size			i;
	size			uiNumElements;
	size			uiKept;
	CBaseObject*	pcObject;

	//Both keep the order they were found in.
	uiKept = 0;
	uiNumElements = mpapcKilled->NumElements();
	for (i = 0; i < uiNumElements; i++)
	{
		pcObject = mpapcKilled->GetPtr(i);
		if (pcObject->IsNesPointedTo())
		{
			mapcKilledNesObjects.Add(pcObject);
		}
		else
		{
			*mpapcKilled->Get(uiKept) = pcObject;
			uiKept++;
		}
	}
	mpapcKilled->FakeSetUsedElements(uiKept);

	OrderNesObjects();
	return &mapcKilledNesObjects;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static int CompareKillOrderObject(const void* pv1, const void* pv2)
{
	CBaseObject*	pc1;
	CBaseObject*	pc2;

	pc1 = ((SKillOrderObject*)pv1)->pcObject;
	pc2 = ((SKillOrderObject*)pv2)->pcObject;
	if (pc1 < pc2)
	{
		return -1;
	}
	else if (pc1 > pc2)
	{
		return 1;
	}
	return 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The objects that can go next, by where they were found, smallest first.
//
//////////////////////////////////////////////////////////////////////////
static void KillOrderReadyPush(CArrayTemplate<size>* pauiReady, size uiFound)
{
	size	uiChild;
	size	uiParent;
	size	uiSwap;

	pauiReady->Push(&uiFound);
	uiChild = pauiReady->NumElements() - 1;
	while (uiChild > 0)
	{
		uiParent = (uiChild - 1) / 2;
		if (*pauiReady->Get(uiParent) <= *pauiReady->Get(uiChild))
		{
			break;
		}
		uiSwap = *pauiReady->Get(uiParent);
		*pauiReady->Get(uiParent) = *pauiReady->Get(uiChild);
		*pauiReady->Get(uiChild) = uiSwap;
		uiChild = uiParent;
	}
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
static size KillOrderReadyPop(CArrayTemplate<size>* pauiReady)
{
	size	uiFirst;
	size	uiLast;
	size	uiNum;
	size	uiParent;
	size	uiChild;
	size	uiSwap;

	uiFirst = *pauiReady->Get(0);
	pauiReady->Pop(&uiLast);
	uiNum = pauiReady->NumElements();
	if (uiNum == 0)
	{
		return uiFirst;
	}

	*pauiReady->Get(0) = uiLast;
	uiParent = 0;
	for (;;)
	{
		uiChild = uiParent * 2 + 1;
		if (uiChild >= uiNum)
		{
			break;
		}
		if ((uiChild + 1 < uiNum) && (*pauiReady->Get(uiChild + 1) < *pauiReady->Get(uiChild)))
		{
			uiChild++;
		}
		if (*pauiReady->Get(uiParent) <= *pauiReady->Get(uiChild))
		{
			break;
		}
		uiSwap = *pauiReady->Get(uiParent);
		*pauiReady->Get(uiParent) = *pauiReady->Get(uiChild);
		*pauiReady->Get(uiChild) = uiSwap;
		uiParent = uiChild;
	}
	return uiFirst;
}


//////////////////////////////////////////////////////////////////////////
//
// Orders the Nes pointed to objects so each is freed after the objects
// freed with it that hold Nes pointers to it (a chain: a component needs
// its panel, which needs its window).  Of those that can go next the
// first found goes.  Objects holding Nes pointers to each other in a cycle
// can't be ordered; they are freed in the order found.
//
// Each object counts the waiting objects that hold Nes pointers to it;
// ordering a holder counts it off the objects it holds, and those that
// get to none can go.  Linear in the objects and their Nes pointers (and
// a log for picking the first found).
//
//////////////////////////////////////////////////////////////////////////
void CKillOrderCalculator::OrderNesObjects(void)
{
	CArrayTemplate<SKillOrderNeed>	asNeeds;
	CArrayTemplate<size>			auiNumHolders;
	CArrayTemplate<size>			auiFirstNeed;
	CArrayTemplate<size>			auiHeld;
	CArrayTemplate<size>			auiReady;
	CArrayBaseObjectPtr				apcOrdered;
	SKillOrderNeed*					psNeed;
	size							uiNumObjects;
	size							uiNumNeeds;
	size							i;
	size							uiFound;
	size							uiNeed;
	size							uiHeld;

	uiNumObjects = mapcKilledNesObjects.NumElements();
	if (uiNumObjects < 2)
	{
		return;
	}

	asNeeds.Init();
	auiNumHolders.Init();
	FindNeeds(&asNeeds, &auiNumHolders);

	//Each holder's needs, together: the held objects of holder i are auiHeld[auiFirstNeed[i]] to before auiFirstNeed[i + 1].
	uiNumNeeds = asNeeds.NumElements();
	auiFirstNeed.Init();
	auiFirstNeed.GrowToAtLeastNumElements(uiNumObjects + 1, true, 0);
	for (i = 0; i < uiNumNeeds; i++)
	{
		(*auiFirstNeed.Get(asNeeds.Get(i)->uiHolder + 1))++;
	}
	for (i = 0; i < uiNumObjects; i++)
	{
		*auiFirstNeed.Get(i + 1) += *auiFirstNeed.Get(i);
	}
	auiHeld.Init();
	auiHeld.GrowToAtLeastNumElements(uiNumNeeds);
	for (i = 0; i < uiNumNeeds; i++)
	{
		psNeed = asNeeds.Get(i);
		*auiHeld.Get((*auiFirstNeed.Get(psNeed->uiHolder))++) = psNeed->uiHeld;
	}
	//Filling moved each start to the next holder's.
	for (i = uiNumObjects; i > 0; i--)
	{
		*auiFirstNeed.Get(i) = *auiFirstNeed.Get(i - 1);
	}
	*auiFirstNeed.Get(0) = 0;
	asNeeds.Kill();

	auiReady.Init();
	for (i = 0; i < uiNumObjects; i++)
	{
		if (*auiNumHolders.Get(i) == 0)
		{
			KillOrderReadyPush(&auiReady, i);
		}
	}

	apcOrdered.Init();
	while (auiReady.NumElements() > 0)
	{
		uiFound = KillOrderReadyPop(&auiReady);
		apcOrdered.Add(mapcKilledNesObjects.GetPtr(uiFound));
		for (uiNeed = *auiFirstNeed.Get(uiFound); uiNeed < *auiFirstNeed.Get(uiFound + 1); uiNeed++)
		{
			uiHeld = *auiHeld.Get(uiNeed);
			(*auiNumHolders.Get(uiHeld))--;
			if (*auiNumHolders.Get(uiHeld) == 0)
			{
				KillOrderReadyPush(&auiReady, uiHeld);
			}
		}
	}

	if (apcOrdered.NumElements() != uiNumObjects)
	{
		//Those left wait on each other: a cycle, or held from one.
		i = 0;
		while (*auiNumHolders.Get(i) == 0)
		{
			i++;
		}
		gcLogger.Error2(__METHOD__, " Objects freed together hold Nes pointers to each other in a cycle, starting with {", ObjectToString(mapcKilledNesObjects.GetPtr(i)), "}.  They are freed in the order found.", NULL);
		for (; i < uiNumObjects; i++)
		{
			if (*auiNumHolders.Get(i) != 0)
			{
				apcOrdered.Add(mapcKilledNesObjects.GetPtr(i));
			}
		}
	}

	mapcKilledNesObjects.ReInit();
	mapcKilledNesObjects.InsertArrayAfterEnd(&apcOrdered);

	apcOrdered.Kill();
	auiReady.Kill();
	auiHeld.Kill();
	auiFirstNeed.Kill();
	auiNumHolders.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// Each Nes pointer from one of the objects to another (or to an object
// embedded in another) is a need, and each object counts the needs of it.
// A Nes pointer from an object to itself isn't one.
//
//////////////////////////////////////////////////////////////////////////
void CKillOrderCalculator::FindNeeds(CArrayTemplate<SKillOrderNeed>* pasNeeds, CArrayTemplate<size>* pauiNumHolders)
{
	CArrayTemplate<SKillOrderObject>	asByAddress;
	CArrayEmbeddedObjectPtrE8			apcHolders;
	SKillOrderObject*					psObject;
	SKillOrderObject					sHolder;
	SKillOrderNeed*						psNeed;
	CBaseObject*						pcObject;
	size								uiNumObjects;
	size								i;
	size								j;
	size								uiNumHolders;
	size								uiIndex;

	uiNumObjects = mapcKilledNesObjects.NumElements();
	asByAddress.Init();
	for (i = 0; i < uiNumObjects; i++)
	{
		psObject = asByAddress.Add();
		psObject->pcObject = mapcKilledNesObjects.GetPtr(i);
		psObject->uiFound = i;
	}
	asByAddress.QuickSort(&CompareKillOrderObject);
	sHolder.uiFound = 0;

	pauiNumHolders->GrowToAtLeastNumElements(uiNumObjects, true, 0);
	apcHolders.Init();
	for (i = 0; i < uiNumObjects; i++)
	{
		pcObject = mapcKilledNesObjects.GetPtr(i);
		apcHolders.ReInit();
		pcObject->GetNesPointerToThis(&apcHolders);
		uiNumHolders = apcHolders.NumElements();
		for (j = 0; j < uiNumHolders; j++)
		{
			sHolder.pcObject = apcHolders.GetPtr(j)->GetEmbeddingContainer();
			if ((sHolder.pcObject != pcObject) && asByAddress.FindInSorted(&sHolder, &CompareKillOrderObject, &uiIndex))
			{
				psNeed = pasNeeds->Add();
				psNeed->uiHolder = asByAddress.Get(uiIndex)->uiFound;
				psNeed->uiHeld = i;
				(*pauiNumHolders->Get(i))++;
			}
		}
	}
	apcHolders.Kill();
	asByAddress.Kill();
}
