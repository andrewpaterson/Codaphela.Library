#ifndef __DIST_TO_ROOT_EFFECTED_FROMS_H__
#define __DIST_TO_ROOT_EFFECTED_FROMS_H__
#include "BaseLib/Chars.h"
#include "ObjectPointerArrays.h"


class CBaseObject;


struct SDistToRoot
{
	CBaseObject*	pcObject;
	int				iExpectedDist;
	bool			bRemoved;
};


//A heap node orders by expected dist, then by when the object was first added (its index in the expected dists).
struct SDistToRootHeapNode
{
	int				iExpectedDist;
	size			uiIndex;
};


//An object to check while clearing distances to the root, and the distance the cleared object that points to it had.
struct SDistToRootClearCandidate
{
	CBaseObject*	pcObject;
	int				iClearedFromDistToRoot;
};


typedef CArrayTemplate<SDistToRoot>				CArrayDistToRoot;
typedef CArrayLargeTemplate<SDistToRootClearCandidate>	CArrayDistToRootClearCandidates;  //Large: freeing a big graph queues one per object.
typedef CArrayTemplate<SDistToRootHeapNode>		CArrayDistToRootHeap;


class CDistCalculatorParameters
{
private:
	uint32						muiWalk;  //This calculation's walk stamp base.
	CArrayBaseObjectPtrE32		mapcTouched;
	CArrayDistToRoot						macExpectedDists;  //In the order the objects were added; removed ones stay, marked.
	size									muiRemovedExpectedDists;
	CArrayDistToRootHeap					macExpectedDistsHeap;  //Lowest first; stale nodes are skipped.
	size*									mpuiExpectedDistsTable;  //Open addressing, object -> index + 1.  Only once there are many.
	size									muiExpectedDistsTableSize;
	CArrayDistToRootClearCandidates			macClearCandidates;  //A stack, so clearing walks depth first without recursing.
	CArrayEmbeddedObjectPtr					mapcTos;  //One object's pointer tos at a time, its memory kept between them.
	CArrayBaseObjectPtrE32		mapcDetachedFromRoot;

	CArrayBaseObjectPtr					mapcCompletelyDetached;

public:
	void					Init(void);
	void					Kill(void);

	void					AddExpectedDist(CBaseObject* pcObject, int iExpectedDist);
	SDistToRoot*			GetLowestExpectedDist(void);
	size					NumExpectedDists(void);
	SDistToRoot*			GetExpectedDist(int iIndex);
	SDistToRoot*			GetExpectedDist(CBaseObject* pcObject);
	void					RemoveExpectedDist(int iIndex);
	void					RemoveExpectedDist(SDistToRoot* psDistToRoot);

	CArrayEmbeddedObjectPtr*	StartPointerTos(void);
	void					AddClearCandidate(CBaseObject* pcObject, int iClearedFromDistToRoot);
	bool					PopClearCandidate(CBaseObject** ppcObject, int* piClearedFromDistToRoot);


	void					AddDetachedFromRoot(CBaseObject* pcObject);
	size					NumDetachedFromRoot(void);
	CBaseObject*			GetDetachedFromRoot(size iIndex);

	void					AddCompletelyDetached(CBaseObject* pcObject);
	size					NumCompletelyDetached(void);
	CBaseObject*			GetCompletelyDetached(size iIndex);
	void					RemoveCompletelyDetached(size iIndex);
	void					CopyRootDetachedToCompletelyDetached(void);
	CArrayBaseObjectPtr*	GetCompletelyDetachedArray(void);

	void					MarkReachedFromStack(CBaseObject* pcObject);
	bool					IsReachedFromStack(CBaseObject* pcObject);

	void					AddTouched(CBaseObject* pcObject);
	size					NumTouched(void);
	CBaseObject*			GetTouched(size iIndex);

	void					ClearTouchedFlags(void);

	void					Print(CChars* psz, bool bCommaSeparate = true);
	void					Dump(void);

	void					PrintArray(CChars* psz, CArrayBaseObjectPtrE32* pcArray, bool bCommaSeparate);
	void					PrintArray(CChars* psz, CArrayBaseObjectPtr* pcArray, bool bCommaSeparate);
	void					PrintArray(CChars* psz, CArrayDistToRoot* pcArray, bool bCommaSeparate);

protected:
	size					FindExpectedDistIndex(CBaseObject* pcObject);
	size					ExpectedDistsTableSlot(CBaseObject* pcObject);
	void					InsertExpectedDistsTable(size uiIndex);
	void					PutExpectedDistsTable(size uiIndex);
	void					GrowExpectedDistsTable(void);
	void					PushExpectedDistsHeap(int iExpectedDist, size uiIndex);
	void					PopExpectedDistsHeap(void);
	bool					IsExpectedDistsHeapNodeStale(SDistToRootHeapNode* psNode);
	SDistToRoot*			GetLiveExpectedDist(size uiLiveIndex);
};


#endif // __DIST_TO_ROOT_CALCULATOR_H__

