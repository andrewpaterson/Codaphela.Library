/** ---------------- COPYRIGHT NOTICE, DISCLAIMER, and LICENSE ------------- **

Copyright (c) 2026 Andrew Paterson

This file is part of The Codaphela Project: Codaphela CppParserLib

Codaphela CppParserLib is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Codaphela CppParserLib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Codaphela CppParserLib.  If not, see <http://www.gnu.org/licenses/>.

** ------------------------------------------------------------------------ **/
#ifndef __I_R_LIVE_SETS_H__
#define __I_R_LIVE_SETS_H__
#include "BaseLib/ArrayTemplate.h"
#include "IRFunction.h"


//Which temporaries are live into and out of each block of a function, the dataflow both back ends' register
//allocation starts from (CIRLiveness for the 65C64, CIR816Backend::PackTemporaries).  The caller says what each block
//uses and defines, in order (Use, Define), and the phi arguments it passes on (PassOut), then Solve works out
//in = used | (out & ~defined) and out = the successors' ins until nothing changes.
class CIRLiveSets
{
protected:
	CIRFunction*			mpcFunction;
	size					muiWords;		//Words in a block's set: a bit per temporary.
	CArrayTemplate<uint32>	mauiUsed;		//Per block: muiWords words, the temporaries it uses before defining them.
	CArrayTemplate<uint32>	mauiDefined;
	CArrayTemplate<uint32>	mauiLiveIn;
	CArrayTemplate<uint32>	mauiLiveOut;

public:
	void	Init(void);
	void	Init(CMallocator* pcMalloc);
	void	Kill(void);

	void	Start(CIRFunction* pcFunction);
	void	Use(size uiBlock, SIRReference sValue);
	void	Define(size uiBlock, SIRReference sValue);
	void	PassOut(size uiBlock, SIRReference sValue);
	void	Solve(void);

	size	NumWords(void);
	uint32*	LiveIn(size uiBlock);
	uint32*	LiveOut(size uiBlock);
	bool	IsLiveIn(size uiBlock, size uiTemporary);
	bool	IsLiveOut(size uiBlock, size uiTemporary);
};


#endif // __I_R_LIVE_SETS_H__
