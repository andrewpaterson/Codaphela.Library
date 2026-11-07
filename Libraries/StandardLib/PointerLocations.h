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
#ifndef __POINTER_LOCATIONS_H__
#define __POINTER_LOCATIONS_H__
#include "BaseLib/PrimitiveTypes.h"


struct SPointerLocation
{
	void*	pvPointer;  //NULL for an empty slot.
	ptr		uiLocation;
};


struct SPointerLocationsIterator
{
	void*	pvPointer;
	size	uiSlot;
};


//Where each of many pointers is held: a pointer may have any number of locations (a member of a collection more than
//once, or under more than one key).  What a location means is up to the holder: a slot in an array, a key...  A holder
//with many members builds one so that finding a member isn't a search of all of them.  Open addressing, with linear
//probing; removing shifts the following slots back, so nothing is left behind to slow lookups down.
class CPointerLocations
{
private:
	SPointerLocation*	masSlots;
	size				muiNumSlots;  //A power of two, or 0.
	size				muiNumLocations;

public:
	void	Init(void);
	void	Kill(void);

	void	Add(void* pvPointer, ptr uiLocation);
	bool	Remove(void* pvPointer, ptr uiLocation);
	bool	Contains(void* pvPointer, ptr uiLocation);
	size	NumLocations(void);
	size	NumLocations(void* pvPointer);

	bool	StartIteration(SPointerLocationsIterator* psIter, void* pvPointer, ptr* puiLocation);
	bool	Iterate(SPointerLocationsIterator* psIter, ptr* puiLocation);
	bool	StartAllIteration(SPointerLocationsIterator* psIter, void** ppvPointer, ptr* puiLocation);
	bool	IterateAll(SPointerLocationsIterator* psIter, void** ppvPointer, ptr* puiLocation);

	size	MovePointer(void* pvOld, void* pvNew);
	void	MoveLocations(ptr uiFrom, ptrdiff iBy);

private:
	size	HomeSlot(void* pvPointer);
	size	NextSlot(size uiSlot);
	bool	Grow(void);
	void	RemoveSlot(size uiSlot);
};


#endif // __POINTER_LOCATIONS_H__

