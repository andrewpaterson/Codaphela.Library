/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2009 Andrew Paterson

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
#ifndef __FREE_LIST_MAYBE_H__
#define __FREE_LIST_MAYBE_H__
#include "FreeListBlock.h"


#define NUM_ELEMENTS_BEFORE_FREELIST	16
#define NUM_ELEMENTS_IN_CHUNK			8


class CFreeListMaybe
{
private:
	void* PrivateCreateLinkList(void);
	void* PrivateCreateFreeList(void);

public:
	CFreeListBlock*	mpcFreeList;
	CLinkListBlock*	mpcLinkList;
	int					miElementSize;
	int					miChunkSize;

	void	Init(int iElementSize, int iChunkSize = NUM_ELEMENTS_IN_CHUNK);
	void	Kill(void);
	void*	Add(void);  //This is a malloc equivalent.
	void*	AddUseFreelist(int iChunkSize = 0);				//Force usage of a freelist if not yet used.
	void*	AddUseMalloc(void);  //Just for completeness.
	void	Remove(void* pvElement);
	BOOL	SafeRemove(void* pvElement);  //Make sure to only remove elements which we have allocated.
};


#endif // __FREE_LIST_MAYBE_H__

