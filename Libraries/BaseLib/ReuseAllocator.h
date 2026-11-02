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

Microsoft Windows is Copyright Microsoft Corporation

** ------------------------------------------------------------------------ **/
#ifndef __REUSE_ALLOCATOR_H__
#define __REUSE_ALLOCATOR_H__
#include "Mallocator.h"


//One buffer's allocator (a CChars or an array that is emptied and filled over and over, like a line read at a
//time).  It keeps the one block it allocated: Realloc never makes it smaller and Free keeps it for the next Malloc.
//So the buffer grows to its biggest use and is not allocated again.  A block bigger than the keep limit is freed
//when it is freed, and shrunk when it is reallocated to fit the limit again (one ridiculously long line is not
//kept).  A keep limit of 0 keeps any block.
//
//Only one block is kept.  Should a second be wanted while the first is in use it comes from gcSystemAllocator.
//Kill frees the kept block: kill the buffer first.
class CReuseAllocator : public CMallocator
{
CONSTRUCTABLE(CReuseAllocator)
protected:
	void*		mpvBlock;
	lsize		muiBlockSize;
	lsize		muiKeepLimit;
	bool		mbInUse;

public:
	void		Init(lsize uiKeepLimit);
	void		Kill(void);

	void*		Malloc(lsize uiSize);
	void*		Realloc(void* pv, lsize uiSize);
	bool		Free(void* pv);

	bool		IsLocal(void);
	size		SizeOffset(void);

	const char*	ShortName(void);

	lsize		BlockSize(void);

protected:
	void		FreeBlock(void);
};


#endif // __REUSE_ALLOCATOR_H__
