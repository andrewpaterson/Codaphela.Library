#ifndef __DIST_CALCULATOR_H__
#define __DIST_CALCULATOR_H__
#include "BaseLib/Chars.h"
#include "ObjectPointerArrays.h"
#include "DistToRootExpectedDists.h"


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
	uint32						muiWalk;  //This calculation's walk stamp base.
	CArrayBaseObjectPtrE32		mapcTouched;
	CArrayEmbeddedObjectPtr		mapcTos;  //One object's pointer tos at a time, its memory kept between them.
	CArrayBaseObjectPtrE32		mapcDetachedFromRoot;
	CArrayBaseObjectPtr			mapcCompletelyDetached;

public:
			void					Init(void);
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

