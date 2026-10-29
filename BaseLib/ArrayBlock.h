#ifndef __ARRAY_BASE_H__
#define __ARRAY_BASE_H__
#include "PointerRemapper.h"
#include "Mallocator.h"
#include "FileIO.h"
#include "Malloc.h"
#include "ArrayElementNotFound.h"
#include "DataCompare.h"


//Arrays of pointers write this, the size of a pointer in a 32 bit build, as their element size so the file is the same
//in 32 and 64 bit builds.
#define ARRAY_FILE_POINTER_SIZE	4


#include "ArraySizeNatural.inl"
#include "ArrayBlockClass.inl"
#include "ArraySizeUndef.inl"

#include "ArraySizeLarge.inl"
#include "ArrayBlockClass.inl"
#include "ArraySizeUndef.inl"


#endif // __ARRAY_BASE_H__

