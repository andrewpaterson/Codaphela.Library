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
#ifndef __SCRATCH_PAD_H__
#define __SCRATCH_PAD_H__
#include "MemoryStackExtended.h"


#ifdef __W65C816__
#define SCRATCH_PAD_CHUNK_SIZE	(32 KB)		//Each chunk is allocated whole; the W65C816 has 16MB in all.
#else
#define SCRATCH_PAD_CHUNK_SIZE	(6 MB)
#endif // __W65C816__


struct SSPNode
{
public:
	size			iSize;
	bool			bUsed;
};


struct SScratchPadParams
{
	int32	iChunkSize;	//Written to files: four bytes everywhere.
};


class CScratchPad
{
protected:
	CMemoryStackExtended	mcScratchPad;
	size					miSourceChunkSize;

public:
	void	Init(void);
	void	Init(size uiChunkSize);
	void	Kill(void);

	void*	Add(size uiSize);
	void	Reset(void);

	size	GetMemorySize(void);
	size	GetUsedSize(void);

	void	GetParams(SScratchPadParams* psParams);
};


#endif // __SCRATCH_PAD_H__

