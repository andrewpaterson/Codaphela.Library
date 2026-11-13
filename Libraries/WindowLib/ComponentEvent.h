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
#include "StandardLib/MemberCalls.h"


//Something that happens to a component of class S, like a button being clicked, and the member functions called when
//it does (see CMemberCalls).  A function takes the component it happened to, or nothing.
//
//The functions' objects aren't held, so an object removes its functions (Remove(this)) before it goes.
template<class S>
class CComponentEvent
{
protected:
	CMemberCalls	mcCalls;

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
};


//A component event that also gives a value of class V, like the page of a tab that was removed.  A function takes the
//component and the value, or just the component, or nothing.
template<class S, class V>
class CComponentValueEvent : public CComponentEvent<S>
{
public:
	using CComponentEvent<S>::Add;
	template<class M, class O>
	void	Add(O* pcObject, void(M::*fFunction)(S* pcSource, V xValue));

	void	Call(S* pcSource, V xValue);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S>
void CComponentEvent<S>::Init(void)
{
	mcCalls.Init();
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S>
void CComponentEvent<S>::Kill(void)
{
	mcCalls.Kill();
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
	mcCalls.Add(pcObject, fFunction);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S>
template<class M, class O>
void CComponentEvent<S>::Add(O* pcObject, void(M::*fFunction)(void))
{
	mcCalls.Add(pcObject, fFunction);
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
	return mcCalls.Remove(pvObject);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S>
size CComponentEvent<S>::NumCalls(void)
{
	return mcCalls.NumCalls();
}


//////////////////////////////////////////////////////////////////////////
//
// Calls each function in the order they were added (see
// CMemberCalls::Call).
//
//////////////////////////////////////////////////////////////////////////
template<class S>
void CComponentEvent<S>::Call(S* pcSource)
{
	mcCalls.Call(pcSource, NULL);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class S, class V>
template<class M, class O>
void CComponentValueEvent<S, V>::Add(O* pcObject, void(M::*fFunction)(S* pcSource, V xValue))
{
	this->mcCalls.Add(pcObject, fFunction);
}


//////////////////////////////////////////////////////////////////////////
//
// Calls each function as CComponentEvent::Call does, with xValue for the
// ones that take it.
//
//////////////////////////////////////////////////////////////////////////
template<class S, class V>
void CComponentValueEvent<S, V>::Call(S* pcSource, V xValue)
{
	this->mcCalls.Call(pcSource, &xValue);
}


#endif // __COMPONENT_EVENT_H__

