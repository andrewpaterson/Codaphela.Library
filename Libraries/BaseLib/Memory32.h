#ifndef __MEMORY_32_H__
#define __MEMORY_32_H__
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "PrimitiveTypes.h"


//The 32 suffixed memory functions, for counts that can pass 64KB (lsize), on every target.  On the W65C816 size_t
//is 16 bits and memcpy and the others take 16 bit counts: Libc65's memcpy32, malloc32 and the others take 32 bit ones
//(and malloc32's blocks of more than a bank have whole banks).  Everywhere else size_t holds an lsize and these are
//the plain functions.  A call needn't name them: memcpy, malloc and the others called with an lsize count are these on
//the W65C816 (PrimitiveTypes.h's overloads choose by the count's type).  They are named where the code means the 32 bit
//one whatever it is given (SArrayLargeSize, CSystemAllocator), and strlen32 always is, as a result's type chooses nothing.
#if !defined(__W65C816__)
inline void*	memcpy32(void* pvDest, const void* pvSource, lsize uiBytes)	{ return memcpy(pvDest, pvSource, uiBytes); }
inline void*	memmove32(void* pvDest, const void* pvSource, lsize uiBytes)	{ return memmove(pvDest, pvSource, uiBytes); }
inline void*	memset32(void* pvDest, int iValue, lsize uiBytes)				{ return memset(pvDest, iValue, uiBytes); }
inline int		memcmp32(const void* pv1, const void* pv2, lsize uiBytes)		{ return memcmp(pv1, pv2, uiBytes); }
inline void*	memchr32(const void* pv, int iValue, lsize uiBytes)				{ return (void*)memchr(pv, iValue, uiBytes); }
inline lsize	strlen32(const char* sz)										{ return strlen(sz); }
inline void*	malloc32(lsize uiBytes)											{ return malloc(uiBytes); }
inline void*	calloc32(lsize uiCount, lsize uiBytes)							{ return calloc(uiCount, uiBytes); }
inline void*	realloc32(void* pv, lsize uiBytes)								{ return realloc(pv, uiBytes); }
inline lsize	fread32(void* pv, lsize uiSize, lsize uiCount, FILE* pcFile)	{ return fread(pv, uiSize, uiCount, pcFile); }
inline lsize	fwrite32(const void* pv, lsize uiSize, lsize uiCount, FILE* pcFile)	{ return fwrite(pv, uiSize, uiCount, pcFile); }
#endif // !__W65C816__


#endif // __MEMORY_32_H__
