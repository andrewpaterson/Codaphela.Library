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

