#ifndef __OBJECT_ALLOCATOR_H__
#define __OBJECT_ALLOCATOR_H__
#include "Pointer.h"


//When are we:
//  Allocating an object that already exists on disk...
//    With a Name and Index?
//    With an Index?
//  Allocating an object that does not exist at all?
//  Allocating and overwriting an object that already exists on disk...
//    With a Name?
//  Allocating and overwriting an object that already exists in memory...
//    With a Name?


//This is a private style helper class for allocating objects with a view to either over-writing or re-allocating existing objects that share the index being allocated.
//If you just want to alllocate an object then use Objects.Add
class CObjects;
class CObjectAllocator
{
protected:
	CObjects*	mpcObjects;

public:

};


#endif // __OBJECT_ALLOCATOR_H__

