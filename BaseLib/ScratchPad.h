/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2012 Andrew Paterson

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


class CScratchPad
{
protected:
	CMemoryStackExtended	mcScratchPad;

public:
	void	Init(void);
	void	Init(int iChunkSize);
	void	Kill(void);

	void*	Add(int iBytes);
	void	Remove(void);
	void	Remove(int iNumToRemove);
	void	Reset(void);

	int		GetMemorySize(void);
	int		GetUsedSize(void);
};


#endif // __SCRATCH_PAD_H__

