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
#ifndef __DIST_TO_ROOT_EXPECTED_DISTS_H__
#define __DIST_TO_ROOT_EXPECTED_DISTS_H__
#include "BaseLib/Chars.h"
#include "BaseLib/ArrayTemplate.h"
#include "PointerLocations.h"


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
	CPointerLocations		mcIndices;  //Each object's indices in macDists, once there are many (mbIndexed).
	bool					mbIndexed;

public:
	void			Init(void);
	void			Kill(void);

	void			Add(CBaseObject* pcObject, int iExpectedDist);
	SDistToRoot*	GetLowest(void);
	void			Remove(SDistToRoot* psDistToRoot);

	void			Print(CChars* psz, bool bCommaSeparate);

private:
	size			FindIndex(CBaseObject* pcObject);
	void			AddIndex(size uiIndex);
	void			PushHeap(int iExpectedDist, size uiIndex);
	void			PopHeap(void);
	bool			IsHeapNodeStale(SDistToRootHeapNode* psNode);
};


#endif // __DIST_TO_ROOT_EXPECTED_DISTS_H__

