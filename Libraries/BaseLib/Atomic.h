/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela BaseLib

Codaphela BaseLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela BaseLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela BaseLib.  If not, see <http://www.gnu.org/licenses/>.

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __ATOMIC_H__
#define __ATOMIC_H__
#include "PrimitiveTypes.h"


//Reads and writes of memory that more than one thread touches.  They are inline, as what uses them (a spinning lock,
//an allocator's lists) is only worth having if it costs a few instructions.
//
//A Load or a Store is one whole value read or written, never half of one.  An Exchange, an Add and a CompareExchange
//are a read and a write no other thread can come between.
//
//The int32's are for flags and counts that threads wait on and hand work over by.  What a thread did before it
//changed one is seen by any thread whose Load reads that change (so a thread that waits for a flag may then read
//what the thread that set it left for it), and the Loads, Exchanges, Adds and CompareExchanges of all the threads
//happen in one order they all agree on (so of two threads that each Exchange a flag of their own and then Load the
//other's, at least one sees the other's set).  A Store promises only the first.
//
//A pointer's Store and Load publish what it points at: what a thread wrote before it Stored the pointer is seen by a
//thread whose Load reads it (so an object made and then pointed at is whole to the thread that follows the pointer).
//On x86-64 they are the plain instructions.  The pointers' Exchange and CompareExchange are as the int32's.
//
//The sizes' Load and Store promise neither: they are for a value one thread writes and others only need some whole
//value of.
//
//The 65C64 and the W65C816 run one thread, so there they are plain reads and writes.


#if defined(__LIBC65__)


inline int32 AtomicLoad(int32* pi)
{
	return *pi;
}


inline void AtomicStore(int32* pi, int32 iValue)
{
	*pi = iValue;
}


inline int32 AtomicExchange(int32* pi, int32 iValue)
{
	int32	iOld;

	iOld = *pi;
	*pi = iValue;
	return iOld;
}


inline int32 AtomicAdd(int32* pi, int32 iValue)
{
	*pi += iValue;
	return *pi;
}


inline lsize AtomicLoadSize(lsize* pui)
{
	return *pui;
}


inline void AtomicStoreSize(lsize* pui, lsize uiValue)
{
	*pui = uiValue;
}


inline void* AtomicLoadPointer(void** ppv)
{
	return *ppv;
}


inline void AtomicStorePointer(void** ppv, void* pv)
{
	*ppv = pv;
}


inline void* AtomicExchangePointer(void** ppv, void* pv)
{
	void*	pvOld;

	pvOld = *ppv;
	*ppv = pv;
	return pvOld;
}


inline bool AtomicCompareExchangePointer(void** ppv, void** ppvExpected, void* pv)
{
	if (*ppv == *ppvExpected)
	{
		*ppv = pv;
		return true;
	}
	*ppvExpected = *ppv;
	return false;
}


inline void AtomicPause(void)
{
}


#elif defined(_MSC_VER)
#include <intrin.h>


//On the processors MSVC builds for, an aligned read or write of a value no bigger than a pointer is whole, and volatile
//keeps the compiler from reading it twice or not at all.
inline int32 AtomicLoad(int32* pi)
{
	return *(volatile int32*)pi;
}


inline void AtomicStore(int32* pi, int32 iValue)
{
	*(volatile int32*)pi = iValue;
}


inline int32 AtomicExchange(int32* pi, int32 iValue)
{
	return (int32)_InterlockedExchange((volatile long*)pi, (long)iValue);
}


inline int32 AtomicAdd(int32* pi, int32 iValue)
{
	return (int32)_InterlockedExchangeAdd((volatile long*)pi, (long)iValue) + iValue;
}


inline lsize AtomicLoadSize(lsize* pui)
{
	return *(volatile lsize*)pui;
}


inline void AtomicStoreSize(lsize* pui, lsize uiValue)
{
	*(volatile lsize*)pui = uiValue;
}


inline void* AtomicLoadPointer(void** ppv)
{
	return *(void* volatile*)ppv;
}


inline void AtomicStorePointer(void** ppv, void* pv)
{
	*(void* volatile*)ppv = pv;
}


inline void* AtomicExchangePointer(void** ppv, void* pv)
{
	return _InterlockedExchangePointer((void* volatile*)ppv, pv);
}


inline bool AtomicCompareExchangePointer(void** ppv, void** ppvExpected, void* pv)
{
	void*	pvWas;

	pvWas = _InterlockedCompareExchangePointer((void* volatile*)ppv, pv, *ppvExpected);
	if (pvWas == *ppvExpected)
	{
		return true;
	}
	*ppvExpected = pvWas;
	return false;
}


inline void AtomicPause(void)
{
	_mm_pause();
}


#else // g++


inline int32 AtomicLoad(int32* pi)
{
	return __atomic_load_n(pi, __ATOMIC_SEQ_CST);
}


inline void AtomicStore(int32* pi, int32 iValue)
{
	__atomic_store_n(pi, iValue, __ATOMIC_RELEASE);
}


inline int32 AtomicExchange(int32* pi, int32 iValue)
{
	return __atomic_exchange_n(pi, iValue, __ATOMIC_SEQ_CST);
}


//Returns what it is after iValue is added.
inline int32 AtomicAdd(int32* pi, int32 iValue)
{
	return __atomic_add_fetch(pi, iValue, __ATOMIC_SEQ_CST);
}


inline lsize AtomicLoadSize(lsize* pui)
{
	return __atomic_load_n(pui, __ATOMIC_RELAXED);
}


inline void AtomicStoreSize(lsize* pui, lsize uiValue)
{
	__atomic_store_n(pui, uiValue, __ATOMIC_RELAXED);
}


inline void* AtomicLoadPointer(void** ppv)
{
	return __atomic_load_n(ppv, __ATOMIC_ACQUIRE);
}


inline void AtomicStorePointer(void** ppv, void* pv)
{
	__atomic_store_n(ppv, pv, __ATOMIC_RELEASE);
}


inline void* AtomicExchangePointer(void** ppv, void* pv)
{
	return __atomic_exchange_n(ppv, pv, __ATOMIC_SEQ_CST);
}


//False if *ppv wasn't *ppvExpected (or, now and then, when it was): *ppvExpected is then what it is, to try again with.
inline bool AtomicCompareExchangePointer(void** ppv, void** ppvExpected, void* pv)
{
	return __atomic_compare_exchange_n(ppv, ppvExpected, pv, true, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED);
}


//Tells the processor this is a loop waiting on another thread.
inline void AtomicPause(void)
{
#if defined(__i386__) || defined(__x86_64__)
	__builtin_ia32_pause();
#endif // __i386__ || __x86_64__
}


#endif // __LIBC65__, _MSC_VER, g++


#endif // __ATOMIC_H__

