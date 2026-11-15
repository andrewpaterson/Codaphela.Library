#ifndef __ARRAY_BASE_H__
#define __ARRAY_BASE_H__
#include "PointerRemapper.h"
#include "Mallocator.h"
#include "FileIO.h"
#include "Malloc.h"
#include "ArrayElementNotFound.h"
#include "DataCompare.h"
#include "StringHelper.h"


//Arrays of pointers write this, the size of a pointer in a 32 bit build, as their element size so the file is the same
//in 32 and 64 bit builds.
#define ARRAY_FILE_POINTER_SIZE	4


//What an array counted in size (natural) and one counted in lsize (large) differ in, given to the array templates as
//their SIZE.  A natural array on the W65C816 holds at most MAX_SIZE bytes, so it moves them with Libc65's 16 bit
//counts.  Where size is 16 bits (SIZE_16) a natural array that would need more than MAX_SIZE bytes fails as if out of
//memory (CheckSize).
struct SArrayNaturalSize
{
	typedef size		Count;

	static const size	Max = MAX_SIZE;
	static const size	NotFound = ARRAY_ELEMENT_NOT_FOUND;
#if defined(SIZE_16)
	static const bool	CheckSize = true;
#else
	static const bool	CheckSize = false;
#endif // SIZE_16

#if defined(__W65C816__)
	static void*	Copy(void* pvDest, const void* pvSource, size uiBytes)	{ return memcpy16(pvDest, pvSource, uiBytes); }
	static void*	Move(void* pvDest, const void* pvSource, size uiBytes)	{ return memmove16(pvDest, pvSource, uiBytes); }
	static void*	Set(void* pvDest, int iValue, size uiBytes)				{ return memset16(pvDest, iValue, uiBytes); }
	static int		Compare(const void* pv1, const void* pv2, size uiBytes)	{ return memcmp16(pv1, pv2, uiBytes); }
	static void		Swap(const void* pv1, const void* pv2, size uiBytes)	{ MemSwp16(pv1, pv2, uiBytes); }
#else
	static void*	Copy(void* pvDest, const void* pvSource, size uiBytes)	{ return memcpy(pvDest, pvSource, uiBytes); }
	static void*	Move(void* pvDest, const void* pvSource, size uiBytes)	{ return memmove(pvDest, pvSource, uiBytes); }
	static void*	Set(void* pvDest, int iValue, size uiBytes)				{ return memset(pvDest, iValue, uiBytes); }
	static int		Compare(const void* pv1, const void* pv2, size uiBytes)	{ return memcmp(pv1, pv2, uiBytes); }
	static void		Swap(const void* pv1, const void* pv2, size uiBytes)	{ MemSwp(pv1, pv2, uiBytes); }
#endif // __W65C816__
};


struct SArrayLargeSize
{
	typedef lsize		Count;

	static const lsize	Max = MAX_LSIZE;
	static const lsize	NotFound = ARRAY_LARGE_ELEMENT_NOT_FOUND;
	static const bool	CheckSize = false;

	static void*	Copy(void* pvDest, const void* pvSource, lsize uiBytes)	{ return memcpy(pvDest, pvSource, uiBytes); }
	static void*	Move(void* pvDest, const void* pvSource, lsize uiBytes)	{ return memmove(pvDest, pvSource, uiBytes); }
	static void*	Set(void* pvDest, int iValue, lsize uiBytes)			{ return memset(pvDest, iValue, uiBytes); }
	static int		Compare(const void* pv1, const void* pv2, lsize uiBytes)	{ return memcmp(pv1, pv2, uiBytes); }
	static void		Swap(const void* pv1, const void* pv2, lsize uiBytes)	{ MemSwp(pv1, pv2, uiBytes); }
};


#include "ArrayBlockClass.inl"


//ArrayBlock.cpp instantiates them; no other unit compiles their functions.
extern template struct SArrayHeaderBase<SArrayNaturalSize>;
extern template struct SArrayHeaderBase<SArrayLargeSize>;
extern template class CArrayBlockBase<SArrayNaturalSize>;
extern template class CArrayBlockBase<SArrayLargeSize>;


//The names the arrays are known by.  They are classes, not typedefs, so they can be declared ahead.
struct SArrayTemplateHeader : public SArrayHeaderBase<SArrayNaturalSize> {};
struct SArrayLargeTemplateHeader : public SArrayHeaderBase<SArrayLargeSize> {};
class CArrayBlock : public CArrayBlockBase<SArrayNaturalSize> {};
class CArrayLargeBlock : public CArrayBlockBase<SArrayLargeSize> {};


#endif // __ARRAY_BASE_H__

