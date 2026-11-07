#include "BaseLib/SystemAllocator.h"
#include <stdlib.h>
#include <string.h>
#include "BaseLib/ArrayTemplate.h"
#include "BaseLib/PointerFunctions.h"
#include "BaseLib/Logger.h"
#include "PointerLocations.h"


#define POINTER_LOCATIONS_MIN_SLOTS		16


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerLocations::Init(void)
{
	masSlots = NULL;
	muiNumSlots = 0;
	muiNumLocations = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
void CPointerLocations::Kill(void)
{
	gcSystemAllocator.Free(masSlots);
	masSlots = NULL;
	muiNumSlots = 0;
	muiNumLocations = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// The table is kept at most half full.
//
//////////////////////////////////////////////////////////////////////////
void CPointerLocations::Add(void* pvPointer, ptr uiLocation)
{
	size	uiSlot;

	if ((muiNumLocations + 1) * 2 > muiNumSlots)
	{
		if (!Grow())
		{
			gcLogger.Error2(__METHOD__, " Could not allocate a location.", NULL);
			return;
		}
	}

	uiSlot = HomeSlot(pvPointer);
	while (masSlots[uiSlot].pvPointer != NULL)
	{
		uiSlot = NextSlot(uiSlot);
	}
	masSlots[uiSlot].pvPointer = pvPointer;
	masSlots[uiSlot].uiLocation = uiLocation;
	muiNumLocations++;
}


//////////////////////////////////////////////////////////////////////////
//
// Removes one of pvPointer's locations, uiLocation.
//
//////////////////////////////////////////////////////////////////////////
bool CPointerLocations::Remove(void* pvPointer, ptr uiLocation)
{
	size	uiSlot;

	if (muiNumSlots == 0)
	{
		return false;
	}

	uiSlot = HomeSlot(pvPointer);
	while (masSlots[uiSlot].pvPointer != NULL)
	{
		if ((masSlots[uiSlot].pvPointer == pvPointer) && (masSlots[uiSlot].uiLocation == uiLocation))
		{
			RemoveSlot(uiSlot);
			muiNumLocations--;
			return true;
		}
		uiSlot = NextSlot(uiSlot);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointerLocations::Contains(void* pvPointer, ptr uiLocation)
{
	SPointerLocationsIterator	sIter;
	ptr							uiFound;
	bool						bExists;

	bExists = StartIteration(&sIter, pvPointer, &uiFound);
	while (bExists)
	{
		if (uiFound == uiLocation)
		{
			return true;
		}
		bExists = Iterate(&sIter, &uiFound);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointerLocations::NumLocations(void)
{
	return muiNumLocations;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointerLocations::NumLocations(void* pvPointer)
{
	SPointerLocationsIterator	sIter;
	ptr							uiLocation;
	bool						bExists;
	size						uiCount;

	uiCount = 0;
	bExists = StartIteration(&sIter, pvPointer, &uiLocation);
	while (bExists)
	{
		uiCount++;
		bExists = Iterate(&sIter, &uiLocation);
	}
	return uiCount;
}


//////////////////////////////////////////////////////////////////////////
//
// pvPointer's first location (false if it has none).  Iterate gives the
// rest.  The table mustn't change while iterating.
//
//////////////////////////////////////////////////////////////////////////
bool CPointerLocations::StartIteration(SPointerLocationsIterator* psIter, void* pvPointer, ptr* puiLocation)
{
	if (muiNumSlots == 0)
	{
		return false;
	}

	psIter->pvPointer = pvPointer;
	psIter->uiSlot = HomeSlot(pvPointer);
	while (masSlots[psIter->uiSlot].pvPointer != NULL)
	{
		if (masSlots[psIter->uiSlot].pvPointer == pvPointer)
		{
			*puiLocation = masSlots[psIter->uiSlot].uiLocation;
			return true;
		}
		psIter->uiSlot = NextSlot(psIter->uiSlot);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointerLocations::Iterate(SPointerLocationsIterator* psIter, ptr* puiLocation)
{
	psIter->uiSlot = NextSlot(psIter->uiSlot);
	while (masSlots[psIter->uiSlot].pvPointer != NULL)
	{
		if (masSlots[psIter->uiSlot].pvPointer == psIter->pvPointer)
		{
			*puiLocation = masSlots[psIter->uiSlot].uiLocation;
			return true;
		}
		psIter->uiSlot = NextSlot(psIter->uiSlot);
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// Every pointer's every location, in no order.  The table mustn't change
// while iterating.
//
//////////////////////////////////////////////////////////////////////////
bool CPointerLocations::StartAllIteration(SPointerLocationsIterator* psIter, void** ppvPointer, ptr* puiLocation)
{
	psIter->pvPointer = NULL;
	psIter->uiSlot = 0;
	if (muiNumSlots == 0)
	{
		return false;
	}
	psIter->uiSlot = muiNumSlots;
	return IterateAll(psIter, ppvPointer, puiLocation);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointerLocations::IterateAll(SPointerLocationsIterator* psIter, void** ppvPointer, ptr* puiLocation)
{
	while (psIter->uiSlot > 0)
	{
		psIter->uiSlot--;
		if (masSlots[psIter->uiSlot].pvPointer != NULL)
		{
			*ppvPointer = masSlots[psIter->uiSlot].pvPointer;
			*puiLocation = masSlots[psIter->uiSlot].uiLocation;
			return true;
		}
	}
	return false;
}


//////////////////////////////////////////////////////////////////////////
//
// pvOld's locations become pvNew's (a hollow object replaced by the
// object loaded for it).  Returns how many there were.
//
//////////////////////////////////////////////////////////////////////////
size CPointerLocations::MovePointer(void* pvOld, void* pvNew)
{
	CArrayTemplate<ptr>			auiLocations;
	SPointerLocationsIterator	sIter;
	ptr							uiLocation;
	bool						bExists;
	size						i;
	size						uiNumLocations;

	auiLocations.Init();
	bExists = StartIteration(&sIter, pvOld, &uiLocation);
	while (bExists)
	{
		auiLocations.Add(&uiLocation);
		bExists = Iterate(&sIter, &uiLocation);
	}

	uiNumLocations = auiLocations.NumElements();
	for (i = 0; i < uiNumLocations; i++)
	{
		uiLocation = *auiLocations.Get(i);
		Remove(pvOld, uiLocation);
		Add(pvNew, uiLocation);
	}
	auiLocations.Kill();
	return uiNumLocations;
}


//////////////////////////////////////////////////////////////////////////
//
// Every location at or after uiFrom moves by iBy: an array's slots after
// one is inserted or removed.  The pointers don't change, so nothing moves
// in the table.
//
//////////////////////////////////////////////////////////////////////////
void CPointerLocations::MoveLocations(ptr uiFrom, ptrdiff iBy)
{
	size	i;

	for (i = 0; i < muiNumSlots; i++)
	{
		if ((masSlots[i].pvPointer != NULL) && (masSlots[i].uiLocation >= uiFrom))
		{
			masSlots[i].uiLocation = (ptr)((ptrdiff)masSlots[i].uiLocation + iBy);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//
// Objects are aligned, so the low bits say little.
//
//////////////////////////////////////////////////////////////////////////
size CPointerLocations::HomeSlot(void* pvPointer)
{
	ptr		uiHash;

	uiHash = (ptr)pvPointer;
	uiHash = (uiHash >> 4) ^ (uiHash >> 13);
	return (size)(uiHash & (muiNumSlots - 1));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
size CPointerLocations::NextSlot(size uiSlot)
{
	return (uiSlot + 1) & (muiNumSlots - 1);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
bool CPointerLocations::Grow(void)
{
	SPointerLocation*	asOldSlots;
	SPointerLocation*	asNewSlots;
	size				uiOldNumSlots;
	size				uiNumSlots;
	size				i;

	uiNumSlots = (muiNumSlots == 0) ? POINTER_LOCATIONS_MIN_SLOTS : muiNumSlots * 2;
	asNewSlots = (SPointerLocation*)gcSystemAllocator.Malloc((lsize)uiNumSlots * sizeof(SPointerLocation));
	if (asNewSlots == NULL)
	{
		//Grow is called while the slots are at most half full, so they still have room, just slower.
		return muiNumSlots != 0;
	}
	memset(asNewSlots, 0, (lsize)uiNumSlots * sizeof(SPointerLocation));

	asOldSlots = masSlots;
	uiOldNumSlots = muiNumSlots;
	masSlots = asNewSlots;
	muiNumSlots = uiNumSlots;
	muiNumLocations = 0;

	for (i = 0; i < uiOldNumSlots; i++)
	{
		if (asOldSlots[i].pvPointer != NULL)
		{
			Add(asOldSlots[i].pvPointer, asOldSlots[i].uiLocation);
		}
	}
	gcSystemAllocator.Free(asOldSlots);
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Empties uiSlot and moves back the following slots that can't be found
// past an empty one, so no lookup stops short.
//
//////////////////////////////////////////////////////////////////////////
void CPointerLocations::RemoveSlot(size uiSlot)
{
	size	uiNext;
	size	uiHome;
	bool	bMove;

	uiNext = uiSlot;
	for (;;)
	{
		uiNext = NextSlot(uiNext);
		if (masSlots[uiNext].pvPointer == NULL)
		{
			break;
		}

		//The entry at uiNext can move to uiSlot unless its home is in (uiSlot, uiNext], going around the end.
		uiHome = HomeSlot(masSlots[uiNext].pvPointer);
		if (uiSlot <= uiNext)
		{
			bMove = (uiHome <= uiSlot) || (uiHome > uiNext);
		}
		else
		{
			bMove = (uiHome <= uiSlot) && (uiHome > uiNext);
		}

		if (bMove)
		{
			masSlots[uiSlot] = masSlots[uiNext];
			uiSlot = uiNext;
		}
	}
	masSlots[uiSlot].pvPointer = NULL;
	masSlots[uiSlot].uiLocation = 0;
}

