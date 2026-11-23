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

** ------------------------------------------------------------------------ **/
#ifndef __QUEUE_TEMPLATE_H__
#define __QUEUE_TEMPLATE_H__
#include "ArrayTemplate.h"


//The dead elements before the front are moved out once there are at least this many and more of them than live ones.
#define QUEUE_TEMPLATE_MIN_COMPACT	8


//A queue of M: pushed at the back, taken or dropped at the front, and cut from the back (a history going back, or
//the redo end).  The elements from muiFront to the array's end are the queue, in order, so Get(0) is the front.
//Taking the front moves muiFront on; when the dead elements before it outnumber the live ones (and are at least
//QUEUE_TEMPLATE_MIN_COMPACT) one RemoveRange moves the live ones down, so taking the front is amortised O(1).  A
//pointer into the queue lasts until the next Push, Pop or Remove (see HashMapPlan.txt's CQueueTemplate).  BLOCK is
//CArrayBlock (counted in size) or CArrayLargeBlock (lsize), as the arrays'.  A new element is given its virtual
//function table but isn't zeroed, as CArrayTemplate's aren't: the caller initialises it.  Nothing is destructed: a
//caller that must Kill an element does so before taking it.
template<class M, class BLOCK = CArrayBlock>
class CQueueTemplate
{
public:
	typedef typename BLOCK::Count	Count;

protected:
	CArrayTemplateBase<M, BLOCK>	maElements;
	Count							muiFront;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);
	void	ReInit(void);

	M*		Push(void);
	M*		Push(M* pElement);
	bool	Pop(M* pDest);
	bool	Pop(void);

	M*		Front(void);
	M*		Back(void);
	M*		Get(Count uiIndex);
	Count	NumElements(void) const;
	bool	IsEmpty(void) const;

	bool	RemoveBack(void);
	void	RemoveFrom(Count uiIndex);

protected:
	void	Taken(void);
};


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
void CQueueTemplate<M, BLOCK>::Init(void)
{
	maElements.Init();
	muiFront = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
void CQueueTemplate<M, BLOCK>::Init(CMallocator* pcMalloc)
{
	maElements.Init(pcMalloc);
	muiFront = 0;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
void CQueueTemplate<M, BLOCK>::Kill(void)
{
	maElements.Kill();
	muiFront = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// Empties the queue.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
void CQueueTemplate<M, BLOCK>::ReInit(void)
{
	maElements.ReInit();
	muiFront = 0;
}


//////////////////////////////////////////////////////////////////////////
//
// A new element at the back, not initialised.  NULL if there is no
// memory for it.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CQueueTemplate<M, BLOCK>::Push(void)
{
	return maElements.Add();
}


//////////////////////////////////////////////////////////////////////////
//
// A copy of *pElement at the back.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CQueueTemplate<M, BLOCK>::Push(M* pElement)
{
	return maElements.Add(pElement);
}


//////////////////////////////////////////////////////////////////////////
//
// The front copied to *pDest and taken.  False if the queue is empty.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
bool CQueueTemplate<M, BLOCK>::Pop(M* pDest)
{
	if (IsEmpty())
	{
		return false;
	}
	memcpy((void*)pDest, (const void*)maElements.Get(muiFront), sizeof(M));
	muiFront++;
	Taken();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The front taken.  False if the queue is empty.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
bool CQueueTemplate<M, BLOCK>::Pop(void)
{
	if (IsEmpty())
	{
		return false;
	}
	muiFront++;
	Taken();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// The oldest element, or NULL.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CQueueTemplate<M, BLOCK>::Front(void)
{
	return IsEmpty() ? NULL : maElements.Get(muiFront);
}


//////////////////////////////////////////////////////////////////////////
//
// The newest element, or NULL.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CQueueTemplate<M, BLOCK>::Back(void)
{
	return IsEmpty() ? NULL : maElements.Tail();
}


//////////////////////////////////////////////////////////////////////////
//
// The element uiIndex from the front (0 the front itself).
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
M* CQueueTemplate<M, BLOCK>::Get(Count uiIndex)
{
	return maElements.Get(muiFront + uiIndex);
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
typename BLOCK::Count CQueueTemplate<M, BLOCK>::NumElements(void) const
{
	return maElements.NumElements() - muiFront;
}


//////////////////////////////////////////////////////////////////////////
//
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
bool CQueueTemplate<M, BLOCK>::IsEmpty(void) const
{
	return maElements.NumElements() == muiFront;
}


//////////////////////////////////////////////////////////////////////////
//
// The newest element taken.  False if the queue is empty.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
bool CQueueTemplate<M, BLOCK>::RemoveBack(void)
{
	if (IsEmpty())
	{
		return false;
	}
	maElements.RemoveTail();
	Taken();
	return true;
}


//////////////////////////////////////////////////////////////////////////
//
// Every element from uiIndex (from the front) to the back taken.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
void CQueueTemplate<M, BLOCK>::RemoveFrom(Count uiIndex)
{
	if (uiIndex >= NumElements())
	{
		return;
	}
	maElements.RemoveRange(muiFront + uiIndex, maElements.NumElements());
	Taken();
}


//////////////////////////////////////////////////////////////////////////
//
// After an element is taken: an empty queue starts again at the start of
// its memory; a front with more dead elements before it than live after
// has the live ones moved down.
//
//////////////////////////////////////////////////////////////////////////
template<class M, class BLOCK>
void CQueueTemplate<M, BLOCK>::Taken(void)
{
	Count	uiLive;

	uiLive = maElements.NumElements() - muiFront;
	if (uiLive == 0)
	{
		maElements.RemoveRange(0, maElements.NumElements());
		muiFront = 0;
	}
	else if ((muiFront >= QUEUE_TEMPLATE_MIN_COMPACT) && (muiFront > uiLive))
	{
		maElements.RemoveRange(0, muiFront);
		muiFront = 0;
	}
}


#endif // __QUEUE_TEMPLATE_H__

