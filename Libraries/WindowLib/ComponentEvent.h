/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela WindowLib

Codaphela WindowLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela WindowLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela WindowLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __COMPONENT_EVENT_H__
#define __COMPONENT_EVENT_H__
#include "BaseLib/ArrayTemplate.h"
#include "MemberCall.h"


//Something that happens to a component of class S, like a button being clicked, and the member functions called when
//it does (see SMemberCall).  A function takes the component it happened to, or nothing.
//
//The functions' objects aren't held, so an object removes its functions (Remove(this)) before it goes.
template<class S>
class CComponentEvent
{
protected:
	CArrayTemplate<SMemberCall>	masCalls;

public:
	void	Init(void);
	void	Kill(void);

	template<class M, class O>
	void	Add(O* pcObject, void(M::*fFunction)(S* pcSource));
	template<class M, class O>
	void	Add(O* pcObject, void(M::*fFunction)(void));
	bool	Remove(void* pvObject);
	size	NumCalls(void);

	void	Call(S* pcSource);

protected:
	bool	Contains(SMemberCall* psCall);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S>
void CComponentEvent<S>::Init(void)
{
	masCalls.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S>
void CComponentEvent<S>::Kill(void)
{
	masCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
// pcObject is usually the object adding it, this.
//
//////////////////////////////////////////////////////////////////////////
template<class S>
template<class M, class O>
void CComponentEvent<S>::Add(O* pcObject, void(M::*fFunction)(S* pcSource))
{
	masCalls.Add()->Init(pcObject, fFunction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S>
template<class M, class O>
void CComponentEvent<S>::Add(O* pcObject, void(M::*fFunction)(void))
{
	masCalls.Add()->Init(pcObject, fFunction);
}


//////////////////////////////////////////////////////////////////////////
//
// Removes every function added for pvObject.  Returns false if there
// were none.
//
//////////////////////////////////////////////////////////////////////////
template<class S>
bool CComponentEvent<S>::Remove(void* pvObject)
{
	size	i;
	bool	bRemoved;

	bRemoved = false;
	for (i = masCalls.NumElements(); i > 0; i--)
	{
		if (masCalls.Get(i - 1)->IsOn(pvObject))
		{
			masCalls.RemoveAt(i - 1);
			bRemoved = true;
		}
	}
	return bRemoved;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S>
size CComponentEvent<S>::NumCalls(void)
{
	return masCalls.NumElements();
}


//////////////////////////////////////////////////////////////////////////
//
// Calls each function in the order they were added.  A function can add
// or remove functions; one removed before its turn isn't called, and one
// added isn't called until the next time.
//
//////////////////////////////////////////////////////////////////////////
template<class S>
void CComponentEvent<S>::Call(S* pcSource)
{
	CArrayTemplate<SMemberCall>	asCalls;
	SMemberCall*				psCall;
	size						i;

	asCalls.Init();
	asCalls.Copy(&masCalls);
	for (i = 0; i < asCalls.NumElements(); i++)
	{
		psCall = asCalls.Get(i);
		if (Contains(psCall))
		{
			psCall->Call(pcSource);
		}
	}
	asCalls.Kill();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S>
bool CComponentEvent<S>::Contains(SMemberCall* psCall)
{
	size	i;

	for (i = 0; i < masCalls.NumElements(); i++)
	{
		if (masCalls.Get(i)->Equals(psCall))
		{
			return true;
		}
	}
	return false;
}


#endif // __COMPONENT_EVENT_H__
