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
#ifndef __DIST_CALCULATOR_H__
#define __DIST_CALCULATOR_H__
#include "BaseLib/Chars.h"
#include "ObjectPointerArrays.h"
#include "DistToRootExpectedDists.h"


class CObjects;

//An object to check while clearing distances to the root, and the distance the cleared object that points to it had.
struct SDistToRootClearCandidate
{
	CBaseObject*	pcObject;
	int				iClearedFromDistToRoot;
};

typedef CArrayLargeTemplate<SDistToRootClearCandidate>	CArrayDistToRootClearCandidates;  //Large: freeing a big graph queues one per object.


//Works out what a from going away detaches.  The distances to the root are kept exact (a breadth first search from
//the root) as heap froms change (DistToRoot.cpp); the objects detached from the root that can't be reached from the
//stack either are the result: the completely detached objects, to be freed (DistToStack.cpp).  It is one walk: what it
//finds is in the walked objects' stamps (WALK_STATE_...).  Objects only give access to their froms, tos, distances and
//flags.
class CDistCalculator
{
private:
	CObjects*					mpcObjects;  //The objects whose walk stamps this uses.
	uint32						muiWalk;  //This calculation's walk stamp base.
	CArrayBaseObjectPtrE32		mapcTouched;
	CArrayEmbeddedObjectPtr		mapcTos;  //One object's pointer tos at a time, its memory kept between them.
	CArrayBaseObjectPtrE32		mapcDetachedFromRoot;
	CArrayBaseObjectPtr			mapcCompletelyDetached;

public:
			void					Init(CObjects* pcObjects);
			void					Kill(void);

			CArrayBaseObjectPtr*	Calculate(CBaseObject* pcFromChanged, bool bHeapFromChanged);
	static	void					FromAdded(CBaseObject* pcObject, int iExpectedDistToRoot);
	static	bool					IsHeldFromStackOrRoot(CBaseObject* pcObject);

			//The steps of Calculate, for tests.
			void					CalculateDistToRoot(CBaseObject* pcFromRemoved);
			void					CalculateDistToStack(CBaseObject* pcFromRemoved);
			bool					RecalculateKnownHeldFromStack(CBaseObject* pcFromRemoved);
			void					CalculateFromTouched(void);

			size					NumTouched(void);
			CBaseObject*			GetTouched(size iIndex);
			size					NumDetachedFromRoot(void);
			CBaseObject*			GetDetachedFromRoot(size iIndex);
			size					NumCompletelyDetached(void);
			CBaseObject*			GetCompletelyDetached(size iIndex);
			CArrayBaseObjectPtr*	GetCompletelyDetachedArray(void);
			bool					IsReachedFromStack(CBaseObject* pcObject);

			void					Print(CChars* psz, bool bCommaSeparate = true);
			void					Dump(void);

private:
			uint32					GetWalkState(CBaseObject* pcObject);
			void					SetWalkState(CBaseObject* pcObject, uint32 uiState);
			void					MarkReachedFromStack(CBaseObject* pcObject);
			void					AddTouched(CBaseObject* pcObject);
			CArrayEmbeddedObjectPtr*	StartPointerTos(void);

			//DistToRoot.cpp
			void					CollectAndClearInvalidDistToRoot(CBaseObject* pcFromRemoved);
			void					ClearInvalidDistToRoot(CBaseObject* pcObject, int iClearedFromDistToRoot, CArrayDistToRootClearCandidates* pacCandidates);
			bool					HasCloserFrom(CBaseObject* pcObject);
			void					ResetClearedToUnattached(void);
			void					UpdateAttachedTosDistToRoot(CDistToRootExpectedDists* pcExpectedDists);
			void					UpdateAttachedTosDistToRoot(CBaseObject* pcObject, CDistToRootExpectedDists* pcExpectedDists);
			void					AddExpectedDistToRoot(CEmbeddedObject* pcPointedTo, int iExpectedDist, CDistToRootExpectedDists* pcExpectedDists);

			//DistToStack.cpp
			size					CollectDetachedFromRoot(void);
			size					CollectDetachedFromRoot(CBaseObject* pcObject, CArrayBaseObjectPtr* papcFroms);
			bool					CollectDetached(CBaseObject* pcObject, CArrayBaseObjectPtr* papcFroms);
			void					AddDetachedFromRoot(CBaseObject* pcObject);
			void					InitialiseCompletelyDetached(void);
			void					AddCompletelyDetached(CBaseObject* pcObject);
			void					RemoveCompletelyDetached(size iIndex);
			void					CopyRootDetachedToCompletelyDetached(void);
			bool					HasFromReachedFromStack(CBaseObject* pcObject);
			void					MarkPointedToFromReached(CArrayBaseObjectPtr* papcReached);
			void					FindReachedFromStack(void);
	static	bool					HasStackOrOlderKnownHeldFrom(CBaseObject* pcObject);
	static	void					ClearKnownHeldFromStack(CBaseObject* pcObject, uint32 uiCleared, CArrayBaseObjectPtr* papcCandidates, CArrayBaseObjectPtr* papcCleared);
	static	void					AddNewerTos(CBaseObject* pcObject, CArrayBaseObjectPtr* papcObjects);

			void					PrintArray(CChars* psz, CArrayBaseObjectPtrE32* pcArray, bool bCommaSeparate);
			void					PrintArray(CChars* psz, CArrayBaseObjectPtr* pcArray, bool bCommaSeparate);
};


#endif // __DIST_CALCULATOR_H__

