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
#ifndef __MEMBER_CALL_H__
#define __MEMBER_CALL_H__
#include <string.h>
#include "BaseLib/PrimitiveTypes.h"


//Holds a member function pointer whatever its class's inheritance; MSVC's are up to 24 bytes.
#define MEMBER_CALL_FUNCTION_SIZE	32


//Calls the member function kept in pucFunction on pvObject, giving it what it needs of pvArgument and of the value
//pvValue points to.
typedef void (*MemberCallThunk)(void* pvObject, uint8* pucFunction, void* pvArgument, void* pvValue);


//////////////////////////////////////////////////////////////////////////
//
// A thunk for a member function of M that takes nothing.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void MemberCallPlain(void* pvObject, uint8* pucFunction, void* pvArgument, void* pvValue)
{
	void(M::*fFunction)(void);

	memcpy(&fFunction, pucFunction, sizeof(fFunction));
	(((M*)pvObject)->*fFunction)();
}


//////////////////////////////////////////////////////////////////////////
//
// A thunk for a member function of M that takes nothing and answers true
// or false, which it puts in the bool pvValue points to.
//
//////////////////////////////////////////////////////////////////////////
template<class M>
void MemberCallAsk(void* pvObject, uint8* pucFunction, void* pvArgument, void* pvValue)
{
	bool(M::*fFunction)(void);

	memcpy(&fFunction, pucFunction, sizeof(fFunction));
	*(bool*)pvValue = (((M*)pvObject)->*fFunction)();
}


//////////////////////////////////////////////////////////////////////////
//
// A thunk for a member function of M that takes the argument, an A.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class A>
void MemberCallWith(void* pvObject, uint8* pucFunction, void* pvArgument, void* pvValue)
{
	void(M::*fFunction)(A*);

	memcpy(&fFunction, pucFunction, sizeof(fFunction));
	(((M*)pvObject)->*fFunction)((A*)pvArgument);
}


//////////////////////////////////////////////////////////////////////////
//
// A thunk for a member function of M that takes the argument, an A, and
// the value, a V.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class A, class V>
void MemberCallWithValue(void* pvObject, uint8* pucFunction, void* pvArgument, void* pvValue)
{
	void(M::*fFunction)(A*, V);

	memcpy(&fFunction, pucFunction, sizeof(fFunction));
	(((M*)pvObject)->*fFunction)((A*)pvArgument, *(V*)pvValue);
}


//A member function and the object it is called on, as an action, an input action or a component event keeps them.
//The object isn't held, so it must outlive the call or be removed from what holds it.  The object can be of a class
//derived from the function's, as when a component adds a function it inherits; if the function's class is a later base
//the object is kept as both.
//
//The member function pointer is kept as bytes with a thunk that knows its class; casting it to some other class's
//member pointer would lose the parts multiple inheritance adds.
struct SMemberCall
{
	void*			pvObject;  //As it was given.
	void*			pvCalled;  //As the function's class, which the thunk is given.
	MemberCallThunk	fThunk;
	uint8			aucFunction[MEMBER_CALL_FUNCTION_SIZE];

	template<class M, class O>
	void	Init(O* pcObject, void(M::*fFunction)(void));
	template<class M, class O>
	void	Init(O* pcObject, bool(M::*fFunction)(void));
	template<class M, class A, class O>
	void	Init(O* pcObject, void(M::*fFunction)(A*));
	template<class M, class A, class V, class O>
	void	Init(O* pcObject, void(M::*fFunction)(A*, V));
	template<class M, class O, class F>
	void	Init(O* pcObject, F fFunction, MemberCallThunk fCaller);

	void	Clear(void);
	bool	IsSet(void);
	void	Call(void* pvArgument, void* pvValue = NULL);
	bool	Ask(void);
	bool	IsOn(void* pvOther);
	bool	Equals(SMemberCall* psOther);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void SMemberCall::Init(O* pcObject, void(M::*fFunction)(void))
{
	Init<M>(pcObject, fFunction, &MemberCallPlain<M>);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O>
void SMemberCall::Init(O* pcObject, bool(M::*fFunction)(void))
{
	Init<M>(pcObject, fFunction, &MemberCallAsk<M>);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class A, class O>
void SMemberCall::Init(O* pcObject, void(M::*fFunction)(A*))
{
	Init<M>(pcObject, fFunction, &MemberCallWith<M, A>);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class A, class V, class O>
void SMemberCall::Init(O* pcObject, void(M::*fFunction)(A*, V))
{
	Init<M>(pcObject, fFunction, &MemberCallWithValue<M, A, V>);
}


//////////////////////////////////////////////////////////////////////////
//
// For a function of some other shape, with a thunk that calls it.  The
// thunk is given the object as M, the function's class, so it can cast
// the void* straight back to it.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class O, class F>
void SMemberCall::Init(O* pcObject, F fFunction, MemberCallThunk fCaller)
{
	M*	pcCalled;

	static_assert(sizeof(F) <= MEMBER_CALL_FUNCTION_SIZE, "MEMBER_CALL_FUNCTION_SIZE is too small for this member function pointer.");

	pcCalled = pcObject;
	memset(this, 0, sizeof(SMemberCall));
	pvObject = (void*)pcObject;
	pvCalled = (void*)pcCalled;
	fThunk = fCaller;
	memcpy(aucFunction, &fFunction, sizeof(F));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline void SMemberCall::Call(void* pvArgument, void* pvValue)
{
	fThunk(pvCalled, aucFunction, pvArgument, pvValue);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls a function that answers true or false, and gives its answer.
//
//////////////////////////////////////////////////////////////////////////
inline bool SMemberCall::Ask(void)
{
	bool	bAnswer;

	bAnswer = false;
	fThunk(pvCalled, aucFunction, NULL, &bAnswer);
	return bAnswer;
}


//////////////////////////////////////////////////////////////////////////
//
// No function; IsSet is false.
//
//////////////////////////////////////////////////////////////////////////
inline void SMemberCall::Clear(void)
{
	memset(this, 0, sizeof(SMemberCall));
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
inline bool SMemberCall::IsSet(void)
{
	return fThunk != NULL;
}


//////////////////////////////////////////////////////////////////////////
//
// pvOther can be the object as it was given or as the function's class,
// as when a base class removes its own functions (Remove(this)).
//
//////////////////////////////////////////////////////////////////////////
inline bool SMemberCall::IsOn(void* pvOther)
{
	return (pvObject == pvOther) || (pvCalled == pvOther);
}


//////////////////////////////////////////////////////////////////////////
//
// The same function on the same object.
//
//////////////////////////////////////////////////////////////////////////
inline bool SMemberCall::Equals(SMemberCall* psOther)
{
	return memcmp(this, psOther, sizeof(SMemberCall)) == 0;
}


#endif // __MEMBER_CALL_H__
