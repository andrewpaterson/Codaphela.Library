#ifndef __DIST_TO_ROOT_EXPECTED_DISTS_H__
#define __DIST_TO_ROOT_EXPECTED_DISTS_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"


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


typedef CArrayTemplate<SDistToRoot>			CArrayDistToRoot;
typedef CArrayTemplate<SDistToRootHeapNode>		CArrayDistToRootHeap;


//The objects whose distance to the root is being given back, each with the closest it can be, taken closest first.
//Each object is in it once (unless removed); adding it again only ever lowers its expected dist.
class CDistToRootExpectedDists
{
private:
	CArrayDistToRoot		macDists;  //In the order the objects were added; removed ones stay, marked.
	CArrayDistToRootHeap	macHeap;  //Lowest first; stale nodes are skipped.
	size*					mpuiTable;  //Open addressing, object -> index + 1.  Only once there are many.
	size					muiTableSize;

public:
	void			Init(void);
	void			Kill(void);

	void			Add(CBaseObject* pcObject, int iExpectedDist);
	SDistToRoot*	GetLowest(void);
	void			Remove(SDistToRoot* psDistToRoot);

	void			Print(CChars* psz, bool bCommaSeparate);

private:
	size			FindIndex(CBaseObject* pcObject);
	size			TableSlot(CBaseObject* pcObject);
	void			InsertTable(size uiIndex);
	void			PutTable(size uiIndex);
	void			GrowTable(void);
	void			PushHeap(int iExpectedDist, size uiIndex);
	void			PopHeap(void);
	bool			IsHeapNodeStale(SDistToRootHeapNode* psNode);
};


#endif // __DIST_TO_ROOT_EXPECTED_DISTS_H__

